/* This program has generic functions that are used throughout the program.
 * these functions help with reading lines from files, analyzing input, handling errors, and more.
 * these functions are used almost everywhere, and they are mostly for convenience.
 * this file is included in all the other source files.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "../common/constants.h"
#include "../common/operations.h"

#define safeFree(p) if (p != NULL) {free(p);} /* this macro frees a point if it's not null */
#define safeClose(fp) if (fp != NULL) {fclose(fp);} /* this macro closes a file if it's not null */

enum {VALID,CODE_ERROR,FILE_FAILURE,MALLOC_FAILURE}; /* state codes for the program state */

typedef struct _ProgramState *pProgramState;

/* this struct is used to store information about the program state.
 * the state can be VALID,CODE_ERROR,FILE_FAILURE,MALLOC_FAILURE
 * when updating the program state, the new state has to be above the old state,
 * for example: if the current state is MALLOC_FAILURE, you can't change it to CODE_ERROR.
 * this is because the errors are ordered from least problematic to most.
 * if the state is CODE_ERROR, the file will still be read, but the program won't move to the next stage.
 * if the state is FILE_FAILURE, the file can't be analyzed anymore, so the program moves to the next source file.
 * if the state is MALLOC_FAILURE, then the program itself failed, and it will exit without reading the next files. */
typedef struct _ProgramState{
	unsigned long lineNumber; /* the number of the line the program is currently reading */
	int state; /* the state of the program, as explained above */
} ProgramState;



pProgramState createProgramState(); /* creates a pointer to a program state struct and sets its values */

/* updates the program state to the new state and prints the error. the errorStr can have formatting like printf */
void error(pProgramState pState,int newState,char *errorStr, ...);

/* the program divides characters into 3 types: whitespace characters, word characters, and separating characters
 * whitespace characters are as considered in the ANSI standard.
 * separating characters are commas (,) and colons (:).
 * all other characters are considered word characters (except \0 for obvious reasons). */
void skipLineWhitespaces(char **line); /* moves the line pointer to the next non whitespace character */
void skipLineWord(char **line); /* moves the line pointer to the next non word character (whitespace or separating character) */
void skipLineSeparateChar(char **line); /* if line points to a separating character, line moves to the next character */

/* fills the line array with the next line in the file, and increases the line counter in pState.
 * after the whole file was read, the first character of line will be \0, and that's the sign to stop reading the file.
 * this function always returns 1 for convenience. */
int readLine(FILE *fp,char *line,pProgramState pState);

/* compares the next word in the line string to the word given in the parameters.
 * if the two words are the same, the function returns 1, otherwise 0.
 * if the nextWord parameter isn't null, it will be pointed to the start of the next word in line string.
 * this function is mostly just an improved version of strcmp */
int cmpNextWord(char *line,char *word,char **nextWord);

/* this function lets you compare the next word in the line string to multiple different other words.
 * if the next word is equal to at least one of the given words, the function returns 1, otherwise 0.
 * the wordsAmount parameter is the number of words you want to compare the next word to.  */
int cmpNextWords(char *line,int wordsAmount, ...);

int isLegalMacroName(char *macroName,pProgramState pState); /* returns if a given macro's name is legal or not */

int isLegalLabelName(char *labelName,pProgramState pState); /* returns if a given label's name is legal or not */

/* this function checks if a given line has a label declaration or not.
 * if it does, then the function will allocate memory for the label name, and point the labelName parameter to it.
 * it will also set nextWord to the next word after the label declaration. */
void isLabel(char *line,char **labelName,char **nextWord,pProgramState pState);

/* similar to atoi, but also checks the syntax.
 * number is a pointer to a long. the function will set it to the matching number.
 * length will be set to the amount of characters the number takes in the string.
 * length is useful for skipping the characters this function already read. */
int getNumber(char *str,long *number,int *length,pProgramState pState);

/* checks if the next word in str is a valid register or not.
 * if it is a register, the function will return the number of the register (this ended up being useless),
 * otherwise -1. */
int isRegister(char *str);

/* returns the matching operation struct for the line. if no operation was found,
 * the function returns the operation "null", with a code of -1 */
Operation getOperationCode(char *line);

/* opens the file with the given file name and the extension.
 * mode is the mode to open the file in (same as in fopen),
 * and fullName is a pointer that will be set to the full name of the file (fileName + extension). */
FILE *openFile(char *fileName,char *extension,char *mode,char **fullName,pProgramState pState);

#endif
