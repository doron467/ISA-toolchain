/* This program has generic functions that are used throughout the program.
 * these functions help with reading lines from files, analyzing input, handling errors, and more.
 * these functions are used almost everywhere, and they are mostly for convenience.
 * this file is included in all the other source files.
 */

#include "utils.h"

/* creates a pointer to a program state and returns it, also sets the program state's values */
pProgramState createProgramState(){
	pProgramState pState; /* the returned program state pointer */

	pState = (pProgramState)malloc(sizeof(ProgramState));
	if (pState == NULL){
		return NULL;
	}

	pState->lineNumber = 0; /* line counter is increases before the line is read, so it should start at 0 */
	pState->state = VALID; /* state starts at valid obviously, which is least problematic state */

	return pState;
}

/* updates the program state to the new state, and prints the errorStr like printf would.
 * the program state will change to the new state only if the new state is more problematic than the current state. */
void error(pProgramState pState,int newState,char *errorStr, ...){
	va_list ap; /* arguments list for the arguments after the errorStr */

	va_start(ap,errorStr);

	if (newState == CODE_ERROR){
		printf("\terror at line %lu: ",pState->lineNumber);
	}

	vprintf(errorStr,ap); /* use vprintf to format the errorStr and print it */

	va_end(ap);

	/* update the state */
	if (newState > pState->state){
		pState->state = newState;
	}
}

/* move the line pointer to the next non whitespace character */
void skipLineWhitespaces(char **line){
	for (; **line != '\0' && **line != ',' && isspace(**line); (*line)++);
}

/* move the line pointer to the next non word character */
void skipLineWord(char **line){
	for (; **line != '\0' && **line != ',' && **line != ':' && !isspace(**line); (*line)++);
}

/* if line points to a seprarating character, it increases to the next character */
void skipLineSeparateChar(char **line){
	if (**line == ',' || **line == ':'){
		(*line)++;
	}
}

/* fills the line array with the next line in the given file.
 * also increases the line counter in pState by 1.
 * the line will also have the \n character (if it's in the file). */
int readLine(FILE *fp,char *line,pProgramState pState){

	unsigned int lineLength = 0; /* final length of the line */
	int c; /* the next character */

	pState->lineNumber++;

	while ((c = getc(fp)) != EOF){
		line[lineLength++] = c;
		if (c == '\n'){
			break;
		}

		if (lineLength >= LINE_MAX_SIZE){

			/* need to read until the next line to clear the buffer */
			for (c = getc(fp); c != EOF && c != '\0' && c != '\n'; c = getc(fp));

			if (line[0] == ';'){ /* comments shouldn't have a length limit, preprocessor ignores them anyway */
				break;
			}

			error(pState,CODE_ERROR,"line length cannot be above %d\n",LINE_MAX_SIZE);
			break;
		}

	}

	line[lineLength] = '\0';
	return 1;
}

/* compares the next word in the given line to the given word parameter (similar to strcmp).
 * returns 1 if they are equal, otherwise 0. if nextWord isn't null, it will be set
 * to the next word in the line string. if there is no next word, nextWord is set to null. */
int cmpNextWord(char *line,char *word,char **nextWord){
	int isEqual; /* 1 if the words are equal, 0 otherwise */

	skipLineSeparateChar(&line); /* skip separating character before the word (it should be checked outside the function if it's legal or not) */
	skipLineWhitespaces(&line); /* skip whitespaces before word */

	for (; *line != '\0' && *word != '\0' && !isspace(*line) && *line != ','; line++, word++){
		if (*line != *word){
			break;
		}
	}

	isEqual = *word == '\0' && (isspace(*line) || *line == ',' || *line == ':' || *line == '\0'); /* word has to properly end */

	if (nextWord == NULL){ /* no need to do all the extra stuff if nextWord is null */
		return isEqual;
	}

	skipLineWord(&line); /* go to end of the word */
	skipLineWhitespaces(&line); /* skip whitespaces after word */

	if (*line == '\0'){
		*nextWord = NULL;
	} else {
		*nextWord = line;
	}

	return isEqual;
}

/* checks if the next word in line is equal to at least one of the given words.
 * if it is, function returns 1, otherwise 0.
 * wordsAmount is the amount of words you want to compare the next word to.
 * after wordsAmount are the words you want to compare the next word to.
 * this function works by calling cmpNextWord for all the given words */
