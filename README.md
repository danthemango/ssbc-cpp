ssbc-cpp
========
The SSBC (Simple Stack-Based Computer) implemented in C++

TODO
====
- [ ] build ssbc interpreter
- [ ] 
- [ ] build bin2int tool
- [ ] build int2bin tool
- [ ] build 'cpp2assem' C++ to SSBC assembly transpiler
- [x] build 'assem2mac' ssbc assembly to machine code program
- [x] build linemac program (adds hex line number and hex->binary information to each line)
- [ ] build mac2bin machine code to binary program
- [ ] build disassembler (?)

SSBC Machine Code (.mac)
========================
- [ ] todo write

SSSBC Assembly (.s)
===================
A number of rules were added in order to implement this version of ssbc assembly:
- an address may be labelled with in the format '#mylabel', and used in the form of '@mylabel'
    - labels must be added immediately prior to the operation or byte to be labelled
    - for example the following substitution may be expected, where we are use jnz to jmp to the
    address where 'jnz' is found:

| .s code        | .mac code                |
| -------------  | --------------------     |
| #myRoutine     | #myRoutine               |
| jnz            | 00000111 jnz             | 
| @myRoutine     | 00000000 @myRoutine      |

- to add a variable, just add a label and a value:

| .s code        | .mac code       |
| -------------  | --------------- |
| #myVar 0xFA    | 11111010 #myVar |

- use .H and .L to specify an addresses high and low byte, e.g. '@myRoutine.H'
- you may use offset addresses if necessary, for example '@myRoutine+5' to jump to
    5 bytes after '#myRoutine'
- you must use leading 0's to differenciate a 2-byte hex value from a 1 byte hex value
    - For example, since jnz is an extended-mode operation, you may use:
        - 'jnz 0x00AD or 'jnz 0x0A 0x0F'
    - this is done to allow tagging of the second byte of operation
    - hex values shall be only 1 or 2 bytes in length
- decimal values shall be assumed to a value of the size expected
    - 1 byte by default
    - 1 byte for immediate mode addressing
    - 2 bytes for extended mode of addressing (so extra padding shall be added for pushext for
        example)
- single-line comments may be labelled with ';', '//'

CPP compiler
============
This will only be a partial implementation of the C++, with the following parts implemented:
- [ ] inline ssbc assembly
- [ ] global variables
- [ ] function definitions
- [ ] local variables
- [ ] arrays
- [ ] dynamic memory allocations?
- [ ] classes
    - [ ] member variables
    - [ ] member functions

SSBC will be loaded assembly code which jumps to a function labelled 'main',
main will return an int, and this int shall be placed into portA, with a halt operation.


'main' shall be a function, 

Since the unique architecture of an SSBC machine, 

inline ssbc assembly is c