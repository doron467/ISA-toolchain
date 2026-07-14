/* this program has functions that support handling labels,
 * like storing information about them inside linked lists,
 * extern and entery labels, and more. */

#include "symbols_table.h"

/* creates a symbol list, sets the head and tail to null, and returns a pointer to the list */
pSymbolList createSymbolList(pProgramState pState){
	pSymbolList list; /* the returned list */

	list = (pSymbolList)malloc(sizeof(SymbolList));
	if (list == NULL){
		error(pState,MALLOC_FAILURE,"sign list malloc failed\n");
		return NULL;
	}

	list->head = NULL;
	list->tail = NULL;
	return list;
}

/* creates a symbol node with the given information in the parameters,
 * inserts the node to the end of the list, and returns a pointer to the node.
 * the macro list is the list of macros from the preprocessor, which is used to make sure
 * no macros and symbols share names. the variables of the node will be set to the
 * matching given parameters. if any error occurred, the function will return null. */
pSymbolNode addNode(pSymbolList list,pMacroList macroList,char *name,unsigned int dataLocation,int signType,pProgramState pState){
	pSymbolNode node; /* starts as the head of the symbol list, then becomes the new node */
	pMacroNode macroNode; /* the current macro node, used to compare the name of the symbol to the macros */

	/* make sure there isn't already a symbol with the same name */
	for (node = list->head; node != NULL; node = node->next){
		if (strcmp(node->name,name) == 0){
			error(pState,CODE_ERROR,"multiple definitions of %s\n",name);
			return NULL;
		}
	}

	/* make sure there isn't any macro with the same name */
	for (macroNode = macroList->head; macroNode != NULL; macroNode = macroNode->next){
		if (strcmp(macroNode->name,name) == 0){
			error(pState,CODE_ERROR,"macros and labels cannot share names\n");
			return NULL;
		}
	}

	/* create the new node */
	node = (pSymbolNode)malloc(sizeof(SymbolNode));
	if (node == NULL){
		error(pState,MALLOC_FAILURE,"sign node malloc failed\n");
		return NULL;
	}

	/* insert the node to the end of the list */
	if (list->head == NULL){
		list->head = node;
		list->tail = node;
	} else {
		list->tail->next = node;
		list->tail = node;
	}

	/* set all the node's variables */
	node->name = name;
	node->dataLocation = dataLocation;
	node->signType = signType;
	node->next = NULL;
	return node;
}

/* returns a pointer to the symbol node with the matching given name.
 * the name parameter will then be set to the next word.
 * if no node was found, the function returns null. */
pSymbolNode getSymbolNode(pSymbolList list,char **name){
	pSymbolNode node; /* the returned node */

	/* search for the matching symbol node */
	for (node = list->head; node != NULL && !cmpNextWord(*name,node->name,NULL); node = node->next);
	cmpNextWord(*name,"0",name);; /* update name to the next word */
	return node;
}

/* adds an external node to the given list. used for extern lines.
 * note that the line parameter should point to the next character/word after the extern word. */
void externLabel(pSymbolList list,pMacroList macroList,char *line,pProgramState pState){
	char *label; /* pointer to the start of the name of the label in the line */
	char *labelName; /* copy of the label name for storing it */
	int labelLength; /* length of the label's name, \0 included */

	skipLineWhitespaces(&line);
	if (*line == '\0'){
		error(pState,CODE_ERROR,"missing label name after extern\n");
		return;
	}

	label = line;
	skipLineWord(&line);

	labelLength = line - label + 1;
	labelName = (char*)malloc(labelLength * sizeof(char));
	if (labelName == NULL){
		error(pState,MALLOC_FAILURE,"extern label name malloc failed\n");
		return;
	}

	strncpy(labelName,label,labelLength - 1);
	labelName[labelLength - 1] = '\0';

	addNode(list,macroList,labelName,0,EXTERNAL_SIGN,pState);

	skipLineWhitespaces(&line);
	if (*line != '\0'){
		error(pState,CODE_ERROR,"extraneous text after extern label name\n");
		return;
	}

}

/* writes the label into the given entryFile. if the file wasn't created yet,
 * it will create the file with the given fileName.
 * entryName is a pointer to the next word after the entry word.
 * this function is called for entry lines. */
void enterLabel(FILE **entryFile,char *fileName,pSymbolList list,char *entryLine,pProgramState pState){
	pSymbolNode symbolNode; /* the matching symbol node of the entry label */
	char *entryName = entryLine; /* pointer to the name of the entry label in the line */

	/* create the entry file if needed */
	if (*entryFile == NULL){
		char *fullName = NULL; /* full name of the entry file */
		*entryFile = openFile(fileName,".ent","w",&fullName,pState);
		safeFree(fullName); /* full name isn't actually needed, so it can be freed */
		if (*entryFile == NULL){ /* return if an error occurred */
			return;
		}
	}

	symbolNode = getSymbolNode(list,&entryLine); /* get the matching symbol node, entry line gets increased to next word */

	/* make sure the line ends properly */
	if (entryLine != NULL){
		skipLineWhitespaces(&entryLine);
		if (*entryLine != '\0'){
			error(pState,CODE_ERROR,"extraneous text after entry name\n");
			return;
		}
	}


	if (symbolNode == NULL){
		/* get entry name length */
		char *entryNameEnd = entryName; /* points to the end of the entry name */
		int entryLength; /* length of the entry label's name */
		skipLineWord(&entryNameEnd);
		entryLength = entryNameEnd - entryName;
		/* limit print to only the name of the entry */
		error(pState,CODE_ERROR,"undeclared label %.*s called in entry\n",entryLength,entryName);
		return;
	}

	fprintf(*entryFile,"%s %u\n",symbolNode->name,symbolNode->dataLocation);

}

/* writes the label into the given externFile. if the file wasn't created yet,
 * it will create the file with the given fileName.
 * node is a pointer to the matching node of the symbol to write into the file.
 * ic is the instruction counter from the machine code image, since this file doesn't
 * have access to the memory image.
 * this function is called whenever an external symbol is used inside an operation */
void externWord(FILE **externFile,char *fileName,pSymbolNode node,unsigned long ic,pProgramState pState){

	/* create the file if needed */
	if (*externFile == NULL){
		char *fullName = NULL; /* full name of the file */
		safeFree(fullName); /* full name isn't actually needed, so it can be freed */
		*externFile = openFile(fileName,".ext","w",&fullName,pState);
		if (*externFile == NULL){
			return;
		}
	}

	fprintf(*externFile,"%s %04lu\n",node->name,ic);
}
