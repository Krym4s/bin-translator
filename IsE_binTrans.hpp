#include <stdlib.h>
#include <stdio.h>
#include "IsE_LinesF.h"
#include "IsE_Elf.hpp"

const unsigned int version = 1;

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

enum IsE_codes
{
    IsE_HLT     =   0,
    IsE_ADD     =   1,
    IsE_SUB     =   17,
    IsE_MUL     =   2,
    IsE_DIV     =   18,
    IsE_IN      =   3,
    IsE_OUT     =   19,

    IsE_PUSH    =   4, //push number
    
    IsE_POP     =   20,//ordinar pop from stack
    
    IsE_RET     =   29,
    IsE_CALL    =   28,

    IsE_JMP     =   21,
    IsE_JA      =   22,
    IsE_JB      =   24,
    IsE_JAE     =   23,
    IsE_JBE     =   25,
    IsE_JE      =   26,
    IsE_JNE     =   27
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

struct IsE_Header
{
    unsigned int signature;
    unsigned int version;
    unsigned int nMembers;
};

BIN_TRANSLATOR_ERRORS FindIsECommands (char* input, char* output, command* commands, int* commandN, int libSize, int* IsErip, int* Elfrip);
BIN_TRANSLATOR_ERRORS FindlableAdresses (char* input, command* commands, lable* lables, int commandN, unsigned int* lableN);
BIN_TRANSLATOR_ERRORS SetLabels (char* output, command* commands, lable* lables, int commandN, unsigned int lableN);

BIN_TRANSLATOR_ERRORS CreateElf (const char* input, const char* output);
BIN_TRANSLATOR_ERRORS IsE_to_x86_translation (char* input_buffer, char* output_buffer,unsigned int* fileSize, unsigned int libSize);
BIN_TRANSLATOR_ERRORS WriteLib  (BinaryMin* structure, const char* filename);
BIN_TRANSLATOR_ERRORS WriteToElf (FILE* output, BinaryMin* codeStructure);

IsE_Commands          IsEOpHandler (char* input, char* output, int* IsErip, int* Elfrip, int libSize); 
void                  JmpRelatedOpHandler (char* input, char* output, int* IsErip, int* Elfrip, int address);
IsE_Commands          IsEPushHandler (char* input, char* output, int* IsErip, int* Elfrip);
IsE_Commands          IsEPopHandler  (char* input, char* output, int* IsErip, int* Elfrip);
IsE_Commands          IsECondJmpHandler (char* input, char* output, int* IsErip, int* Elfrip, char mode, IsE_Commands command);


const unsigned int addressLength    = 4;
const unsigned int regNDelta        = 8;
const unsigned int elfHeadresSize   = 0x80;

const unsigned int HLToffset        = 1;
const unsigned int INoffset         = 3;
const unsigned int OUToffset        = 5;

const unsigned int signature        = 0x4d59524b;


const struct byteInterpret INb      = {{0xe8}, 1, 4};   // call ...
const struct byteInterpret OUTb     = {{0xe8}, 1, 4};   // call ...
const struct byteInterpret CALLb    = {{0xe8}, 1, 4};   // call ...
const struct byteInterpret HLTb     = {{0xe8}, 1, 4};   // call ...
const struct byteInterpret JMPb     = {{0xe9}, 1, 4};   // jmp  ... (near)
const struct byteInterpret RETb     = {{0xc3}, 1, 0};   // ret  ...

const unsigned char ADDop = 0x58;
const unsigned char MULop = 0x59;
const unsigned char SUBop = 0x5c;
const unsigned char DIVop = 0x5e;

const unsigned int  MathModeByteOffset = 11;

const struct byteInterpret Mathb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, // movsd  xmm0, [rsp]  
                                        0x48, 0x83, 0xc4, 0x08,       // add    rsp,    8
                                        0xf2, 0x0f, 0x58, 0x04, 0x24, // addsd  xmm0, [rsp]
                                        0xf2, 0x0f, 0x11, 0x04, 0x24},// movsd  [rsp], xmm0
                                        19, 0};
const struct byteInterpret SUBb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f, 0x5c, 0x04, 0x24, 0xf2, 0x0f, 0x11, 0x04, 0x24}, 19, 0};
const struct byteInterpret MULb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f, 0x59, 0x04, 0x24, 0xf2, 0x0f, 0x11, 0x04, 0x24}, 19, 0};
const struct byteInterpret DIVb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f, 0x5e, 0x04, 0x24, 0xf2, 0x0f, 0x11, 0x04, 0x24}, 19, 0};

const struct byteInterpret ADDb     = {{0xf2, 0x0f, 0x10, 0x04, 0x24, 0x48, 0x83, 0xc4, 0x08, 0xf2, 0x0f, 0x5e, 0x04, 0x24, 0xf2, 0x0f, 0x11, 0x04, 0x24}, 19, 0};


