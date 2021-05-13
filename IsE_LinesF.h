#ifndef ISE_TEXT_H_INCLUDED
#define ISE_TEXT_H_INCLUDED

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

enum LINE_FUNCTION_ERROR
{
    CORRECT_LINE    =  0,
    FILE_NOT_OPENED = -1,
    NO_LINE         = -2
};

struct Line 

{
    char*    line;
    unsigned length;
};

struct Text
{
    struct Line*        lines;
    unsigned            size;
    LINE_FUNCTION_ERROR errCode;
};

//!---------------------------------------------------
//! gets size of file in bytes
//!
//! @param [in] fileName- file 
//!
//! @return size of file in bytes
//!
//! @note size of file in bytes often is bigger than
//!       number of char symbols in file
//!----------------------------------------------------

int getFileSize (FILE* file);



//!-------------------------------------------------------
//!         reads data from file to storage
//!
//! @param  [in] fileName-name of file or filepath
//! @param  [in] storage-buffer of char
//!
//! @return error code
//!
//! @note   returns string that is not normal for c
//!         format of string is
//!-------------------------------------------------------

int readToStorage (const char* fileName, char** storage);

//!---------------------------------------------------------------------------
//!         counts number of input symbols in input string
//!
//! @param  [in] storage-pointer to buffer consisting string
//! @param  [in] symb-symbol which we find
//!
//! @return number of string
//!
//!---------------------------------------------------------------------------

int countSymb (const char* storage, const char symb);

//!---------------------------------------------------------------------------
//!         deletes every double symbol from input and returns size of new storage without double symb;
//!
//! @param  [in] storage-buffer consisting string
//! @param  [in] symb-symbol which we want to delete
//!
//! @return size of new storage without double symb;
//!
//!---------------------------------------------------------------------------

int deleteDoubleSymb (const char* storage, const char symb);

//!---------------------------------------------------------------------------
//!         deletes every double symbol from input and returns size of new storage without double symb;
//!
//! @param  [in] storage-buffer consisting string
//! @param  [in] symb-symbol which we want to delete
//!
//! @return size of new storage without double symb;
//!
//!---------------------------------------------------------------------------

int deleteSymb (const char* storage, const char symb);

//!---------------------------------------------------------------------------
//!         deletes every double symbol from input and returns size of new storage without double symb;
//!
//! @param  [in] storage-buffer consisting string
//! @param  [in] symbOld-symbol which we want to substitute
//! @param  [in] symbNew-symbol which we want to add
//!
//! @return number of sustituted symbols;
//!
//!---------------------------------------------------------------------------

int substituteSymb (const char* storage, const char symbOld, const char symbNew);

//!--------------------------------------------------------------------------
//!         converts sting to array of Strings
//!
//! @param  [in] storage-buffer consisting string
//! @param  [in] nStr-number of string
//! @param  [out] lines-array of lines
//!
//! @return error code
//!
//!---------------------------------------------------------------------------

int splitTextIntoLines (const char* storage, const int nStr, struct Text* lines);



#endif 
