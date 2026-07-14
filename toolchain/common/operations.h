#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "../common/constants.h"

/* this sturct is used to store information about all the 16 operations the assembler has.
 * to store which addressing methos are valid for each operation, a number is used.
 * the amount of bits the number has is equal to the number of all existing addressing methods (4 in this case).
 * to show what addressing methods are legal for the operation, the i bit (from left to right)
 * will be 1 if addressing method number i is legal for this operation.
 * the order of the addressing methods is INSTANT,DIRECT,INDIRECT REGISTER,DIRECT REGISTER.
 * the order can also be seen in constants.h */
typedef struct _Operation{
	char *name; /* name of the operation */
	unsigned int sourceMethods: ADDRESSING_METHODS; /* all legal source addressing methods */
	unsigned int targetMethods: ADDRESSING_METHODS; /* all legal target addressing methods */
	int code: 8; /* code of the operation, 4 bits are enough for the valid operations,
	but it is also useful to have an operation with a -1 code (and the struct has padding anyway) */
} Operation;

#ifdef __cplusplus
extern "C" {
#endif

/* gives you the operation table, returns the amount of legal operations */
int getOperationsTable(Operation **operationsTable);

#ifdef __cplusplus
}
#endif

#endif