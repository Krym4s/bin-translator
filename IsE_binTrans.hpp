#include <stdlib.h>
#include <stdio.h>
#include "IsE_LinesF.h"
#include "IsE_Elf.hpp"

enum BIN_TRANSLATOR_ERRORS
{
    BTR_NO_ERRORS       = 0,
    BTR_NO_INPUT_FILE   = 1,
    BTR_NO_OUTPUT_FILE  = 2,
    BTR_NO_FREE_MEMORY  = 3
};

struct BinaryMin
{
    Elf_Header_x86_min  firstHeader  = {};
    Program_Header      secondHeader = {};
    char*               IsE_STD;  
    unsigned int        libSize;
    char*               code;
};


BIN_TRANSLATOR_ERRORS CreateElf (const char* input, const char* output);
BIN_TRANSLATOR_ERRORS IsE_to_x86_translation (char* input_buffer, char* output_buffer,unsigned int* fileSize);
BIN_TRANSLATOR_ERRORS WriteToElf (FILE* output, BinaryMin* codeStructure);
