#ifndef ISE_ELF_HPP
#define ISE_ELF_HPP
#include <stdio.h>
#include <stdlib.h>

struct Elf_Header_x86_min
{
    const unsigned char      EI_MAG      [4] = {0x7f, 0x45, 0x4C, 0x46};
    const unsigned char      EI_CLASS        =  0x02;
    const unsigned char      EI_DATA         =  0x01;
    const unsigned char      EI_VERSION      =  0x01;
    const unsigned char      EI_OSABI        =  0x00;
    const unsigned char      EI_OSABIVER [8] = {0x00};
    const unsigned char      E_TYPE      [2] = {0x02, 0x00};
    const unsigned char      E_MACHINE   [2] = {0x3E, 0x00};
    const unsigned char      E_VERSION   [4] = {0x01, 0x00, 0x00, 0x00};
    const unsigned char      E_ENTRY     [8] = {0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
    const unsigned char      E_PHOFF     [8] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const unsigned char      E_SHOFF     [8] = {0x00};
    const unsigned char      E_FLAGS     [4] = {0x00};
    const unsigned char      E_EHSIZE    [2] = {0x40, 0x00};
    const unsigned char      E_PHENTSIZE [2] = {0x38, 0x00};
    const unsigned char      E_PHNUM     [2] = {0x01, 0x00};
    const unsigned char      E_SHENTSIZE [2] = {0x40, 0x00};
    const unsigned char      E_SHNUM     [2] = {0x00};
    const unsigned char      E_SHSTRNDX  [2] = {0x00};
};

struct Program_Header {

    const char      P_TYPE   [4]        = {0x01, 0x00, 0x00, 0x00};
    const char      P_FLAGS  [4]        = {0x05, 0x00, 0x00, 0x00};
    const char      P_OFFSET [8]        = {0x00};
    const char      P_VADDR  [8]        = {0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
    const char      P_PADDR  [8]        = {0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
    size_t          P_FILES             = 0x0;
    size_t          P_MEMSZ             = 0x0;
    const char      P_ALIGN  [8]        = {0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const char      P_SPAC   [8]        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

};

#endif 