IsE_Commands          IsEMathHandler (char* input, char* output, int* IsErip, int* Elfrip, IsE_Commands commandId);


const struct byteInterpret PUSHnb   = {{0x48, 0xb8,              // movabs  rax /-next 8 bytes are double number-/ 
                                        0x48, 0x83, 0xec, 0x08,  // sub     rsp,  8
                                        0x48, 0x89, 0x04, 0x24}, // mov    [rsp], rax
                                        10, 8};


const unsigned int PUSHNumberOffset = 2;


const struct byteInterpret PUSHrb   = {{0x48, 0x83, 0xec, 0x08,       // sub    rsp, 8
                                        0xf2, 0x0f, 0x11, 0x14, 0x24},// movsd  [rsp], xmm...    // xmm2 - 0
                                                         /*~~~*/                                 // xmm3 - 8   
                                        /*byte responsible for number 0f xmm register to push*/  // xmm4 - 16
                                        9 ,1};                                                   // xmm5 - 24

const struct byteInterpret POPb     = {{0x48, 0x83, 0xc4, 0x08},     //  add    rsp, 8
                                        4 ,0};
const struct byteInterpret POPrb    = {{0xf2, 0x0f, 0x10, 0x14, 0x24, // movsd  xmm... , [rsp]  // xmm2 - 0
                                                        /*~~~*/                                 // xmm3 - 8   
                                    /*byte responsible for number 0f xmm register to push*/     // xmm4 - 16
                                        0x48, 0x83, 0xc4, 0x08},      // add    rsp, 8          // xmm5 - 24
                                        9 ,1};                                                  
                                                                                                

const unsigned int CondJmpModeBOffset = 23;
const unsigned int CondAddressOffset  = 8;
const unsigned int PopRegOffset       = 6;
const unsigned int PushRegOffset      = 2;

const struct byteInterpret CONDjmp  = {{0xf2, 0x0f, 0x10, 0x04, 0x24, // movsd  xmm0, [rsp]             //  jne     - 0
                                        0x48, 0x83, 0xc4, 0x08,       // add    rsp, 8                  //  jae     - 1
                                        0xf2, 0x0f ,0x10, 0x0c, 0x24, // movsd  xmm1, [rsp]             //  ja      - 2
                                        0x48, 0x83, 0xc4, 0x08,       // add    rsp, 8                  //  jnord   - 3 : is not used
                                        0xf2, 0x0f, 0xc2 ,0xc1, 0x00, // cmpsd  xmm0, xmm1, ...         //  je      - 4
                                                              /*~~~*/                                   //  jb      - 5
                                                    /* byte of mod of cmpsd */                          //  jbe     - 6
                                        0x66, 0x0f, 0x38, 0x17, 0xc0, // test   xmm0, xmm0              //  jor     - 7 : is not used
                                        0x0f, 0x84},                  // je     ...                     //
                                        30, 4}; 


BIN_TRANSLATOR_ERRORS      PUSHPOPOPTIMIZATION (char* output, command* commands, unsigned int* bufferSize, unsigned int commandN);

struct byteSequence
{
    unsigned char bytes[20];
    unsigned int nBytes;
};

const struct byteSequence  PUSHPOPs = {{0x48, 0x83, 0xec, 0x08,       // sub      rsp, 8
                                        0xf2, 0x0f, 0x11, 0x04, 0x24, // movsd    [rsp], xmm0
                                                        /*~~~*/       
                                        /* byte responsible for byte of register*/
                                        0xf2, 0x0f, 0x10, 0x04, 0x24, // movsd    xmm0, [rsp]
                                                        /*~~~*/       
                                        /* byte responsible for byte of register*/
                                        0x48, 0x83, 0xc4, 0x08},      // add      rsp, 8
                                        18};

const struct byteSequence  movOpt = {{0xf2,0x0f,0x10,0xc1},4}; // movsd xmm0, xmm1



///-------------------------------------------------///
/// there are two cases of PUSH - POP optimization  ///
/// 1) value poped and pushed in same register      ///
/// 2) value poped to template register xmm0        ///
///-------------------------------------------------///

void SetUClabel (char* output, command* commands, lable* lables, unsigned int commandN, unsigned int lableN);
void SetClabel (char* output, command* commands, lable* lables, unsigned int commandN, unsigned int lableN);

int  FindPUSHPOPsequence (const byteSequence* first, char* buffer);
const unsigned int firstRegByteOffset  = 7;
const unsigned int secondRegByteOffset = 4; 
const unsigned int thirdRegByteOffset  = 5;



const unsigned char XMM2 = 0x1c;
const unsigned char XMM3 = 0x24;
const unsigned char XMM4 = 0x2c;
const unsigned char XMM5 = 0x3c;

int  checkTranslatable (IsE_Header* header);


