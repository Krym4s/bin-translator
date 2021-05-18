#include "IsE_Elf.hpp"
#include "IsE_binTrans.hpp"

unsigned int MAX_CODE_SIZE = 100000;

BIN_TRANSLATOR_ERRORS CreateElf (const char* input, const char* output)
{
    //--prepairngs for translation--//

    FILE* elfFile = fopen (output, "w");
    if (!elfFile)
        return BTR_NO_OUTPUT_FILE;
        

    char*               inputBuffer     = nullptr;
    unsigned int        fileSz          = 0;
    char*               outputBuffer    = (char*) calloc (MAX_CODE_SIZE, sizeof (unsigned char));
     
    if (!outputBuffer)
    {
        fclose (elfFile);
        return BTR_NO_FREE_MEMORY;
    }

    readToStorage (input, &inputBuffer); 

    //--translations--//

    BinaryMin bin = {};
    bin.libSize = 0;
    bin.code = outputBuffer;

    WriteLib   (&bin, "std_lib_nh");
    IsE_to_x86_translation (inputBuffer, outputBuffer, &fileSz, bin.libSize);
 
    //--output to elf file--//

    bin.secondHeader.P_FILES += (elfHeadresSize + fileSz);
    bin.secondHeader.P_MEMSZ += (elfHeadresSize + fileSz);

    
    WriteToElf (elfFile, &bin); 

    //--free file threads and memory--/
    free    (inputBuffer);
    fclose  (elfFile);

    return BTR_NO_ERRORS;
}

BIN_TRANSLATOR_ERRORS IsE_to_x86_translation (char* input_buffer, char* output_buffer, unsigned int* fileSize, unsigned int libSize)
{
    IsE_Header* header = (IsE_Header*)input_buffer;
    if (checkTranslatable (header))
        return BTR_WRONG_ECE_FORM;

    int commandN = header->nMembers;
    input_buffer += sizeof (IsE_Header);

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
    FindlableAdresses (input_buffer, commands, lables, commandN, &lableN);
    PUSHPOPOPTIMIZATION (output_buffer, commands, fileSize, commandN);

    for (int i = 0; i < commandN; i ++)
        printf ("%d: %x\n", i, commands[i].Elfaddress);
    SetLabels (output_buffer, commands, lables, commandN, lableN);

    free (commands);
    free (lables);
    
    return BTR_NO_ERRORS;

}

BIN_TRANSLATOR_ERRORS WriteToElf (FILE* output, BinaryMin* codeStructure)
{
    printf ("%d %d %d", codeStructure->libSize, codeStructure->secondHeader.P_FILES, codeStructure->secondHeader.P_FILES - elfHeadresSize - codeStructure->libSize);
    fwrite (&(codeStructure->firstHeader) , sizeof (Elf_Header_x86_min), 1                                  , output);
    fwrite (&(codeStructure->secondHeader), sizeof (Program_Header)    , 1                                  , output);
    fwrite (  codeStructure->IsE_STD      , sizeof (char)              , codeStructure->libSize             , output);
    fwrite (  codeStructure->code         , sizeof (char)              , codeStructure->secondHeader.P_FILES - elfHeadresSize - codeStructure->libSize , output);
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
    structure->secondHeader.P_FILES = fsize; 
    structure->secondHeader.P_MEMSZ = fsize; 
    structure->libSize = fsize;
    fclose (lib);

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

    *commandN = i;
    
    return BTR_NO_ERRORS;

}

IsE_Commands IsEOpHandler (char* input, char* output, int* IsErip, int* Elfrip, int libSize)
{
    switch (input[*IsErip])
    {
        case IsE_HLT:
            output[(*Elfrip)++] = HLTb.byteInter[0];
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, HLToffset - libSize - *Elfrip);
            return HLT;
        case IsE_IN:
            output[(*Elfrip)++] = INb.byteInter[0];
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, INoffset - libSize - *Elfrip);
            return IN;
        case IsE_OUT:
            output[(*Elfrip)++] = OUTb.byteInter[0];
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, OUToffset - libSize - *Elfrip);
            return OUT;

        case IsE_JMP:
            output[(*Elfrip)++] = JMPb.byteInter[0];
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, 0); // as addres where to jump will be calculared later
            *IsErip += addressLength;                                               
            return JMP;

        case IsE_ADD:
            return IsEMathHandler (input, output, IsErip, Elfrip, ADD);  
        case IsE_DIV:
            return IsEMathHandler (input, output, IsErip, Elfrip, DIV);  
        case IsE_MUL:
            return IsEMathHandler (input, output, IsErip, Elfrip, MUL);  
        case IsE_SUB:
            return IsEMathHandler (input, output, IsErip, Elfrip, SUB);  
        case IsE_PUSH:
            (*IsErip)++;
            return IsEPushHandler (input, output, IsErip, Elfrip);

        case IsE_POP:
            (*IsErip)++;
            return IsEPopHandler  (input, output, IsErip, Elfrip);

        case IsE_JA:   
            return IsECondJmpHandler (input, output, IsErip, Elfrip, 2, JA);

        case IsE_JAE:   
            return IsECondJmpHandler (input, output, IsErip, Elfrip, 1, JAE);

        case IsE_JB:   
            return IsECondJmpHandler (input, output, IsErip, Elfrip, 5, JB);

        case IsE_JBE:   
            return IsECondJmpHandler (input, output, IsErip, Elfrip, 6, JBE);

        case IsE_JE:   
            return IsECondJmpHandler (input, output, IsErip, Elfrip, 4, JE);

        case IsE_JNE:   
            return IsECondJmpHandler (input, output, IsErip, Elfrip, 0, JNE);

        case IsE_CALL:
            output[(*Elfrip)++] = CALLb.byteInter[0];
            JmpRelatedOpHandler (input, output, IsErip, Elfrip, 0);
            *IsErip += addressLength;
            return CALL;

        case IsE_RET:
            output[(*Elfrip)++] = RETb.byteInter[0];
            (*IsErip)++;
            return RET;
            
        default:
            break;
    }
    return HLT;
}

