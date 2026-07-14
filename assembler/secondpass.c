/* third stage of the program.
 * assumes most of the syntax is valid, since the syntax checks are done in the first pass.
 * builds the second and third words for all the instructions, and does some checks
 * that are impossible to do in the first pass.
 * also writes all of the entries into the entry file.
 */

#include "secondpass.h"

/* builds the second and third words for the instructions.
 * the word parameter is the first word of the instruction that was built in the first pass.
 * the data string is a pointer to the first char/operand after the name of the operation.
 * the operand parameter can be either SOURCE_OPERAND or TARGET_OPERAND.
 * ic is the instruction counter of the second pass. it should be noted that it is different
 * from the instruction counter in the machine code image, which is the end of the program.
 * the externFile parameter is the file where all uses of external symbols are written.
 * the function returns 1 if it was successful, 0 otherwise. */
int buildOperandWord(ProgramData programData,Word word,char **data,int operand,unsigned long ic,FILE **externFile){
	int offset = (operand == SOURCE_OPERAND) ? SOURCE_METHOD_START : TARGET_METHOD_START; /* offset to the start of the addressing method field */
	int registerOffset = (operand == SOURCE_OPERAND) ? 3 : 0; /* offset to the register field in the second/third words (from the end of A.R.E field) */

	pMachineCodeImage machineCodeImage = programData.machineCodeImage; /* pointer to machine code image */
	pSymbolList symbolsTable = programData.symbolsTable; /* pointer to symbols table */
	pProgramState pState = programData.pState; /* pointer to program state */

	Word nextWord; /* the word the function builds */

	if (getWordBit(word,offset + INSTANT_ADDRESSING)){
		long number; /* the number used in the instant addressing */
		int length; /* length of the number in the line */

		getNumber(++(*data),&number,&length,pState);
		*data += length;
		nextWord = buildWord(ABSOLUTE_ADDRESSING,number,pState);

	} else if (getWordBit(word,offset + DIRECT_ADDRESSING)){

		pSymbolNode usedSymbol; /* the symbol used in the direct addressing */

		usedSymbol = getSymbolNode(symbolsTable,data);
		if (usedSymbol == NULL){
			error(pState,CODE_ERROR,"undeclared label used (%s operand)\n",operand == SOURCE_OPERAND ? "source" : "target");
			return 1;
		}

		if (usedSymbol->signType == EXTERNAL_SIGN){
			nextWord = buildWord(EXTERNAL_ADDRESSING,0,pState);
			externWord(externFile,programData.fileName,usedSymbol,ic + PROGRAM_START_INDEX,pState);
			checkProgramState(programData,0);
		} else {
			nextWord = buildWord(RELATIVE_ADDRESSING,usedSymbol->dataLocation,pState);
		}

	} else if (getWordBit(word,offset + INDIRECT_REGISTER_ADDRESSING)){

		int registerNumber; /* the number of the register used in the indirect register addressing */

		*data += 2; /* skip the *r characters */
		registerNumber = *((*data)++) - '0';

		nextWord = buildWord(ABSOLUTE_ADDRESSING,registerNumber << registerOffset,pState);

	} else if (getWordBit(word,offset + DIRECT_REGISTER_ADDRESSING)){
		int registerNumber; /* the number of the register used in the direct register addressing */

		(*data)++; /* skip the r character */
		registerNumber = *((*data)++) - '0';

		nextWord = buildWord(ABSOLUTE_ADDRESSING,registerNumber << registerOffset,pState);

	} else {
		return 0;
	}

	machineCodeImage->image[ic + PROGRAM_START_INDEX] = nextWord; /* insert the new word into the machine code image */
	return 1;
}

/* main function of the file. does just as described in the start of the file.
 * takes the program data from the first pass and returns it by the end. */
