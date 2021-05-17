#include <stdlib.h>
#include <stdio.h>
#include "IsE_LinesF.h"
#include "IsE_Elf.hpp"

enum BIN_TRANSLATOR_ERRORS
{
    BTR_NO_ERRORS       = 0,
    BTR_NO_INPUT_FILE   = 1,
    BTR_NO_OUTPUT_FILE  = 2,
    BTR_NO_FREE_MEMORY  = 3,
    BTR_WRONG_ECE_FORM  = 4,
};

struct BinaryMin
{
    Elf_Header_x86_min  firstHeader  = {};
    Program_Header      secondHeader = {};
    char*               IsE_STD;  
    unsigned int        libSize;
    char*               code;
};

enum IsE_Commands
{
    HLT     =   0,
    ADD     =   1,
    SUB     =   2,
    MUL     =   3,
    DIV     =   4,
    IN      =   5,
    OUT     =   6,

    PUSHn   =   7, //push number
    PUSHnr  =   8, //push register 
    PUSHr   =   9, //push from register
    PUSHmn  =   10,//push from memory by number
    PUSHmr  =   11,//push from memory by register
    PUSHmnr =   12,//push from memory by sum of register and number

    POP     =   13,//ordinar pop from stack
    POPr    =   14,//pop to register
    POPmn   =   15,//pop to memory by number
    POPmr   =   16,//pop to memory by register
    POPmnr  =   17,//pop to memory by sum of register and number    

    RET     =   18,
    CALL    =   19,

    JMP     =   20,
    JA      =   21,
    JB      =   22,
    JAE     =   23,
    JBE     =   24,
    JE      =   25,
    JNE     =   26
};



struct command
{
    IsE_Commands    commandType;
    int             IsEaddress;
    int             Elfaddress;
};

struct lable
{
    unsigned int    commandN;
    unsigned int    lableN;  
};

struct byteInterpret
{
    unsigned char   byteInter[50];
    unsigned int    opSize;
    unsigned int    argSize;
};

BIN_TRANSLATOR_ERRORS FindIsECommands (char* input, char* output, command* commands, int* commandN, int libSize, int* IsErip, int* Elfrip);
BIN_TRANSLATOR_ERRORS FindlableAdresses (char* input, command* commands, lable* lables, int commandN, unsigned int* lableN);
BIN_TRANSLATOR_ERRORS SetLabels (char* output, command* commands, lable* lables, int commandN, unsigned int lableN);

BIN_TRANSLATOR_ERRORS CreateElf (const char* input, const char* output);
BIN_TRANSLATOR_ERRORS IsE_to_x86_translation (char* input_buffer, char* output_buffer,unsigned int* fileSize);
BIN_TRANSLATOR_ERRORS WriteLib  (BinaryMin* structure, const char* filename);
BIN_TRANSLATOR_ERRORS WriteToElf (FILE* output, BinaryMin* codeStructure);

IsE_Commands          IsEOpHandler (char* input, char* output, int* IsErip, int* Elfrip, int libSize); 
void                  JmpRelatedOpHandler (char* input, char* output, int* IsErip, int* Elfrip, int address);
IsE_Commands          IsEPushHandler (char* input, char* output, int* IsErip, int* Elfrip);
IsE_Commands          IsEPopHandler  (char* input, char* output, int* IsErip, int* Elfrip);
IsE_Commands          IsECondJmpHandler (char* input, char* output, int* IsErip, int* Elfrip, char mode, IsE_Commands command);

const struct byteInterpret INb      = {{0xe8}, 1, 4};
const struct byteInterpret OUTb     = {{0xe8}, 1, 4};
const struct byteInterpret CALLb    = {{0xe8}, 1, 4};
const struct byteInterpret HLTb     = {{0xe8}, 1, 4};
const struct byteInterpret JMPb     = {{0xe9}, 1, 4};
const struct byteInterpret RETb     = {{0xc3}, 1, 0};

const struct byteInterpret ADDb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f, 0x58, 0x04, 0x24, 0xf2, 0x0f, 0x11, 0x04, 0x24}, 19, 0};
const struct byteInterpret SUBb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f, 0x5c, 0x04, 0x24, 0xf2, 0x0f, 0x11, 0x04, 0x24}, 19, 0};
const struct byteInterpret MULb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f, 0x59, 0x04, 0x24, 0xf2, 0x0f, 0x11, 0x04, 0x24}, 19, 0};
const struct byteInterpret DIVb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f, 0x5e, 0x04, 0x24, 0xf2, 0x0f, 0x11, 0x04, 0x24}, 19, 0};

const struct byteInterpret PUSHnb   = {{0x48, 0xb8, 0x48, 0x83, 0xec, 0x08, 0x48,0x89, 0x04, 0x24}, 10, 8};
const struct byteInterpret PUSHrb   = {{0x48, 0x83, 0xec, 0x08, 0xf2, 0x0f,0x11, 0x14, 0x24},9 ,1};

const struct byteInterpret POPb     = {{0x48, 0x83, 0xc4, 0x08},4 ,0};
const struct byteInterpret POPrb    = {{0xf2, 0x0f, 0x10 , 0x14/**/, 0x24, 0x48, 0x83, 0xc4, 0x08},9 ,1};


const struct byteInterpret CONDjmp  = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f ,0x10, 0x0c, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 
                                        0x0f, 0xc2 ,0xc1, 0x00/*changeble*/ , 0x66, 0x0f, 0x38, 0x17, 0xc0, 0x0f, 0x84}, 30, 1};



void SetUClabel (char* output, command* commands, lable* lables, unsigned int commandN, unsigned int lableN);
void SetClabel (char* output, command* commands, lable* lables, unsigned int commandN, unsigned int lableN);


