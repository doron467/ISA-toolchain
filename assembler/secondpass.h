/* third stage of the program.
 * assumes most of the syntax is valid, since the syntax checks are done in the first pass.
 * builds the second and third words for all the instructions, and does some checks
 * that are impossible to do in the first pass.
 * also writes all of the entries into the entry file.
 */

#ifndef SECONDPASS_H
#define SECONDPASS_H

#include "firstpass.h"
#include "utils.h"

enum {SOURCE_OPERAND,TARGET_OPERAND};

/* main function of the file. does just as described in the start of the file.
 * takes the program data from the first pass and returns it by the end. */
ProgramData secondpass(ProgramData proramData);

#endif