int cmpNextWords(char *line,int wordsAmount, ...){
	int i;
	char *word; /* the current word the function is comparing the next word to */
	va_list ap; /* arguments list, starts after wordsAmount */
	int isEqual = 0; /* the returned value */

	va_start(ap,wordsAmount);

	for (i = 0; i < wordsAmount; i++){
		word = va_arg(ap,char*);

		if (cmpNextWord(line,word,NULL)){
			isEqual = 1;
			break;
		}
	}

	va_end(ap);
	return isEqual;
}

/* checks if the next word in str is a valid register. if it is,
 * the function will return the number of the register, otherwise -1.
 * returning the number of the register ended up being useless,
 * but I figured I should keep the function like this anyway */
int isRegister(char *str){
	int registerNum; /* number of the register */
	if (*str == 'r'){
		registerNum = *(++str) - '0';
		if (registerNum < REGISTERS_AMOUNT && registerNum >= 0){
			str++;
			if (isspace(*str) || *str == ',' || *str == ':' || *str == '\0'){
				return registerNum;
			}
		}
	}
	return -1;
}

/* checks if a given macro's name is legal or not. if it is, function returns 1, otherwise 0.
 * a macro's name counts as legal if it follows all the next rules:
 * the first character has to be either an alphabetical character or a number
 * the name isn't an existing operation (like mov,add,stop,etc)
 * the name isn't a valid register
 * the name isn't an existing instruction (entry,extern)
 * the name isn't a macro keyword (macr,endmacr)
 * the length of the name isn't above the max (should be 31 if it wasn't changed)
 *
 * note that this function doesn't check if the length of the name is above the max,
 * as it should already be checked when allocating the memory for the name*/
int isLegalMacroName(char *macroName,pProgramState pState){

	if (isRegister(macroName) != -1){
		error(pState,CODE_ERROR,"macro name cannot be a register name\n");
		return 0;
	}

	if (getOperationCode(macroName).code != -1){
		error(pState,CODE_ERROR,"macro name can't be an existing operation\n");
		return 0;
	}

	if (cmpNextWords(macroName,4,"macr","endmacr","entry","extern")){
		error(pState,CODE_ERROR,"macro name can't be an instruction\n");
		return 0;
	}

	if (!isalnum(macroName[0])){
		error(pState,CODE_ERROR,"macro name has to start with an alphabetical character or number\n");
		return 0;
	}

	return 1;
}

/* checks if a given label name is legal or not. if it is, function returns 1, otherwise 0
 * a label name counts as legals if it follows all the next rules:
 * the first character has to be an alphabetical character
 * all of the characters are either alphabetical characters or numbers
 * the name isn't an existing operation (like mov,add,stop,etc)
 * the name isn't a valid register
 * the name isn't an existing instruction (entry,extern)
 * the name isn't a macro keyword (macr,endmacr)
 * the length of the name isn't above the max (should be 31 if it wasn't changed)
 *
 * note that this function doesn't check if the length of the name is above the max,
 * as it should already be checked when allocating the memory for the name. */
int isLegalLabelName(char *labelName,pProgramState pState){

	if (isRegister(labelName) != -1){
		error(pState,CODE_ERROR,"label name cannot be a register name\n");
		return 0;
	}

	if (getOperationCode(labelName).code != -1){
		error(pState,CODE_ERROR,"label name can't be an existing operation\n");
		return 0;
	}

	if (cmpNextWords(labelName,4,"macr","endmacr","entry","extern")){
		error(pState,CODE_ERROR,"label name can't be an instruction\n");
		return 0;
	}

	if (!isalpha(labelName[0])){
		error(pState,CODE_ERROR,"label name first character has to be an alphabetic character\n");
		return 0;
	}

	for (labelName++; *labelName != '\0'; labelName++){
		if (!isalnum(*labelName)){
			error(pState,CODE_ERROR,"label name can only include alphabetic characters and numbers\n");
			return 0;
		}
	}

	return 1;
}

/* checks if a given line has a label declaration.
 * if there is a label declaration, the function will malloc space for the label name,
 * and point the labelName to it. nextWord works the same as in cmpNextWord,
 * it gets pointed to the next word in the line.
 * if the name of the label is illegal, labelName will point to null. */
