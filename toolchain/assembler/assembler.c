/* main file of the assembler.
 * loops over all the files and calls the preprocessor, first pass, and second pass
 */

#include "firstpass.h"
#include "preprocessor.h"
#include "secondpass.h"
#include "utils.h"

/* the main function of the program, does just as described in the start of the file */
int main(int argc,char **argv){

	char *fileName; /* name of the current file being read (without the extension) */
	pPreprocessorInfo pInfo; /* pointer to the preprocessor info */
	ProgramData programData; /* pointer to the program data */

	if (argc < 2){
		printf("error: assembler called with no files\n");
		exit(1);
	}

	while (--argc){
		printf("\n\n\n"); /* add some spacing between the files */

		fileName = *++argv;
		pInfo = preprocess(fileName);

		if (pInfo != NULL && pInfo->amFile != NULL){
			printf("preprocessor successful\n");

			programData = firstpass(pInfo);
			if(programData.pState->state == VALID){
				printf("first pass successful\n");

				programData.fileName = fileName; /* add the file name to the program data from the first pass */

				programData = secondpass(programData);
				if (programData.pState->state == VALID){
					printf("second pass successful\n");
				} else {
					printf("second pass failed\n");
				}

			} else {
				printf("first pass failed\n");
			}

			freeProgramData(programData);

		} else {
			printf("preprocessor failed\n");
		}

	}

	return 0;
}
