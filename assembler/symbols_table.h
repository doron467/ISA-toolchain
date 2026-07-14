/* this program has functions that support handling labels,
 * like storing information about them inside linked lists,
 * extern and entery labels, and more. */

#ifndef SYMBOLS_TABLE_H
#define SYMBOLS_TABLE_H

#include "preprocessor.h"
#include "utils.h"

/* different signs for labels, DATA_SIGN is for data and string labels,
 * EXTERNAL_SIGN is for external labels,
 * and CODE_SIGN is for operation labels */
enum {DATA_SIGN,EXTERNAL_SIGN,CODE_SIGN};

typedef struct _SymbolNode *pSymbolNode;
typedef struct _SymbolList *pSymbolList;

/* a struct for storing information about symbol labels */
typedef struct _SymbolNode{
	unsigned int dataLocation; /* the location of the symbol inside the ram */
	int signType; /* the type of the sign of the symbol, can be DATA_SIGN,EXTERNAL_SIGN,CODE_SIGN */
	char *name; /* name of the label */
	pSymbolNode next; /* pointer to the next node in the linked list */
} SymbolNode;

/* a struct for a linked list of symbol nodes */
typedef struct _SymbolList{
	pSymbolNode head; /* pointer to the head of the list */
	pSymbolNode tail; /* pointer to the tail of the list */
} SymbolList;

/* creates a pointer to a symbol list, sets the head and tail to null, and returns the pointer */
pSymbolList createSymbolList(pProgramState pState);

/* creates a symbol node with the information given, inserts the node to the list and returns a pointer to the node
 * macroList is the list of macros from the preprocessor. it is used to check no labels and macros share names.
 * the other parameters are for setting the matching variables of the symbol node. */
pSymbolNode addNode(pSymbolList list,pMacroList macroList,char *name,unsigned int dataLocation,int signType,pProgramState pState);

/* returns a pointer to the symbol node with the given name.
 * the name parameter will then be set to the next word.
 * if no node was found, the function returns null */
pSymbolNode getSymbolNode(pSymbolList list,char **name);

/* adds an external node to the given list, called for extern lines.
 * label is a pointer to the next character/word after the extern word. */
void externLabel(pSymbolList list,pMacroList macroList, char *label, pProgramState pState);

/* writes the label into the given entryFile. if the file wasn't created yet,
 * it will create the file with the given fileName.
 * entryName is a pointer to the next word after the entry word.
 * this function is called for entry lines. */
void enterLabel(FILE **entryFile,char *fileName,pSymbolList list,char *entryName,pProgramState pState);

/* writes the label into the given externFile. if the file wasn't created yet,
 * it will create the file with the given fileName.
 * node is a pointer to the matching node of the symbol to write into the file.
 * ic is the instruction counter from the machine code image.
 * this function is called whenever an external symbol is used inside an operation */
void externWord(FILE **externFile,char *fileName,pSymbolNode node,unsigned long ic,pProgramState pState);

#endif
