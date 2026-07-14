/* the second stage of the program.
 * builds the first word for every instruction, gets all the symbols declared in the file,
 * builds all of the data image, and checks the syntax.
 */

#ifndef FIRSTPASS_H
#define FIRSTPASS_H

#include "memory_image.h"
#include "preprocessor.h"
#include "symbols_table.h"
#include "utils.h"

typedef struct _ProgramData{ /* holds data about the program */
	char *fileName; /* name of the file that the program is currently reading (without the extension) */
	FILE *entryFile; /* pointer to the entry file */
	FILE *externFile; /* pointer to the extern file */
	pProgramState pState; /* pointer to the program state */
	pSymbolList symbolsTable; /* pointer to the symbols table */
	pDataImage dataImage; /* pointer to the data image */
	pMachineCodeImage machineCodeImage; /* pointer to the machine code image */
	pPreprocessorInfo pInfo; /* pointer to the preprocessor info */
} ProgramData;

void freeProgramData(ProgramData programData); /* frees all of the program data, including the preprocessor info */
void exitProgram(ProgramData programData,int code); /* calls freeProgramData and exits with the given code */

/* checks the program state from the programData and reacts accordingly.
 * returnVal is the value returned in case of file failure. */
#define checkProgramState(programData,returnVal) { \
	/* check for ram overflow */ \
	if (programData.dataImage->dc + programData.machineCodeImage->ic + PROGRAM_START_INDEX >= MEMORY_IMAGE_SIZE){ \
		error(programData.pState,MALLOC_FAILURE,"error: ram overflow\n"); \
	} \
	if (programData.pState->state == MALLOC_FAILURE) { exitProgram(programData,1); } \
	if (programData.pState->state == FILE_FAILURE) { return returnVal; } \
}

/* main function of the file. does just as described in the start of the file.
 * takes the preprocessor info and returns the program data */
ProgramData firstpass(pPreprocessorInfo pInfo);

#endif
