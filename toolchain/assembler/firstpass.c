/* the second stage of the program.
 * builds the first word for every instruction, gets all the symbols declared in the file,
 * builds all of the data image, and checks the syntax.
 */

#include "firstpass.h"

/* frees all of the program data, including the preprocessor data */
void freeProgramData(ProgramData programData){

	printf("freeing program data\n");

	safeClose(programData.pInfo->amFile);
	safeClose(programData.entryFile);
	safeClose(programData.externFile);

	safeFree(programData.dataImage);
	safeFree(programData.machineCodeImage);
	safeFree(programData.pState);

	if (programData.symbolsTable != NULL){
		pSymbolNode symbolNode = programData.symbolsTable->head;
		pSymbolNode tempSymbolNode;

		for (; symbolNode != NULL; symbolNode = tempSymbolNode){
			tempSymbolNode = symbolNode->next;
			free(symbolNode->name);
			free(symbolNode);
		}

		free(programData.symbolsTable);
	}

	freePreprocessorInfo(programData.pInfo);
}

/* calls freeProgramData and exits with the given code */
void exitProgram(ProgramData programData,int code){
	printf("exiting program\n");
	freeProgramData(programData);
	exit(code);
}

/* main function of the file. does just as described in the start of the file.
 * takes the preprocessor info and returns the program data */
ProgramData firstpass(pPreprocessorInfo pInfo){

	char line[FULL_LINE_MAX_SIZE]; /* line buffer */
	char *word; /* word the function is currently reading */
	char *labelName; /* name of the current label */
	char *data; /* the next word after a data instruction (data,string,entry,extern) */

	pProgramState pState; /* pointer to program state */
	pSymbolList symbolsTable; /* pointer to symbols table */
	pDataImage dataImage; /* pointer to data image */
	pMachineCodeImage machineCodeImage; /* pointer to machine code image */

	ProgramData programData; /* program data struct */

	pSymbolNode symbolNode; /* current symbol node, used for updating their data location after first pass */

	pState = programData.pState = createProgramState();
	if (pState == NULL){
		printf("program state malloc failed\n");
		exit(1);
	}

	symbolsTable = programData.symbolsTable = createSymbolList(pState);
	dataImage = programData.dataImage = createDataImage(pState);
	machineCodeImage = programData.machineCodeImage = createMachineCodeImage(pState);

	programData.pInfo = pInfo;

	programData.entryFile = NULL;
	programData.externFile = NULL;

	if (pState->state == MALLOC_FAILURE){
		freeProgramData(programData);
		exit(1);
	}

	printf("reading file %s\n",pInfo->amName);

	while (readLine(pInfo->amFile,line,pState) && line[0] != '\0') {
		word = line;

		skipLineWhitespaces(&word);
		if (*word == '\0'){
			continue;
		}

		if (*word == ','){
			error(pState,CODE_ERROR,"comma can't appear before command\n");
			continue;
		}

		isLabel(line,&labelName,&word,pState);
		checkProgramState(programData,programData);

		if (cmpNextWord(word,".data",&data)){

			if (labelName != NULL){
				addNode(symbolsTable,pInfo->macroList,labelName,dataImage->dc,DATA_SIGN,pState);
			}

			pushData(dataImage,data,pState);

		} else if (cmpNextWord(word,".string",&data)){

			if (labelName != NULL){
				addNode(symbolsTable,pInfo->macroList,labelName,dataImage->dc,DATA_SIGN,pState);
			}

			pushString(dataImage,data,pState);

		} else if (cmpNextWord(word,".extern",&data)){
			safeFree(labelName);
			externLabel(symbolsTable,pInfo->macroList,data,pState);

		} else if (cmpNextWord(word,".entry",&data)){
			/* ignore entry on first pass */
			safeFree(labelName);

		} else {
			Operation operation;
			Word firstWord;

			operation = getOperationCode(word);
			if (labelName != NULL){
				addNode(symbolsTable,pInfo->macroList,labelName,machineCodeImage->ic + PROGRAM_START_INDEX,CODE_SIGN,pState);
			}

			if (operation.code == -1){
				error(pState,CODE_ERROR,"illegal operation\n");
				continue;
			}

			/* analyze operation here, and build first word */
			firstWord = buildFirstWord(operation,data,pState);
			machineCodeImage->image[machineCodeImage->ic + PROGRAM_START_INDEX] = firstWord;

			machineCodeImage->ic += getWordSize(operation,firstWord);
		}

		checkProgramState(programData,programData);
	}

	/* add ic to all data symbols while keeping the list ordered */
	for (symbolNode = symbolsTable->head; symbolNode != NULL; symbolNode = symbolNode->next){
		if (symbolNode->signType == DATA_SIGN){
			symbolNode->dataLocation += machineCodeImage->ic + PROGRAM_START_INDEX;
		}
	}

	return programData;
}
