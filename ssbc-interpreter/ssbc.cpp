#include <cstdio>

// opcodes
enum {
    op_noop = 0,      // no operation
    op_halt = 1,      // halt
    op_pushimm = 2,   // push immediate
    op_pushext = 3,   // push external
    op_popinh = 4,    // pop inherent
    op_popext = 5,    // pop external
    op_jnz = 6,       // jump not zero
    op_jnn = 7,       // jump not negative
    op_add = 8,       // add
    op_sub = 9,       // subtract
    op_nor = 10,      // nor
};

enum {
    state_ins_int,      // instruction interpertation
    state_ins_exe,      // instruction execution
    // state_noop,         // no operation
    // state_halt,         // halt
    // state_pushimm,      // push immediate
    // state_pushext,      // push external
    // state_popinh,       // pop inherent
    // state_popext,       // pop external
    // state_jnz,          // jump not zero
    // state_jnn,          // jump not negative
    // state_add,          // add
    // state_sub,          // subtract
    // state_nor,          // nor
};

const int mem_size = 0xFFFF;

char MEM[mem_size];     // main memory
int PC = 0;             // program counter
int SP = mem_size-1;    // stack pointer
int MR = 0;             // memory register
char R0 = 0;            // register 0
char R1 = 0;            // register 1
char R2 = 0;            // register 2
char R3 = 0;            // register 3
char IR = 0;            // instruction register
/* 1-bit indicators */
bool FAULT = false;
bool HALT = false;
bool RESET = false;
// memory map
enum {
    map_PSW = 0xFFFB,
    map_portA = 0xFFFC,
    map_portB = 0xFFFD,
    map_portC = 0xFFFE,
    map_portD = 0xFFFF
};

// ports A,B,C,D
inline char portA() { return MEM[map_portA]; }
inline char portB() { return MEM[map_portB]; }
inline char portC() { return MEM[map_portC]; }
inline char portD() { return MEM[map_portD]; }

inline char PSW() { return MEM[map_PSW]; }              // program status word
inline char Z_set() { return (PSW() >> 7) & 1; }        // zero flag bit
inline char N_set() { return (PSW() >> 6) & 1; }        // not flag bit
inline char ii() { return MEM[PC]; }                    // current instruction
inline char s1() { return MEM[SP+1]; }                  // first on stack
inline char s2() { return MEM[SP+2]; }                  // second on stack
// external addressing
inline int ext() { return (0xFF00 & (MEM[PC] << 8)) | (0x00FF & MEM[PC+1]); }

int main() {

}