IsE_Commands IsEMathHandler (char* input, char* output, int* IsErip, int* Elfrip, IsE_Commands commandId)
{
    for (int i = 0; i < Mathb.opSize; i++)
        output[(*Elfrip)++] = Mathb.byteInter[i];

    switch (commandId)
    {
    case ADD:
        output[(*Elfrip) - Mathb.opSize + MathModeByteOffset] = ADDop;
    break;

    case SUB:
        output[(*Elfrip) - Mathb.opSize + MathModeByteOffset] = SUBop;
    break;

    case MUL:
        output[(*Elfrip) - Mathb.opSize + MathModeByteOffset] = MULop;
    break;

    case DIV:
        output[(*Elfrip) - Mathb.opSize + MathModeByteOffset] = DIVop;
    break;
    default:
        break;
    }
    

    (*IsErip) ++;      
    return commandId;  
}

BIN_TRANSLATOR_ERRORS FindlableAdresses (char* input, command* commands, lable* lables, int commandN, unsigned int* lableN)
{
    for (int i = 0; i < commandN; i++)
    {
        fflush (stdout);
        if (commands[i].commandType <= JNE && commands[i].commandType >= CALL)
        {
            int* addr = (int*) (input + commands[i].IsEaddress + 1);

            for (int j = 0; j < commandN; j ++)
                if (commands[j].IsEaddress == *addr)
                {
                    lables[*lableN].commandN    = i;
                    lables[(*lableN)++].lableN  = j;
                    break;
                }
        }
            
    }
    return BTR_NO_ERRORS;
}

void SetAddress (char* output, command* commands, lable* lables, int i, int addrByte, int jmpLen)
{
    int* bytes = (int*)(output + commands[lables[i].commandN].Elfaddress + addrByte);
    *bytes = - addrByte - jmpLen - commands[lables[i].commandN].Elfaddress + commands[lables[i].lableN].Elfaddress;   
}

BIN_TRANSLATOR_ERRORS SetLabels (char* output, command* commands, lable* lables, int commandN, unsigned int lableN)
{
    int addrByte = 0;
    int jmpLen   = 0;
    for (int i = 0; i < lableN ; i ++)
    {
        
        switch (commands[lables[i].commandN].commandType)
        {
            case JMP:
            case CALL:
                SetAddress (output, commands, lables, i, JMPb.opSize, JMPb.argSize);
            break;

            case JA:
            case JAE:
            case JB:
            case JBE:
            case JE:
            case JNE:
                SetAddress (output, commands, lables, i, CONDjmp.opSize, CONDjmp.argSize);
            break;
        }

    }
    return BTR_NO_ERRORS;
}

void   JmpRelatedOpHandler (char* input, char* output, int* IsErip, int* Elfrip, int address)
{
    (*IsErip) ++;

    int* addr = (int*)(output + *Elfrip);
    *addr = address;

    (*Elfrip) += addressLength;  
}

