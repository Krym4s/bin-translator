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

    WriteLib   (&bin, "std_lib_nh");
    WriteToElf (elfFile, &bin); 

    //--free file threads and memory--/
    free    (inputBuffer);
    fclose  (elfFile);

    return BTR_NO_ERRORS;
}

BIN_TRANSLATOR_ERRORS IsE_to_x86_translation (char* input_buffer, char* output_buffer, unsigned int* fileSize)
{
    if(strncmp ("KRYM", input_buffer,4))
        return BTR_WRONG_ECE_FORM;
    
    input_buffer += 4;

    printf ("norm");  

    input_buffer += 4;

    int commandN = *(int*)input_buffer;

    input_buffer += 4;

    FILE* lib = fopen ("std_lib_nh", "r");
    unsigned int libSize = getFileSize (lib);


    int IsErip = 0;
    int Elfrip = 0;
    command* commands = (command*) calloc (sizeof (command), MAX_CODE_SIZE);
    lable* lables = (lable*) calloc (sizeof (lable), MAX_CODE_SIZE);

    if (!commands)
        return BTR_NO_FREE_MEMORY;

    if (!lables)
    {
        free (commands);
        return BTR_NO_FREE_MEMORY;
    }
    FindIsECommands (input_buffer, output_buffer,commands, &commandN, libSize, &IsErip, &Elfrip);
    *fileSize = Elfrip;

    unsigned int lableN = 0;

    printf ("commandN %d", commandN);
      
    FindlableAdresses (input_buffer, commands, lables, commandN, &lableN);
    printf ("labeln %d\n", lableN);

    SetLabels (output_buffer, commands, lables, commandN, lableN);


    //set  lables
    //translate
    
    
    
/*
    unsigned int rip = 0;
   
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
    
*/   

    free (commands);
    free (lables);
    
    return BTR_NO_ERRORS;

}

BIN_TRANSLATOR_ERRORS WriteToElf (FILE* output, BinaryMin* codeStructure)
{
    fwrite (&(codeStructure->firstHeader) , sizeof (Elf_Header_x86_min), 1                                  , output);
    fwrite (&(codeStructure->secondHeader), sizeof (Program_Header)    , 1                                  , output);
    fwrite (  codeStructure->IsE_STD      , sizeof (char)              , codeStructure->libSize             , output);
    fwrite (  codeStructure->code         , sizeof (char)              , codeStructure->secondHeader.P_FILES - 0x80 - codeStructure->libSize , output);
    fflush(stdout); 
    return BTR_NO_ERRORS;
}

BIN_TRANSLATOR_ERRORS WriteLib  (BinaryMin* structure, const char* filename)
{
    assert (filename);

    readToStorage (filename, &(structure->IsE_STD));
    FILE* lib = fopen (filename, "r");
    if (!lib)
    {
        return BTR_NO_INPUT_FILE;
    }

    unsigned int fsize = getFileSize (lib);
    structure->secondHeader.P_FILES += fsize; 
    structure->secondHeader.P_MEMSZ += fsize; 
    structure->libSize = fsize;
    fclose (lib);

printf ("%s", structure->IsE_STD);
    return BTR_NO_ERRORS;

}

BIN_TRANSLATOR_ERRORS FindIsECommands (char* input, char* output, command* commands, int* commandN, int libSize,  int* IsErip, int* Elfrip)
{
    int i = 0;
    for (; *IsErip < *commandN; i ++)
    {
        commands[i].Elfaddress = *Elfrip;
        commands[i].IsEaddress = *IsErip;
        commands[i].commandType = IsEOpHandler (input, output, IsErip, Elfrip, libSize);
    }

    printf ("i index is %d\n", i);

    *commandN = i;
    
    return BTR_NO_ERRORS;

}

IsE_Commands IsEOpHandler (char* input, char* output, int* IsErip, int* Elfrip, int libSize)
{
    int address = 0;
    switch (input[*IsErip])
    {
        case 0x00:
            output[(*Elfrip)++] = HLTb.byteInter[0];
            address = 1 - libSize - *Elfrip;
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, address);
            return HLT;
        case 3:
            output[(*Elfrip)++] = INb.byteInter[0];
            address = 3 - libSize - *Elfrip;
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, address);
            return IN;
        case 0x13:
            output[(*Elfrip)++] = OUTb.byteInter[0];
            address = 5 - libSize - *Elfrip;
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, address);
            return OUT;

        case 21:
            printf ("jmp is here\n");
            output[(*Elfrip)++] = JMPb.byteInter[0];
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, address);
            *IsErip += 4;
            return JMP;

        case 1:
            for (int i = 0; i < ADDb.opSize; i++)
                  output[(*Elfrip)++] = ADDb.byteInter[i];

            (*IsErip) ++;      
            return ADD;  
        case 17:
            for (int i = 0; i < SUBb.opSize; i++)
                  output[(*Elfrip)++] = SUBb.byteInter[i];

            (*IsErip) ++;      
            return SUB;  
        case 2:
            for (int i = 0; i < MULb.opSize; i++)
                  output[(*Elfrip)++] = MULb.byteInter[i];

            (*IsErip) ++;      
            return MUL;  
        case 18:
            for (int i = 0; i < DIVb.opSize; i++)
                  output[(*Elfrip)++] = DIVb.byteInter[i];

            (*IsErip) ++;      
            return DIV;  

        case 4:
            (*IsErip)++;
            return IsEPushHandler (input, output, IsErip, Elfrip);

        case 20:
            (*IsErip)++;
            return IsEPopHandler  (input, output, IsErip, Elfrip);

        case 26:
            (*IsErip)+= 5;
            for (int i = 0; i < CONDjmp.opSize; i++)
                output[(*Elfrip)++] = CONDjmp.byteInter[i];

            output [*Elfrip - 8] = 4;    

            (*Elfrip) += 4;     
            return JE;

            
        default:
            break;
    }
    return HLT;
}

