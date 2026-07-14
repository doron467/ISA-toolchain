/* the first stage of the assembler.
 * opens the .as file, stores the macros,
 * creates the .am file, copies the .as file
 * into the .am file while filtering out the comments
 * and laying out the macros. the preprocessor info is then returned.
 */

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <string.h>
#include "utils.h"

typedef struct _LineNode *pLineNode;
typedef struct _MacroNode *pMacroNode;
typedef struct _MacroList *pMacroList;
typedef struct _PreprocessorInfo *pPreprocessorInfo;

typedef struct _PreprocessorInfo{ /* a struct to store information about the preprocessor */
	char status; /* 0 for normal, 1 if an error was found */
	FILE *asFile; /* a pointer to the .as file */
	FILE *amFile; /* a pointer to the .am file */
	char *asName; /* the full name of the .as file, should be freed at end of program */
	char *amName; /* the full name of the .am file, should be freed at end of program */
	pMacroList macroList; /* pointer to the macro list of the program */
} PreprocessorInfo;

typedef struct _LineNode{ /* a struct for storing macro lines inside a linked list */
	char *line; /* the line to be stored, should be freed at end of program */
	pLineNode next; /* pointer to next line node */
} LineNode;

typedef struct _MacroNode{ /* a struct for storing macros inside a linked list */
	char *name; /* name of macro */
	pLineNode head; /* pointer to line node of the first macro's line */
	pLineNode tail; /* pointer to line node of the last macro's line */
	pMacroNode next; /* pointer to the next macro node */
} MacroNode;

typedef struct _MacroList{ /* a linked list for storing macro nodes */
	pMacroNode head; /* pointer to first macro node */
	pMacroNode tail; /* pointer to last macro node */
} MacroList;

void freePreprocessorInfo(pPreprocessorInfo pInfo); /* frees all the preprocessor info */

/* main function of the file. does just as described in the start of the file */
pPreprocessorInfo preprocess(char* fileName);

#endif
