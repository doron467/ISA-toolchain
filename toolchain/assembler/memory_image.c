/* this program has functions that help managing the machine code image and data image.
 * it is also responsible for building the words from the input. */

#include "memory_image.h"

/* creates a data image sturct, sets the data counter to 0 and returns a pointer to the struct */
pDataImage createDataImage(pProgramState pState){
	pDataImage image;
	image = (pDataImage)malloc(sizeof(DataImage));
	if (image == NULL){
		error(pState,MALLOC_FAILURE,"data image malloc failed\n");
		return NULL;
	}

	image->dc = 0;
	return image;
}

/* creates a machine code image struct, sets the instruction counter to 0, and returns a pointer to the struct */
pMachineCodeImage createMachineCodeImage(pProgramState pState){
	pMachineCodeImage image;
	image = (pMachineCodeImage)malloc(sizeof(MachineCodeImage));
	if (image == NULL){
		error(pState,MALLOC_FAILURE,"machine code image malloc failed\n");
		return NULL;
	}
	image->ic = 0;
	return image;
}

/* builds a word from a number pushed into data and returns the word */
Word numberToWord(long number){
	Word word; /* the returned word */
	short isNegative = number < 0; /* 1 if number is negative, 0 otherwise */
	long absNumber = isNegative ? -number : number; /* the absolute value of number */

	/* translate absNumber into binary */
	int i;
	short bit;

	word.number = 0;
	for (i = 0; i < WORD_LENGTH && absNumber > 0; i++){
		bit = (absNumber % 2 == 0) ? 0 : 1;
		absNumber /= 2;
		setWordBit(word,i,bit);
	}

	if (isNegative){
		/* invert all bits and add 1 to make number negative */
		word.number = (~word.number) + 1;
	}

	return word;
}

/* pushes a number into the data image */
void pushNumber(pDataImage image,long number,pProgramState pState){
	if (image->dc < MEMORY_IMAGE_SIZE){

		if (number > DATA_NUMBER_MAX_SIZE){
			error(pState,CODE_ERROR,"numbers pushed into data can't be above %d\n",DATA_NUMBER_MAX_SIZE);
		} else if (number < DATA_NUMBER_MIN_SIZE){
			error(pState,CODE_ERROR,"numbers pushed into data can't be below %d\n",DATA_NUMBER_MIN_SIZE);
		}

		image->image[image->dc++] = numberToWord(number);
	}
}

/* pushes a list of numbers into the data image. data string should point to the first
 * character/number after the data instruction */
int pushData(pDataImage image,char *data,pProgramState pState){
	int i;
	int next = 1; /* how many characters to skip over in the string every iteration */
	char c; /* the next character in the string */

	int comma = 0; /* becomes 1 when a comma was read, and 0 when a number was read */
	long number; /* the current number that was read */
	int numbers = 0; /* the amount of numbers the function read */

	for (i = 0; (c = data[i]) != '\0'; i += next){
		next = 1;

		if (isspace(c)){ /* skip whitespaces */
			continue;
		}

		if (c == ','){

			if (comma){
				error(pState,CODE_ERROR,"multiple commas\n");
				break;
			}

			comma = 1;
			continue;
		}

		if (numbers == 0){ /* first number */
			if (comma){
				error(pState,CODE_ERROR,"comma shouldn't appear before first number\n");
				break;
			}
		} else { /* not first number */
			if (!comma){
				error(pState,CODE_ERROR,"numbers should be separated by a comma\n");
				break;
			}
		}

		/* check if number */
		if (getNumber(data + i,&number,&next,pState)){
			pushNumber(image,number,pState);
			numbers++;
			comma = 0;
		}

	}

	if (comma){
		error(pState,CODE_ERROR,"comma shouldn't appear after last number\n");
	}

	return numbers;
}

/* pushes a string into the data image, and returns the length of the string.
 * string should point to the first character after the string instruction */
int pushString(pDataImage image,char *str,pProgramState pState){
	int i;

	if (str[0] != '\"'){
		error(pState,CODE_ERROR,"string should start with \"\n");
	}

	for (i = 1; str[i] != '\0' && str[i] != '"'; i++){
		pushNumber(image,str[i],pState);
	}

	if (str[i] != '\"'){
		error(pState,CODE_ERROR,"string should end with \"\n");
	}

	pushNumber(image,'\0',pState);

	return i - 1;
}

/* sets the field in the word that starts at fieldStart and ends at fieldEnd to all zeros,
 * except the bit in the position of activeBit, which is set to 1 */
void setField(pWord word,int fieldStart,int fieldEnd,int activeBit){
	for (; fieldStart <= fieldEnd; fieldStart++){
		setWordBit(*word,fieldStart,fieldStart == activeBit);
	}
}

/* returns the addressing method used in the given operands string */
int getAddressingMethod(char *operands,pProgramState pState){
	if (*operands == '#'){
		return INSTANT_ADDRESSING;
	} else if (*operands == '*'){
		/* need to check if register is valid */
		if (isRegister(operands + 1) == -1){
			error(pState,CODE_ERROR,"after * needs to come a valid register\n");
		}

		return INDIRECT_REGISTER_ADDRESSING;
	} else if (isRegister(operands) != -1){
		return DIRECT_REGISTER_ADDRESSING;
	} else {
		return DIRECT_ADDRESSING;
	}
}

