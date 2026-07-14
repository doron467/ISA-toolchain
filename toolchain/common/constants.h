/* this file has defines for constants that are used in the program. */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WORD_LENGTH 16 /* number of bits in every word */
#define MEMORY_IMAGE_SIZE 4096 /* ram size */
#define PROGRAM_START_INDEX 0 /* the index the program starts loading in the ram */
#define ADDRESSING_METHODS 4 /* the amount of all possible addressing methods */

#define LINE_MAX_SIZE 80 /* max length of the line (comments don't have a length limit) */
#define LABEL_MAX_SIZE 31 /* max length of a label (macro names also count) */
#define REGISTERS_AMOUNT 8 /* total amount of registers the machine has */

#define FULL_LINE_MAX_SIZE LINE_MAX_SIZE + 2 /* need space for \n and \0 */
#define FULL_LABEL_MAX_SIZE LABEL_MAX_SIZE + 1 /* need space for \0 */

#define SOURCE_METHOD_START 7 /* location of the first bit of the source addressing method field inside the word */
#define TARGET_METHOD_START 3 /* location of the first bit of the target addressing method field inside the word */

#define REGISTER_BITS 3 /*how many bits for the register field in the follow up instructions*/

#define ARE_START 0 /* location of the first bit of the A.R.E field inside the word */
#define ARE_END 2 /* location of the last bit of the A.R.E field inside the word */

#define ABSOLUTE_ADDRESSING 2 /* the bit that activates for absolute addressing method */
#define RELATIVE_ADDRESSING 1 /* the bit that activates for relative addressing method */
#define EXTERNAL_ADDRESSING 0 /* the bit that activates for external addressing method */

#define OP_CODE_START 11 /* first bit of the code field inside the word */
#define OP_CODE_END WORD_LENGTH /* last bit of the code field inside the word */

/* location of the last bit of the source addressing method field inside the word */
#define SOURCE_METHOD_END ((SOURCE_METHOD_START) + (ADDRESSING_METHODS) - 1)
/* location of the last bit of the target addressing method field inside the word */
#define TARGET_METHOD_END ((TARGET_METHOD_START) + (ADDRESSING_METHODS) - 1)

#define INSTANT_ADDRESSING 0 /* the number of instant addressing method */
#define DIRECT_ADDRESSING 1 /* the number of direct addressing method */
#define INDIRECT_REGISTER_ADDRESSING 2 /* the number of indirect register addressing method */
#define DIRECT_REGISTER_ADDRESSING 3 /* the number of direct register addressing method */

/* amount of bits a number has in the addressing methods (second and third words) */
#define NUMBER_LENGTH (WORD_LENGTH - ARE_END - 1)

#define NUMBER_MAX_SIZE ((1 << NUMBER_LENGTH) - 1) /* biggest number possible to store in instructions (2 ^ NUMBER_LENGTH - 1) */
#define NUMBER_MIN_SIZE (-(1 << NUMBER_LENGTH)) /* smallest number possible to store in instructions (- 2 ^ NUMBER_LENGTH) */

#define DATA_NUMBER_MAX_SIZE ((1 << WORD_LENGTH) - 1) /* biggest number possible to store in data image (2 ^ WORD_LENGTH - 1) */
#define DATA_NUMBER_MIN_SIZE (-(1 << WORD_LENGTH)) /* smallest number possible to store in data image (- 2 ^ WORD_LENGTH) */

#endif
