#include "IsE_Elf.hpp"
#include "IsE_binTrans.hpp"

unsigned int MAX_CODE_SIZE = 100000;

BIN_TRANSLATOR_ERRORS CreateElf (const char* input, const char* output)
{
    //--prepairngs for translation--//
    FILE* elfFile = fopen (output, "w");
    if (!elfFile)
        return BTR_NO_OUTPUT_FILE;
        

    char*               inputBuffer     ;
    unsigned int        fileSz          = 0 ;
    char*               outputBuffer    = (char*) calloc (MAX_CODE_SIZE, sizeof (unsigned char));
     
    if (!outputBuffer)
    {
        fclose (elfFile);
        return BTR_NO_FREE_MEMORY;
    }

    readToStorage (input, &inputBuffer); 

    //--translations--//
    IsE_to_x86_translation (inputBuffer, outputBuffer, &fileSz);
 
    //--output to elf file--//
    BinaryMin bin = {};
    bin.libSize = 0;
    bin.code = outputBuffer;

    bin.secondHeader.P_FILES = bin.secondHeader.P_MEMSZ = 0x80 + fileSz;
   
    WriteToElf (elfFile, &bin); 

    //--free file threads and memory--/
    free    (inputBuffer);
    fclose  (elfFile);

    return BTR_NO_ERRORS;
}

BIN_TRANSLATOR_ERRORS IsE_to_x86_translation (char* input_buffer, char* output_buffer, unsigned int* fileSize)
{
    *fileSize = 10;
    *output_buffer = 0xB8;
    *(output_buffer+1) = 0x3C;
    *(output_buffer+2) = 0x00;
    *(output_buffer+3) = 0x00;
    *(output_buffer+4) = 0x00;
    *(output_buffer+5) = 0x48;
    *(output_buffer+6) = 0x31;
    *(output_buffer+7) = 0xff;
    *(output_buffer+8) = 0x0f;
    *(output_buffer+9) = 0x05;

    
    
    return BTR_NO_ERRORS;

}

BIN_TRANSLATOR_ERRORS WriteToElf (FILE* output, BinaryMin* codeStructure)
{
    fwrite (&(codeStructure->firstHeader) , sizeof (Elf_Header_x86_min), 1                                  , output);
    fwrite (&(codeStructure->secondHeader), sizeof (Program_Header)    , 1                                  , output);
    //fwrite (  codeStructure->IsE_STD      , sizeof (char)              , codeStructure->libSize             , output);
    printf ("%p\n", codeStructure->code);
    fwrite (  codeStructure->code         , sizeof (char)              , codeStructure->secondHeader.P_FILES - 0x80, output);
    printf ("aa"); 
    fflush(stdout); 
    return BTR_NO_ERRORS;
}
