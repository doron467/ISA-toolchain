#ifndef LINKER_H
#define LINKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../common/constants.h"

typedef struct _fileData *pFileData;
typedef struct _outputFileData *pOutputFileData;
typedef struct _entryLabel *pEntryLabel;

typedef struct _linkerData {
    pFileData *fileDatas;
    pOutputFileData pOutputData;
    std::unordered_map<std::string,int> labelAddressMap;
    int fileCount;
    FILE *outputFile;
    int programEntryAddress;
} linkerData;

typedef struct _fileData {
    int instructionLength;
    int dataLength;
    char *fileName;
} fileData;

typedef struct _outputFileData {
    int totalInstructionLength;
    int totalDataLength;
    int *instructionLengthDP;
    int *dataLengthDP;
} outputFileData;

typedef struct _entryLabel {
    short fileIndex;
    int localAddress;
} entryLabel;

#endif