#include "IsE_LinesF.h"


int getFileSize (FILE* file){
    assert (file);

    if (!file)
        return FILE_NOT_OPENED;

    fseek (file, 0, SEEK_END);
    int fileSize = ftell (file);
    fseek (file, 0, SEEK_SET);

    return fileSize;
}

//---------------------------------------------------------------

int readToStorage (const char* fileName, char** storage)
{
    assert (fileName);

    FILE* input = fopen (fileName, "rt");

    if (!input)
       return NO_LINE;

    if (!storage)
        return NO_LINE; 


    int fileSz = getFileSize (input);

    *storage = (char*) calloc (fileSz + 2, sizeof (*storage));
    assert (*storage);

    int nCh = fread (*storage, sizeof(char), fileSz, input);
    *(*storage + nCh)     = '\n';
    *(*storage + nCh + 1) = '\0';

    fclose (input);

    return CORRECT_LINE;
}

//------------------------------------------------------------------------

int countSymb (const char* storage, const char symb)
{

    assert (storage != NULL);

    int nStr = 0;

    //int ind = -1;

    char* pointerToSymb = (char*)(storage - 1);
    while ( (pointerToSymb = strchr(pointerToSymb + 1, symb)) != NULL)
        nStr++;
    return nStr;

}

//-------------------------------------------------------------------------

int deleteDoubleSymb (const char* storage, const char symb)
{
    assert (storage != NULL);

    char* read  = (char*) storage;
    char* write = (char*) storage;
    int nSymb = 0;

    while (*read != '\0')
    {
        while (*read == symb && *(read+1) == symb )
            read++;

        *write++ = *read++;
        nSymb++;
    }
    *write = '\0';

    return nSymb;
}

//------------------------------------------------------------------------

int substituteSymb (const char* storage, const char symbOld, const char symbNew)
{
    assert (storage != NULL);

    int nStr = 0;

    char* pointerToSymb = (char*)(storage - 1);
    while ( (pointerToSymb = strchr(pointerToSymb + 1, symbOld)) != NULL)
    {
        nStr++;
        *pointerToSymb = symbNew;
    }

    return nStr;
}

//------------------------------------------------------------------------

int deleteSymb (const char* storage,const char symb)
{
    assert (storage);

    char* read  = (char*) storage;
    char* write = (char*) storage;
    int nSymb = 0;

    while (*read != '\0')
    {
        while (*read == symb)
            read++;

        *write++ = *read++;
        nSymb++;
    }
    *write = '\0';

    return nSymb + 1;
}

//-----------------------------------------------------

int splitTextIntoLines (const char* storage, const int nStr, struct Text* lines)
{
    assert (storage);
    assert (lines);

    lines->lines = (struct Line*) calloc (nStr, sizeof(lines->lines));
    assert (lines->lines);

    int strInd = 0;

    char* beginOfString = (char*) storage;
    char* endOfString   = (char*) storage;

    while ((endOfString = strchr (beginOfString, '\n')) != nullptr)
    {
        (lines->lines + strInd)->line = beginOfString;
        (lines->lines + strInd)->length = endOfString - beginOfString + 1;
        strInd++;
        beginOfString = endOfString + 1;
    }

    lines->size = strInd;
    lines->errCode = CORRECT_LINE;

    return CORRECT_LINE;
}




