/* the first stage of the assembler.
 * opens the .as file, stores the macros,
 * creates the .am file, copies the .as file
 * into the .am file while filtering out the comments
 * and laying out the macros. the preprocessor info is then returned.
 */

#include "preprocessor.h"

/* ends the preprocessor. if code is 0, the preprocessor was successful, otherwise, the preprocessor failed */
void endPreprocessor(pPreprocessorInfo pInfo,pProgramState pState,int code);

/* checks the state of the preprocessor and reacts accordingly.
 * should be called after every function call that can fail. */
#define checkPreprocessorState(pState,pInfo) { \
	if (pState->state == MALLOC_FAILURE) {endPreprocessor(pInfo,pState,1); exit(1);} \
	if (pState->state == FILE_FAILURE) {endPreprocessor(pInfo,pState,1); return 0;} \
}

/* creates a macro list struct, sets the head and tail to null, and returns a pointer to the struct */
pMacroList createMacroList(pProgramState pState){
	pMacroList list;

	list = (pMacroList)malloc(sizeof(MacroList));
	if (list == NULL){
		error(pState,MALLOC_FAILURE,"Macro List malloc failed\n");
		return NULL;
	}

	list->head = NULL;
	list->tail = NULL;
	return list;
}

/* creates a macro node with the given name, inserts it to the end of the given macro list,
 * and returns the created node. */
pMacroNode addMacroNode(pMacroList list,char *name,pProgramState pState){
	pMacroNode node; /* the new node to be created */

	/* check for multiple definitions of the macro */
	for (node = list->head; node != NULL; node = node->next){
		if (strcmp(node->name,name) == 0){
			error(pState,CODE_ERROR,"multiple definitions of macro %s\n",name);
			return NULL;
		}
	}

	/* create the macro node */
	node = (pMacroNode)malloc(sizeof(MacroNode));
	if (node == NULL){
		error(pState,MALLOC_FAILURE,"Macro node malloc failed\n");
		return NULL;
	}

	/* set all the node variables */
	node->name = name;
	node->next = NULL;
	node->head = NULL;
	node->tail = NULL;

	/* insert the node to the end of the list */
	if (list->head == NULL){
		list->head = node;
		list->tail = node;
	} else {
		list->tail->next = node;
		list->tail = node;
	}

	return node;
}

/* creates a line node from the given line by duplicating it into an allocated space,
 * and inserts it to the end of the given macro node's line linked list*/
pLineNode addLineNode(pMacroNode macro,char *line,pProgramState pState){
	pLineNode node; /* the new line node to be created */
	char *lineCopy; /* the copy of the line */
	unsigned int length; /* length of the line, \0 included */

	/* create the copy of the line */
	length = strlen(line) + 1;
	lineCopy = (char*)malloc(length * sizeof(char));
	if (lineCopy == NULL){
		error(pState,MALLOC_FAILURE,"Line malloc failed\n");
		return NULL;
	}

	strncpy(lineCopy,line,length - 1);
	lineCopy[length - 1] = '\0';

	/* create the new line node */
	node = (pLineNode)malloc(sizeof(LineNode));
	if (node == NULL){
		error(pState,MALLOC_FAILURE,"Line Node malloc failed\n");
		return NULL;
	}

	/* set all the line node's variables */
	node->next = NULL;
	node->line = lineCopy;

	/* insert the line node to the end of the macro node's line linked list */
	if (macro->head == NULL){
		macro->head = node;
		macro->tail = node;
	} else {
		macro->tail->next = node;
		macro->tail = node;
	}

	return node;
}

/* frees all the preprocessor info */
void freePreprocessorInfo(pPreprocessorInfo pInfo){
	pMacroList list = pInfo->macroList; /* the preprocessor's macro list */
	pMacroNode macroNode; /* the current macro node */
	pLineNode lineNode; /* the current line node */

	pMacroNode tempMacro; /* used to store the next macro node before freeing the current one */
	pLineNode tempLine; /* used to store the next line node before freeing the current one */

	printf("freeing preprocessor information\n");

	/* frees the macro list */
	for (macroNode = list->head; macroNode != NULL; macroNode = tempMacro){
		for (lineNode = macroNode->head; lineNode != NULL; lineNode = tempLine){
			tempLine = lineNode->next;
			free(lineNode->line);
			free(lineNode);
		}

		tempMacro = macroNode->next;
		free(macroNode->name);
		free(macroNode);
	}

	free(list);
	/* frees the file names */
	safeFree(pInfo->asName);
	safeFree(pInfo->amName);

	free(pInfo);
}

/* ends the preprocessor.
 * if code is 0, then the preprocessor was successful, and the function simply
 * closes the .as file and rewinds the .am file.
 * if code is 1, then the preprocessor failed and the program can't move to the next stage.
 * both files get closed, the .am file gets deleted, and the preproccessor info is freed. */
void endPreprocessor(pPreprocessorInfo pInfo,pProgramState pState,int code){
	if (pInfo->status == 1){ /* an error was already found and the preprocessor ended */
		return;
	}

	if (pInfo->asFile != NULL){
		fclose(pInfo->asFile);
		pInfo->asFile = NULL;
	}

	if (code == 0 && pState->state == VALID){
		rewind(pInfo->amFile);

	} else {

		/* delete the .am file */
		if (pInfo->amFile != NULL){
			fclose(pInfo->amFile);
			pInfo->amFile = NULL;

			remove(pInfo->amName);
		}

		pInfo->status = 1;

		if (pInfo->macroList != NULL){
			freePreprocessorInfo(pInfo);
		}

	}

	free(pState);
}