BIN_TRANSLATOR_ERRORS FindlableAdresses (char* input, command* commands, lable* lables, int commandN, unsigned int* lableN)
{
    printf ("command num in label %d\n", commandN);
    for (int i = 0; i < commandN; i++)
    {
        printf ("mmm %d\n", commands[i].commandType);
        fflush (stdout);
        if (commands[i].commandType <= JNE && commands[i].commandType >= CALL)
        {
            int* addr = (int*) (input + commands[i].IsEaddress + 1);

            for (int j = 0; j < commandN; j ++)
                if (commands[j].IsEaddress == *addr)
                {
                    printf ("found");
                    lables[*lableN].commandN  = i;
                    lables[(*lableN)++].lableN  = j;
                    break;
                }
        }
            
    }
    return BTR_NO_ERRORS;
}

BIN_TRANSLATOR_ERRORS SetLabels (char* output, command* commands, lable* lables, int commandN, unsigned int lableN)
{
    int addrByte = 0;
    int jmpLen   = 0;
    for (int i = 0; i < lableN ; i ++)
    {
        printf ("first commad type %d\n", commands[lables[i].commandN].commandType);
        
        switch (commands[lables[i].commandN].commandType)
        {
            case JMP:
                addrByte = 1;
                jmpLen   = 4;
                //SetUClabel (output, commands, lables, lables[i].commandN, lables[i].lableN);
                printf ("label %d seted", i);
            break;

            case JE:
                addrByte = CONDjmp.opSize;
                jmpLen   = 4;
                
            break;
        }
        int* bytes = (int*)(output + commands[lables[i].commandN].Elfaddress + addrByte);
        *bytes = - addrByte - jmpLen - commands[lables[i].commandN].Elfaddress + commands[lables[i].lableN].Elfaddress;
        printf ("deltaaddr %d", - addrByte - jmpLen - commands[lables[i].commandN].Elfaddress + commands[lables[i].lableN].Elfaddress);
    }
    return BTR_NO_ERRORS;
}

void   JmpRelatedOpHandler (char* input, char* output, int* IsErip, int* Elfrip, int address)
{
    (*IsErip) += 1;
    int* addr = (int*)(output + *Elfrip);
    *addr = address;
    (*Elfrip) += 4;
    printf ("elf: %d\n", *Elfrip);  
}

IsE_Commands IsEPushHandler (char* input, char* output, int* IsErip, int* Elfrip)
{
    char delta = 0;
    switch (input[(*IsErip)++])
    {
    case 1:
        for (int i = 0 ; i < 2; i++)
            output[(*Elfrip)++] = PUSHnb.byteInter[i]; 

        for (int i = 0; i < PUSHnb.argSize; i++)    
            output[(*Elfrip)++] = input [(*IsErip)++];

        for (int i = 2 ; i < PUSHnb.opSize; i++)
            output[(*Elfrip)++] = PUSHnb.byteInter[i];

        return PUSHn;

    case 2:
        delta = input[(*IsErip)++];
        printf ("push r reg %d\n", delta);
        for (int i = 0; i < PUSHrb.opSize; i++)
        {
            output[(*Elfrip)++] = PUSHrb. byteInter[i];
            printf ("%x", PUSHrb. byteInter[i]);
        }
            

        output [(*Elfrip) - 2] += (delta * 8);

        return PUSHr;
    
    
    default:
        break;
    }
    return PUSHn;
}

IsE_Commands          IsEPopHandler  (char* input, char* output, int* IsErip, int* Elfrip)
{
    char delta = 0;
    switch (input[(*IsErip)++])
    {
    case 0:
        for (int i = 0; i < POPb.argSize; i++)
            output[(*Elfrip)++] = POPb.byteInter[i];

        return POP;

    case 2:
        delta = input[(*IsErip)++];
        for (int i = 0; i < POPrb.opSize; i++)
            output[(*Elfrip)++] = POPrb. byteInter[i];

        output [(*Elfrip) - 6] += delta * 8;
        printf ("delta %d\n", delta);
        return POPr;
    
    
    default:
        break;
    }
    return POP;
}

void SetUClabel (char* output, command* commands, lable* lables, unsigned int commandN, unsigned int lableN)
{
    printf ("here is fine\n");
    fflush (stdout);
    int* bytes = (int*)(output + commands[commandN].Elfaddress + 1);
    *bytes = - commands[commandN].Elfaddress + commands[lableN].Elfaddress - 5;
    printf ("delta %d\n",- commands[commandN].Elfaddress + commands[lableN].Elfaddress);
}