/* builds the first word of the instruction from the given operands.
 * operation is the matching operation struct of the instruction. */
Word buildFirstWord(Operation operation,char *operands,pProgramState pState){
	Word word; /* the returned word */
	int opCode = operation.code; /* the code of the given operation */
	int sourceMethod; /* the source method of the instruction */
	int targetMethod; /* the target method of the instruction */
	int i;

	/* set A.R.E field */
	setField(&word,ARE_START,ARE_END,ABSOLUTE_ADDRESSING);

	/* set operation code */
	for (i = OP_CODE_START; i < OP_CODE_END; i++){
		setWordBit(word,i,opCode % 2);
		opCode /= 2;
	}

	if (operation.targetMethods == 0){ /* if there is no target method, there is also no source method */
		setField(&word,TARGET_METHOD_START,TARGET_METHOD_END,-1);
		setField(&word,SOURCE_METHOD_START,SOURCE_METHOD_END,-1);
		if (operands == NULL){
			return word;
		}

		skipLineWhitespaces(&operands);
		if (*operands != '\0'){
			error(pState,CODE_ERROR,"extraneous text after end of command\n");
		}
		return word;
	}

	if (operands == NULL){
		error(pState,CODE_ERROR,"missing operands\n");
		return word;
	}

	skipLineWhitespaces(&operands);

	/* set source addressing method */
	if (operation.sourceMethods != 0){ /* if there is a source method, then the first operand is the source */

		if (*operands == ','){
			error(pState,CODE_ERROR,"comma before first operator\n");
			return word;
		}

		sourceMethod = getAddressingMethod(operands,pState);
		setField(&word,SOURCE_METHOD_START,SOURCE_METHOD_END,SOURCE_METHOD_START + sourceMethod);

		if (!isAddressingMethod(operation.sourceMethods,sourceMethod)){
			error(pState,CODE_ERROR,"unmatching source addressing method\n");
			return word;
		}

		skipLineWord(&operands);
		skipLineWhitespaces(&operands);
		/* if there is a source operand, there must also be a target operand */
		if (*operands == '\0'){
			error(pState,CODE_ERROR,"missing operand\n");
			return word;
		} else if (*operands == ':'){
			error(pState,CODE_ERROR,"colon can't appear in command\n");
			return word;
		} else if (*operands != ','){
			error(pState,CODE_ERROR,"missing comma between operands\n");
			return word;
		}

		operands++;
	} else {
		setField(&word,SOURCE_METHOD_START,SOURCE_METHOD_END,-1);
	}

	/* set target addressing method */
	skipLineWhitespaces(&operands);
	if (*operands == ','){
		if (operation.sourceMethods == 0){
			error(pState,CODE_ERROR,"comma before first operator\n");
		} else {
			error(pState,CODE_ERROR,"multiple commas between operators\n");
		}

		return word;
	}

	if (*operands == '\0'){
		error(pState,CODE_ERROR,"missing operand\n");
		return word;
	}

	targetMethod = getAddressingMethod(operands,pState);
	setField(&word,TARGET_METHOD_START,TARGET_METHOD_END,TARGET_METHOD_START + targetMethod);

	if (!isAddressingMethod(operation.targetMethods,targetMethod)){
		error(pState,CODE_ERROR,"unmatching target addressing method\n");
		return word;
	}

	skipLineWord(&operands);
	skipLineWhitespaces(&operands);
	if (*operands != '\0'){
		error(pState,CODE_ERROR,"extraneous text after end of command\n");
	}

	return word;
}

/* builds a word with the given A.R.E field, with the rest of the bits representing the given value, and returns the word.
 * used for building the second and third words. */
Word buildWord(int areField,long value,pProgramState pState){
	Word word; /* the returned word */
	int i;
	short isNegative = value < 0; /* 1 if negative, 0 otherwise */

	if (value > NUMBER_MAX_SIZE){
		error(pState,CODE_ERROR,"number can't be above %d\n",NUMBER_MAX_SIZE);
		return word;
	}

	if (value < NUMBER_MIN_SIZE){
		error(pState,CODE_ERROR,"number can't be below %d\n",NUMBER_MIN_SIZE);
		return word;
	}

	/* build word */
	for (i = ARE_END + 1; i < WORD_LENGTH; i++){
		setWordBit(word,i,value % 2);
		value /= 2;
	}

	if (isNegative){
		word.number = (~word.number) + (1 << (ARE_END + 1));
	}

	/* only set the A.R.E field AFTER the number, so negating the number when it's negative won't ruin the A.R.E field */
	setField(&word,ARE_START,ARE_END,areField);

	return word;
}

/* returns how many words are needed for a given instruction, from the operation and first word */
int getWordSize(Operation operation,Word word){

	if (operation.sourceMethods != 0 && operation.targetMethods != 0){
		if (sharingWord(word)){
			return 2;
		}
		return 3;
	}

	if (operation.sourceMethods == 0 && operation.targetMethods == 0){
		return 1;
	}

	return 2;
}
