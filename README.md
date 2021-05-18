# bin-translator

It is the last project of first course of university. This project is JIT-compiler for my assembler from first semestr. (Assembler was changed because it does not worked on Linux).

This project has several goals
- Discover structure of elf file and create own minimum elf file for x86-64 
- Compare speed of working of processor emulator from first semestr and real processor running program
- Make optimizations for Processor

## Making ELF file
We started with minimum ElF file. The purpose is to create ecxecutable fast - working program. So there is no debugging information in this ELF file. Structure of ELF was taken from this repository.

https://github.com/alpocnito/Create-elf-in-10-minuts

## Creating model of command translation
https://github.com/Krym4s/bin-translator/blob/main/screens/no_optimization.png
1. As we use our assembler from first semester we work mostly with stack. Our assembler let us work with double presicion numbers so I performed replacement of general-purpose registers to xmm registers. As in my assembler there are only general-purpose registers we will not lose opportunities of our assembler using this replacement. 

2. Math operations work with stack, so we need temporary registers to work with math operations.

Summarising this two ideas I have made decision to make xmm0 and xmm1 as temporary register for calculatings and xmm2 as rax, xmm3 as rbx, xmm4 as rcx and xmm5 as rdx.

Also to set same conditions for our processor emulator and real processor we have replaced second stack for addresses of returns rfom functions.

## Comparison of processor emulator and ELF file

So we executed counting of factorial on our emulator and compared it with execution on real processor. 

Processor emulator was compiled with O3 flag and all verifications was taken away.

The average working time of emulator is 3.431 seconds. 
<img align="center"  src="https://github.com/Krym4s/bin-translator/blob/main/screens/emulator.png">

The average working time of programm runned on real processor is 0.011 seconds.
<img align="center"  src="https://github.com/Krym4s/bin-translator/blob/main/screens/no_optimization.png">

To summarise, our bin tranlator gives about 343 percents growth in productivity.

## Implementation optimizations

The simpliest way to optomise our ELF file is to take away unnecessary Pops and Pushes. To implement this optimization we should remember relative address of every command and recalculate it before counting difference between addresses of labels and addresses of commands.

After removing all unnecessary pushes and pops we can see the results. 
<img align="center"  src="https://github.com/Krym4s/bin-translator/blob/main/screens/optimization.png">

Growth in speed is 10%. 