IsE_Commands IsEPushHandler (char* input, char* output, int* IsErip, int* Elfrip)
{
    char delta = 0;
    switch (input[(*IsErip)++])
    {
    case 1:             // push number 
        for (int i = 0 ; i < 2; i++)
            output[(*Elfrip)++] = PUSHnb.byteInter[i]; 

        for (int i = 0; i < PUSHnb.argSize; i++)    
            output[(*Elfrip)++] = input [(*IsErip)++];

        for (int i = 2 ; i < PUSHnb.opSize; i++)
            output[(*Elfrip)++] = PUSHnb.byteInter[i];

        return PUSHn;

    case 2:             // push register
        delta = input[(*IsErip)++];
        for (int i = 0; i < PUSHrb.opSize; i++)
            output[(*Elfrip)++] = PUSHrb. byteInter[i];
            
        output [(*Elfrip) - PushRegOffset] += (delta * regNDelta);

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
    case 0:         // pop nowhere
        for (int i = 0; i < POPb.argSize; i++)
            output[(*Elfrip)++] = POPb.byteInter[i];

        return POP;

    case 2:         // pop to register
        delta = input[(*IsErip)++];
        for (int i = 0; i < POPrb.opSize; i++)
            output[(*Elfrip)++] = POPrb. byteInter[i];

        output [(*Elfrip) - PopRegOffset] += delta * regNDelta;
        return POPr;
    
    
    default:
        break;
    }
    return POP;
}

void SetUClabel (char* output, command* commands, lable* lables, unsigned int commandN, unsigned int lableN)
{
    int* bytes = (int*)(output + commands[commandN].Elfaddress + 1);
    *bytes = - commands[commandN].Elfaddress + commands[lableN].Elfaddress - addressLength - 1;
}

BIN_TRANSLATOR_ERRORS PUSHPOPOPTIMIZATION (char* output, command* commands, unsigned int* bufferSize, unsigned int commandN)
{
    printf ("bufferSize %d\n", *bufferSize);

    char* end = output + *bufferSize;
    for (int i = 0; i < *bufferSize; i++)
    {
        unsigned int currentCommandN = 0;
        if (!FindPUSHPOPsequence (&PUSHPOPs, output + i))
        {
            if (output[i + firstRegByteOffset] == output[i + firstRegByteOffset + secondRegByteOffset + 1])
                printf ("you will be optimised by first %d\n", i);

            if (output[i + firstRegByteOffset + secondRegByteOffset + 1] == 4)
            {
                printf ("you will be optimised by second %d\n", i);
                for (; currentCommandN < commandN; currentCommandN++)
                {
                    if (commands[currentCommandN].Elfaddress >= i)
                        break;
                }
                commands[currentCommandN++].Elfaddress -= (PUSHPOPs.nBytes/2);
                for (; currentCommandN < commandN; currentCommandN++)
                {
                    if (commands[currentCommandN].commandType == HLT ||
                        commands[currentCommandN].commandType == IN  ||
                        commands[currentCommandN].commandType == OUT )
                    {
                        int* temp = (int*)(output + commands[currentCommandN].Elfaddress + 1);
                        *temp += (PUSHPOPs.nBytes - movOpt.nBytes);
                    }
                    commands[currentCommandN].Elfaddress -= (PUSHPOPs.nBytes - movOpt.nBytes);
                }
                char* write = output + i;
                for (int j = 0; j < movOpt.nBytes - 1; j++)
                    *(write++) = (char)movOpt.bytes[j];
                switch (output[i + firstRegByteOffset])
                {
                case XMM2:
                    *(write++) = (char)0xc3;
                    break;
                case XMM3:
                    *(write++) = (char)0xc4;
                    break;
                case XMM4:
                printf ("tuta");
                    *(write++) = (char)0xc5;
                    break;
                case XMM5:
                    *(write++) = (char)0xc6;
                    break;
                default:
                    break;
                }
                printf ("\n\n%x\n\n", output[i + firstRegByteOffset]);
                char* read  = output + i + 18;
                while (read - end != 1)
                    *(write++) = *(read++);

                *bufferSize -= 14;
                


            }
                
        }
    }  
    printf ("optimisation ended\n");
    return BTR_NO_ERRORS;
}

int  FindPUSHPOPsequence (const byteSequence* first, char* buffer)
{
    if (strncmp((char*)first->bytes, buffer, firstRegByteOffset))
        return 1;

    if (strncmp((char*)(first->bytes + firstRegByteOffset + 1), buffer + firstRegByteOffset + 1, secondRegByteOffset))
        return 1;

    if (strncmp((char*)(first->bytes + firstRegByteOffset + secondRegByteOffset + 2), buffer + firstRegByteOffset + secondRegByteOffset + 2, thirdRegByteOffset))
        return 1;

    return 0;
}

IsE_Commands          IsECondJmpHandler (char* input, char* output, int* IsErip, int* Elfrip, char mode, IsE_Commands command)
{
    (*IsErip)+= addressLength + 1;

    for (int i = 0; i < CONDjmp.opSize; i++)
        output[(*Elfrip)++] = CONDjmp.byteInter[i];

    output [*Elfrip - CondAddressOffset] = mode;    

    (*Elfrip) += addressLength;     
    return command;
}

int  checkTranslatable (IsE_Header* header)
{
    if( header->signature == signature && header->version == 1)
        return BTR_NO_ERRORS;

    return BTR_WRONG_ECE_FORM;
}