ProgramData secondpass(ProgramData programData){

	FILE *objectFile = NULL; /* the file where the machine code image will be written */
	char *objectName; /* the full name of the object file */

	char line[FULL_LINE_MAX_SIZE]; /* line buffer */
	char *label; /* points to the start of line, checks if there is a label in the line */
	char *entryLine; /* the next word after the .entry word (if the line has that word) */
	char *data; /* the next word after the label declaration (if a line has it) */

	unsigned long ic = 0; /* the second pass' instruction counter */
	Word word; /* the first word of the current instruction */
	Word nextWord; /* the next word to be built, only used for the second word when an instruction shares the second and third words */

	pPreprocessorInfo pInfo = programData.pInfo; /* pointer to preprocessor info */
	pProgramState pState = programData.pState; /* pointer to program state */
	pMachineCodeImage machineCodeImage = programData.machineCodeImage; /* pointer to machine code image */
	pDataImage dataImage = programData.dataImage; /* pointer to data image */
	pSymbolList symbolsTable = programData.symbolsTable; /* pointer to symbols table */

	rewind(pInfo->amFile);

	pState->lineNumber = 0;

	while (readLine(pInfo->amFile,line,pState) && line[0] != '\0') {
		/* skip label if needed */
		label = data = line;

		skipLineWhitespaces(&label);
		skipLineWord(&label);
		if (*label == ':'){
			data = label + 1;
		}

		skipLineWhitespaces(&data);
		if (*data == '\0'){ /* blank line */
			continue;
		}

		if (cmpNextWords(data,3,".data",".string",".extern")){
			continue;
		} else if (cmpNextWord(data,".entry",&entryLine)){
			enterLabel(&programData.entryFile,programData.fileName,symbolsTable,entryLine,pState);
			checkProgramState(programData,programData);
			continue;
		}

		skipLineWord(&data); /* skip operation name */
		skipLineWhitespaces(&data);
		word = machineCodeImage->image[ic + PROGRAM_START_INDEX];
		ic++;

		/* check if shared word */
		if (sharingWord(word)){

			int sourceNum,targetNum;

			data += getWordBit(word,SOURCE_METHOD_START + DIRECT_REGISTER_ADDRESSING) ? 1 : 2;
			sourceNum = *(data++) - '0';

			skipLineWhitespaces(&data);
			data++;
			skipLineWhitespaces(&data);

			data += getWordBit(word,TARGET_METHOD_START + DIRECT_REGISTER_ADDRESSING) ? 1 : 2;
			targetNum = *data - '0';

			nextWord = buildWord(ABSOLUTE_ADDRESSING,(sourceNum << 3) + targetNum,pState);
			machineCodeImage->image[(ic++) + PROGRAM_START_INDEX] = nextWord;

			continue;
		};

		/* build source word */
		ic += buildOperandWord(programData,word,&data,SOURCE_OPERAND,ic,&programData.externFile);
		checkProgramState(programData,programData);

		/* skip comma */
		skipLineWhitespaces(&data);
		skipLineSeparateChar(&data);
		skipLineWhitespaces(&data);

		/* build target word */
		ic += buildOperandWord(programData,word,&data,TARGET_OPERAND,ic,&programData.externFile);
		checkProgramState(programData,programData);
	}

	if (pState->state != VALID){ /* don't create object file if any errors were discovered */
		return programData;
	}

	/* print machine code image into object file */
	objectFile = openFile(programData.fileName,".ob","w",&objectName,pState);
	safeFree(objectName); /* don't need the file name */
	checkProgramState(programData,programData);

	if (objectFile != NULL){
		int i,j;

		fprintf(objectFile,"%-3lu %lu\n",machineCodeImage->ic,dataImage->dc);

		for (i = 0; i < machineCodeImage->ic; i++){
			fprintf(objectFile,"%04d  %04x\n",i + PROGRAM_START_INDEX,machineCodeImage->image[i + PROGRAM_START_INDEX].number);
		}

		for (j = 0; j < dataImage->dc; j++){
			fprintf(objectFile,"%04d  %04x\n",i + j + PROGRAM_START_INDEX,dataImage->image[j].number);
		}

		fclose(objectFile);
	}

	return programData;
}
