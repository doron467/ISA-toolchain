/* this program has functions that help managing the machine code image and data image.
 * it is also responsible for building the words from the input. */

#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

#include "utils.h"

typedef struct _Word *pWord;
typedef struct _DataImage *pDataImage;
typedef struct _MachineCodeImage *pMachineCodeImage;

typedef struct _Word{ /* struct for word */
	unsigned int number: WORD_LENGTH;
} Word;

typedef struct _DataImage{ /* struct for the data image */
	Word image[MEMORY_IMAGE_SIZE]; /* array storage for data */
	unsigned long dc; /* data counter */
} DataImage;

typedef struct _MachineCodeImage{ /* struct for the machine code image */
	Word image[MEMORY_IMAGE_SIZE]; /* array storage for machine code */
	unsigned long ic; /* instruction counter */
} MachineCodeImage;

/* returns the given word with the bit in the bitNumber position off */
#define setWordBitOff(word,bitNumber) (word).number & (~(1 << (bitNumber) ))
/* returns the given word with the bit in the bitNumber position on */
#define setWordBitOn(word,bitNumber) (word).number | (1 << (bitNumber) )
/* sets the bit in the word in the bitNumber position to the given value (0/1) */
#define setWordBit(word,bitNumber,value) (word).number = (value) ? setWordBitOn(word,bitNumber) : setWordBitOff(word,bitNumber);
/* returns the bit in the given word in the bitNumber position */
#define getWordBit(word,bitNumber) (( (word).number >> (bitNumber) ) & 1)

/* returns 1 if addressing method number methodBit is enabled in the given methods.
 * used to make sure that every operation was called with the legal addressing methods */
#define isAddressingMethod(methods,methodBit) ( (methods) & (1 << (ADDRESSING_METHODS - 1 - (methodBit) )) )

/* returns 1 if the source addressing method is either direct register or indirect register,otherwise 0 */
#define sourceIsRegister(word) (getWordBit(word,SOURCE_METHOD_START + INDIRECT_REGISTER_ADDRESSING) || getWordBit(word,SOURCE_METHOD_START + DIRECT_REGISTER_ADDRESSING))
/* returns 1 if the target addressing method is either direct register or indirect register,otherwise 0 */
#define targetIsRegister(word) (getWordBit(word,TARGET_METHOD_START + INDIRECT_REGISTER_ADDRESSING) || getWordBit(word,TARGET_METHOD_START + DIRECT_REGISTER_ADDRESSING))
/* returns 1 if the second and third word can be shared from looking at the first word, otherwise returns 0 */
#define sharingWord(word) (targetIsRegister(word) && sourceIsRegister(word))

/* creates a pointer to a data image struct, sets the data counter to 0 and returns the pointer */
pDataImage createDataImage(pProgramState pState);

/* pushes a list of numbers into the data image. data string should point to the first
 * character/number after the data instruction */
int pushData(pDataImage image,char *data,pProgramState pState);

/* pushes a string into the data image, and returns the length of the string.
 * string should point to the first character after the string instruction */
int pushString(pDataImage image,char *string,pProgramState pState);

/* creates a pointer to a machine code struct, sets the instruction counter to 0 and returns the pointer */
pMachineCodeImage createMachineCodeImage(pProgramState pState);

/* builds the first word of the instruction from the given operands.
 * operation is the matching operation struct of the instruction. */
Word buildFirstWord(Operation operation,char *operands,pProgramState pState);

/* builds a word with the given A.R.E field, with the rest of the bits representing the given value, and returns the word.
 * used for building the second and third words. */
Word buildWord(int areField,long value,pProgramState pState);

/* returns how many words are needed for a given instruction, from the operation and first word */
int getWordSize(Operation operation,Word word);

#endif
