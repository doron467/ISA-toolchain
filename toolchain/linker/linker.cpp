#include "linker.hpp"

// parses the command line arguments and returns the number of input files
// also gets the output file name
int parseArgs(char **argv,char *&outputFileName){
    static char defaultFileName[] = "a.out";
    int fileCount = 0;

    while (*argv != nullptr){
        char *str = *argv;
        if (*str == '-'){
            // flags
            str++;
            if (*str == 'o'){
                // output file name
                argv++;
                if (*argv == nullptr){
                    printf("error: no output file name was given\n");
                    exit(1);
                }

                outputFileName = *argv;
                
            }
            continue;
        }

        fileCount++;
        argv++;
    }

    if (outputFileName == nullptr){
        outputFileName = defaultFileName;
    }

    return fileCount;
}

pFileData *getFileDatas(char **argv,int fileCount){
    int currentFileIndex = 0;
    pFileData *fileDatas = (pFileData *)malloc(fileCount * sizeof(pFileData));
    if (fileDatas == nullptr){
        printf("error: pFileData malloc failed\n");
        exit(1);
    }

    while (*argv != nullptr){
        char *str = *argv;
        if (*str == '-'){
            // skip flags
            str++;
            if (*str == 'o'){
                argv += 2; // skip output file name
                continue;
            }
            argv++;
            continue;
        }

        pFileData pCurrentData = (pFileData)malloc(sizeof(fileData));
        if (pCurrentData == nullptr){
            printf("error: malloc failed\n");
            exit(1);
        }

        fileDatas[currentFileIndex++] = pCurrentData;

        std::string obFileName = std::string(str) + ".ob";

        FILE *file = fopen(obFileName.c_str(),"r");
        if (file == nullptr){
            printf("error: could not open file %s\n",str);
            exit(1);
        }

        int instructionLength;
        int dataLength;
        fscanf(file,"%d %d",&instructionLength,&dataLength);
        pCurrentData->instructionLength = instructionLength;
        pCurrentData->dataLength = dataLength;
        pCurrentData->fileName = str;

        fclose(file);

        argv++;
    }

    return fileDatas;
}

pOutputFileData getOutputFileData(pFileData *fileDatas,int fileCount){
    pOutputFileData pOutputData = (pOutputFileData)malloc(sizeof(outputFileData));
    if (pOutputData == nullptr){
        printf("error: file output data malloc failed\n");
        exit(1);
    }

    pOutputData->totalInstructionLength = 0;
    pOutputData->totalDataLength = 0;
    pOutputData->instructionLengthDP = new int[fileCount];
    pOutputData->dataLengthDP = new int[fileCount];
    for (int i = 0; i < fileCount; i++){
        pOutputData->totalInstructionLength += fileDatas[i]->instructionLength;
        pOutputData->totalDataLength += fileDatas[i]->dataLength;

        if (i == 0){
            pOutputData->instructionLengthDP[0] = 0;
            pOutputData->dataLengthDP[0] = 0;
        } else {
            pOutputData->instructionLengthDP[i] = pOutputData->instructionLengthDP[i - 1] + fileDatas[i - 1]->instructionLength;
            pOutputData->dataLengthDP[i] = pOutputData->dataLengthDP[i - 1] + fileDatas[i - 1]->dataLength;
        }

    }

    return pOutputData;
}

std::unordered_map<std::string,int> getLabelEntries(pOutputFileData pOutputData,pFileData *fileDatas,int fileCount){
    std::unordered_map<std::string,int> labelAddressMap;

    for (int i = 0; i < fileCount; i++){
        pFileData pCurrentData = fileDatas[i];
        std::string entFileName = std::string(pCurrentData->fileName) + ".ent";
        FILE *entFile = fopen(entFileName.c_str(),"r");
        if (entFile != nullptr){
            char labelName[LABEL_MAX_SIZE + 1];
            int localAddress;
            while (fscanf(entFile,"%s %d",labelName,&localAddress) != EOF){
                int globalAddress;
                if (localAddress < pCurrentData->instructionLength){
                    globalAddress = localAddress + pOutputData->instructionLengthDP[i];
                } else {
                    globalAddress = localAddress - pCurrentData->instructionLength + pOutputData->totalInstructionLength + pOutputData->dataLengthDP[i];
                }
                labelAddressMap[labelName] = globalAddress;
            }

            fclose(entFile);
        }
 
    }

    return labelAddressMap;
}