void isLabel(char *line,char **labelName,char **nextWord,pProgramState pState){
	char *word; /* points to the start of the label name */
	int nameLength; /* length of the name (\0 included in length) */

	*labelName = NULL;

	if (*line == '\0'){
		return;
	}

	skipLineWhitespaces(&line); /* skip whitespaces before word */
	if (*line == ':'){
		error(pState,CODE_ERROR,"colon can't appear before command\n");
		return;
	}

	word = line; /* save start of word */
	skipLineWord(&line); /* go to end of the word */

	if (*line == ':'){ /* label found */
		line++;
		nameLength = line - word; /* \0 included in length */
		if (nameLength > FULL_LABEL_MAX_SIZE){
			error(pState,CODE_ERROR,"label name can't be above %d characters\n",LABEL_MAX_SIZE);
			return;
		}

		*labelName = (char*)malloc(nameLength * sizeof(char));
		if (*labelName == NULL){
			error(pState,MALLOC_FAILURE,"label name malloc failed\n");
			return;
		}

		strncpy(*labelName,word,nameLength-1);
		(*labelName)[nameLength-1] = '\0';

		skipLineWhitespaces(&line); /* skip whitespaces after word */
		*nextWord = line;

		/* check if label name is legal */
		if (!isLegalLabelName(*labelName,pState)){
			free(*labelName);
			*labelName = NULL;
			return;
		}

	}
}

/* similar to atoi, but also checks the syntax of the string.
 * sets the number parameter to the number inside the string.
 * also sets the length parameter to the length of the number inside the string,
 * so the user of the function can skip the number.
 * this function only supports integers/longs, so floating point numbers will result in an error. */
int getNumber(char *str,long *number,int *length,pProgramState pState){
	int sign = 1; /* sign of the number, 1 if positive, -1 if negative */
	*number = 0;
	*length = 0;

	switch(*str){
	case '-':
		sign = -1; /* there is no break here intentionally */
	case '+':
		str++;
		(*length)++;
		break;
	}

	/* build the number, also look for syntax errors */
	for (; !isspace(*str) && *str != ',' && str != NULL; str++, (*length)++ ){

		if (*str == '.'){
			error(pState,CODE_ERROR,"no support for float numbers\n");
			return 0;
		}

		if (*str == '+' || *str == '-'){
			error(pState,CODE_ERROR,"sign has to come before number\n");
			return 0;
		}

		if (!isdigit(*str)){
			error(pState,CODE_ERROR,"can't push non numbers into memory\n");
			return 0;
		}

		*number = *number * 10 + *str - '0';

	}

	*number *= sign;
	return 1;
}

/* this function checks what operation was called in a given line,
 * and returns the matching operation struct.
 * if no operation was found, the function will return the null operation,
 * which has a code of -1.
 * for convenience, here are the common hex codes translated to binary:
 * 0xF = 1111 0x7 = 0111 0x6 = 0110 0x0 = 0000 */
Operation getOperationCode(char *line){

	Operation *operations;
	int i;

	getOperationsTable(&operations);
	for (i = 0; operations[i].code != -1; i++){
		if (cmpNextWord(line,operations[i].name,NULL)){
			return operations[i];
		}
	}

	return operations[i]; /* operation code wasn't found, no such operation exists */
}

/* this function opens the file with the given fileName and extension,
 * in the given mode (same as in fopen, like r,w+,etc).
 * the function also mallocs space for the full name of the file,
 * and points the fullName parameter to it.
 * finally, the function returns a pointer to the file that was opened,
 * or null if there was an error.
 * the full name is equal to fileName + extension. */
FILE *openFile(char *fileName,char *extension,char *mode,char **fullName,pProgramState pState){
	int nameLength,extensionLength,fullNameLength; /* length of the name,extension, and full name respectively */
	FILE *fp; /* pointer to the file that is to be opened */

	nameLength = strlen(fileName);
	extensionLength = strlen(extension);

	fullNameLength = nameLength + extensionLength + 1; /* \0 is included in the length */
	*fullName = (char*)malloc(fullNameLength * sizeof(char));

	if (*fullName == NULL){
		error(pState,MALLOC_FAILURE,"%s%s name malloc failed\n",fileName,extension);
		return NULL;
	}

	sprintf(*fullName,"%s%s",fileName,extension);

	fp = fopen(*fullName,mode);
	if (fp == NULL){
		error(pState,FILE_FAILURE,"failed to open file %s\n",*fullName);
		return NULL;
	}

	return fp;
}