/* returns a pointer to the macro node struct called in the given line from the given macro list.
 * if no macro was found, or if there was other text beside the macro name, the function returns null. */
pMacroNode getMacro(pMacroList list,char *line){
	pMacroNode macro; /* the returned macro node */

	char *nextWord; /* the next word after the macro call */

	for (macro = list->head; macro != NULL; macro = macro->next){
		if (cmpNextWord(line,macro->name,&nextWord)){ /* macro with matching name found */
			break;
		}
	}

	/* check for extraneous text after macro call */
	if (nextWord != NULL){
		return NULL;
	}

	return macro;
}

/* reads the macros from the .as file and lays them inside the .am file wherever they are called.
 * if the function was successful, it returns 1. otherwise, the function ends the preprocessor and returns 0. */
int layMacros(pPreprocessorInfo pInfo,pProgramState pState){
	char line[FULL_LINE_MAX_SIZE]; /* line buffer */
	char *word; /* current word the function is reading */
	char *macroName; /* name of the current macro, gets memory allocated for it */

	int i;
	int nameLength; /* length of the current macro's name, \0 included */

	int inMacro = 0; /* 0 when outside macro, 1 when inside */
	pMacroList list; /* the macro list of the preprocessor */
	pMacroNode macro; /* current macro node */
	pLineNode lineNode; /* current line node */

	list = createMacroList(pState);
	checkPreprocessorState(pState,pInfo);

	pInfo->macroList = list;

	while(readLine(pInfo->asFile,line,pState) && line[0] != '\0') {

		if (line[0] == ';'){ /* ignore comments */
			continue;
		}

		if (inMacro){

			if (cmpNextWord(line,"endmacr",&word)){
				if (word != NULL){
					error(pState,CODE_ERROR,"extraneous text after end of macro\n");
				}

				inMacro = 0;
			} else {

				/* check macro doesn't end mid line */
				while (word != NULL){
					if (cmpNextWord(word,"endmacr",&word)){
						error(pState,CODE_ERROR,"extraneous text before end of macro\n");
						break;
					}
				}

				/* check for macro calls */
				if ((macro = getMacro(list,line)) != NULL){
					/*checkStatus(pInfo); */
					/* macro inside another macro, lay nested macro inside outer macro */
					for (lineNode = macro->head; lineNode != NULL; lineNode = lineNode->next){
						addLineNode(list->tail,lineNode->line,pState);
						checkPreprocessorState(pState,pInfo);
					}
				} else {
					/* save line in macro */
					addLineNode(list->tail,line,pState);
				}

			}

		} else {

			if (cmpNextWord(line,"macr",&word)){

				for (i = 0; word[i] != '\0' && !isspace(word[i]); i++);
				nameLength = i + 1;
				if (nameLength > FULL_LABEL_MAX_SIZE){
					error(pState,CODE_ERROR,"macro name can't be above %d characters\n",LABEL_MAX_SIZE);
					continue;
				}

				for (; word[i] != '\0' && isspace(word[i]); i++);
				if (word[i] != '\0'){
					error(pState,CODE_ERROR,"extraneous text after macro definition\n");
				}

				macroName = (char*)malloc(nameLength * sizeof(char));
				if (macroName == NULL){
					error(pState,MALLOC_FAILURE,"macro name malloc failed\n");
				}

				checkPreprocessorState(pState,pInfo);

				strncpy(macroName,word,nameLength - 1);
				macroName[nameLength - 1] = '\0';

				/* check the macro name follows the rules of the macro syntax */
				isLegalMacroName(macroName,pState);

				addMacroNode(list,macroName,pState);
				checkPreprocessorState(pState,pInfo);

				inMacro = 1;

			} else {
				/* check there is no macro definition in line */
				while (word != NULL){
					if (cmpNextWord(word,"macr",&word)){
						error(pState,CODE_ERROR,"extraneous text before macro definition\n");
						break;
					}
				}
				/*check for macro calls */
				if ((macro = getMacro(list,line)) != NULL){
					/* lay macro in file */
					for (lineNode = macro->head; lineNode != NULL; lineNode = lineNode->next){
						fprintf(pInfo->amFile,"%s",lineNode->line);
					}
				} else {
					/*copy line to .am file */
					fprintf(pInfo->amFile,"%s",line);
				}


			}

		}


	}

	return 1;
}

/* main function of the file. does just as described in the start of the file */
pPreprocessorInfo preprocess(char *fileName){

	pPreprocessorInfo pInfo;
	pProgramState pState;

	pState = createProgramState();
	if (pState == NULL){
		printf("program state malloc failed\n");
		exit(1);
	}

	pInfo = (pPreprocessorInfo)malloc(sizeof(PreprocessorInfo));
	if (pInfo == NULL){
		error(pState,MALLOC_FAILURE,"program info malloc failed\n");
		free(pState);
		exit(1);
	}

	pInfo->asFile = openFile(fileName,".as","r",&(pInfo->asName),pState);
	pInfo->amFile = openFile(fileName,".am","w+",&(pInfo->amName),pState);

	checkPreprocessorState(pState,pInfo);

	pInfo->status = 0;
	pInfo->macroList = NULL;

	printf("reading file %s\n",pInfo->asName);

	if(!layMacros(pInfo,pState)){
		printf("preprocessor failed\n");
		return NULL;
	}

	endPreprocessor(pInfo,pState,0);

	return pInfo;
}