int writeInstructionsToOutput(linkerData linkerData,int currentAddress){
    for (int i = 0; i < linkerData.fileCount; i++){
        pFileData pCurrentData = linkerData.fileDatas[i];
        std::string obFileName = std::string(pCurrentData->fileName) + ".ob";
        std::string extFileName = std::string(pCurrentData->fileName) + ".ext";
        FILE *obFile = fopen(obFileName.c_str(),"r");
        FILE *extFile = fopen(extFileName.c_str(),"r");
        if (obFile == nullptr){
            std::cout << "error: could not open file " << obFileName << "\n";
            return 1;
        }

        int localAddress;
        int instructionCode;
        while (fgetc(obFile) != '\n'); // skip first line
        while (fscanf(obFile,"%d %x",&localAddress,&instructionCode) != EOF){
            
            if (localAddress >= pCurrentData->instructionLength){
                break; // reached data section, stop reading instructions
            }

            if (instructionCode & (1 << ABSOLUTE_ADDRESSING)){
                fprintf(linkerData.outputFile,"%04d  %04x\n",currentAddress,instructionCode);
            } else if (instructionCode & (1 << RELATIVE_ADDRESSING)){
                int labelLocalAddress = instructionCode >> (ARE_END + 1);
                int labelGlobalAddress;
                
                if (labelLocalAddress < pCurrentData->instructionLength){
                    labelGlobalAddress = labelLocalAddress + linkerData.pOutputData->instructionLengthDP[i];
                } else {
                    int totalInstructionLength = linkerData.pOutputData->totalInstructionLength;
                    int *dataLengthDP = linkerData.pOutputData->dataLengthDP;
                    labelGlobalAddress = labelLocalAddress - pCurrentData->instructionLength + totalInstructionLength + dataLengthDP[i];  
                }
                
                int newInstructionCode = (labelGlobalAddress << (ARE_END + 1)) | (1 << RELATIVE_ADDRESSING);

                
                fprintf(linkerData.outputFile,"%04d  %04x\n",currentAddress,newInstructionCode);
            } else if (instructionCode & (1 << EXTERNAL_ADDRESSING)){
                
                if (extFile == nullptr){
                    std::cout << "error: could not open file " << extFileName << "\n";
                    return 1;
                }

                char labelName[LABEL_MAX_SIZE + 1];
                int labelAddress;
                fscanf(extFile,"%s %d",labelName,&labelAddress);
                labelAddress = linkerData.labelAddressMap[labelName];
                int newInstructionCode = (labelAddress << (ARE_END + 1)) | (1 << RELATIVE_ADDRESSING);
                fprintf(linkerData.outputFile,"%04d  %04x\n",currentAddress,newInstructionCode);

            }

            currentAddress++;

        }

        fclose(obFile);
        if (extFile != nullptr){
            fclose(extFile);
        }
        
    }

    return currentAddress;
}

int writeDataToOutput(linkerData linkerData,int currentAddress){
    for (int i = 0; i < linkerData.fileCount; i++){
        pFileData pCurrentData = linkerData.fileDatas[i];
        std::string obFileName = std::string(pCurrentData->fileName) + ".ob";
        FILE *obFile = fopen(obFileName.c_str(),"r");
        if (obFile == nullptr){
            std::cout << "error: could not open file " << obFileName << "\n";
            return 1;
        }
        int localAddress;
        int dataCode;
        while (fgetc(obFile) != '\n'); // skip first line
        while (fscanf(obFile,"%d %x",&localAddress,&dataCode) != EOF){
            
            if (localAddress < pCurrentData->instructionLength){
                continue; // skip instructions, we only want data
            }

            fprintf(linkerData.outputFile,"%04d  %04x\n",currentAddress,dataCode);
            currentAddress++;
        }
    }

    return currentAddress;
}

int getProgramEntryAddress(std::unordered_map<std::string,int> labelAddressMap){
    auto it = labelAddressMap.find("MAIN");
    if (it == labelAddressMap.end()){
        std::cout << "error: no entry point found (label MAIN not found)\n";
        exit(1);
    }

    return it->second;
}

void freeLinkerData(linkerData linkerData){
    for (int i = 0; i < linkerData.fileCount; i++){
        free(linkerData.fileDatas[i]);
    }
    free(linkerData.fileDatas);
    delete[] linkerData.pOutputData->instructionLengthDP;
    delete[] linkerData.pOutputData->dataLengthDP;
    free(linkerData.pOutputData);

}

int main(int argc,char **argv){

    int fileCount = 0;
    char *outputFileName = nullptr;
    linkerData linkerData;

    if (argc < 2){
        printf("error: no input files were given\n");
        exit(1);
    }

    fileCount = parseArgs(argv + 1,outputFileName);
    linkerData.fileCount = fileCount;

    if (fileCount == 0){
        printf("error: no input files were given\n");
        exit(1);
    }
    
    linkerData.fileDatas = getFileDatas(argv + 1,fileCount);


    linkerData.pOutputData = getOutputFileData(linkerData.fileDatas,fileCount);
    linkerData.labelAddressMap = getLabelEntries(linkerData.pOutputData,linkerData.fileDatas,fileCount);
    linkerData.programEntryAddress = getProgramEntryAddress(linkerData.labelAddressMap);

    linkerData.outputFile = fopen(outputFileName,"w");
    if (linkerData.outputFile == nullptr){
        printf("error: could not open output file %s\n",outputFileName);
        return 1;
    }
    fprintf(linkerData.outputFile,"%04d\n",linkerData.programEntryAddress);
    fprintf(linkerData.outputFile,"%d %d\n",linkerData.pOutputData->totalInstructionLength,linkerData.pOutputData->totalDataLength);

    int currentAddress = writeInstructionsToOutput(linkerData,0);
    currentAddress = writeDataToOutput(linkerData,currentAddress);

    freeLinkerData(linkerData);
    fclose(linkerData.outputFile);

    return 0;
}