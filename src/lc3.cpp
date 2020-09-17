#define LC3_DEBUG_BUILD

#ifdef LC3_DEBUG_BUILD
    #define DEBUG(x) do{x}while(0)
#else
    #define DEBUG(x) do{}while(0)
#endif

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <netinet/in.h>
#include "enums.hpp"

const uint16_t PC_START = 0x3000;
uint16_t mem[UINT16_MAX];   // Array addressable with a 16-bit uint with 16-bit width.
uint16_t reg[R_COUNT];      // Array containing all 16-bit registers.


// Utility
uint16_t lc3_memread(uint16_t addr) {
    return mem[addr];
}

void lc3_memwrite(uint16_t addr, uint16_t x) {
    mem[addr] = x;
}

uint16_t signExtend(uint16_t x, int nbits) {
    // Pad a n-bit number to its 16-bit two's complement representation.
    if ((x >> (nbits - 1)) & 0x1)
        // Pad negative number with 1
        return x | 0xFFFF << nbits;
    // Pad positive number with 0
    return x;
}

uint16_t ltob(uint16_t x) {
    // Convert to big endian using network functions.
    return htons(x);
}


// Functionality
bool readBin(const char* fileName) {
    // Read lc3 binary.
    // Returns true on success, false on failure.
    FILE* file;
    file = fopen(fileName, "rb");

    if (file == NULL) {return false;}

    // Read program origin.
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = ltob(origin);

    // Read program into memory.
    uint16_t maxRead = UINT16_MAX - origin;
    uint16_t* p = mem + origin;
    size_t read = fread(p, sizeof(uint16_t), maxRead, file);

    // Flip byte order to big endian.
    while (read-- > 0) {
        *p = ltob(*p);
        ++p;
    }

    fclose(file);
    return true;
}

void updateFlags(uint16_t r) {
    // R_COND must be updated whenever a
    // number is written to a register.
    if (reg[r] == 0)
        reg[R_COND] = FL_ZRO;
    else if (reg[r] >> 15)
        // Check leftmost bit for sign.
        reg[R_COND] = FL_NEG;
    else
        reg[R_COND] = FL_POS;
}


// Read and execute LC3 .obj files.
int main(int argc, char *argv[]) {
    if (argc < 2) {
        // Print usage.
        std::cout << "lc3 [lc3_bin.obj] ..." << std::endl;
        exit(1);
    }

    // Read binaries into memory.
    for (int i = 1; i < argc; ++i) {
        if (!readBin(argv[1])) {
            std::cerr << "Error loading " << argv[1] << std::endl;
            exit(1);
        }
    }
     

    reg[R_PC] = PC_START; // Set PC register to starting address.

    uint16_t instr, op;
    bool running = true;
    while (running) {
        instr = lc3_memread(reg[R_PC]++); // Read instruction from loaded program.
        op = instr >> 12;
        switch(op) {
            // TODO: Implement instructions.
            case OP_BR: {
                /*
                  Test NEG, ZRO, POS conditions.
                  bit 11, n: Test for NEG
                  bit 10, z: Test for ZRO
                  bit 9,  p:  Test for POS
                  If any conditions are true, goto R_PC + PCoffset9.
                */
                bool nzp = (instr >> 9) & 0x7;
                if ((nzp & reg[R_COND]) > 0) {
                    uint16_t PCoffset9 = signExtend(instr & 0x1FF, 9);
                    reg[R_PC] += PCoffset9;
                }
            } break;
            case OP_ADD: {
                /*
                  Register mode: Store sum of values in two registers in a third register.
                  Immediate mode: Store sum of the value in register and a 5-bit number.
                */
                uint16_t dr = (instr >> 9) & 0x7; // Dest. register
                uint16_t r1 = (instr >> 6) & 0x7;

                if ((instr >> 5) & 0x1) {
                    // Immediate mode, 5th bit is 1
                    reg[dr] = reg[r1] + signExtend(instr & 0xF, 5);
                } else {
                    // Register mode
                    uint16_t r2 = instr & 0x7;
                    reg[dr] = reg[r1] + reg[r2];
                }
                updateFlags(dr); 
            } break;
            case OP_LD: {
                /*
                  Load contents of the address R_PC + PCoffset9
                  into a destination register.
                */
                uint16_t dr = (instr >> 9) & 0x7; // Dest. register
                uint16_t PCoffset9 = signExtend(instr & 0x1FF, 9);

                reg[dr] = lc3_memread(reg[R_PC] + PCoffset9);
                updateFlags(dr);
            } break;
            case OP_ST: {
                /*
                  Store contents of register r1 into 
                  memory location R_PC + PCoffset9.
                */
                uint16_t r1 = (instr >> 9) & 0x7;
                uint16_t PCoffset9 = signExtend(instr & 0x1FF, 9);
                
                lc3_memwrite(reg[R_PC] + PCoffset9, reg[r1]);
            } break;
            case OP_JSR: {
                /*
                  Store R_PC in R_R7 and set R_PC to
                  R_PC + PCoffset11 if bit 11 is 1
                  Value in register r1 if bit 11 is 0
                */
                reg[R_R7] = reg[R_PC];
                if ((instr >> 11) & 0x1) {
                    uint16_t PCoffset11 = signExtend(instr & 0x7FF, 11);
                    reg[R_PC] += PCoffset11;
                } else {
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];
                }
            } break;
            case OP_AND: {

            } break;
            case OP_LDR: {

            } break;
            case OP_STR: {

            } break;
            case OP_RTI: {

            } break;
            case OP_NOT: {

            } break;
            case OP_LDI: {
                /*
                  Store a value in memory into a register. The 
                  location of this value is pointed to by a memory location
                  which is offset from the program counter by a 9-bit number.
                */
                uint16_t dr = (instr >> 9) & 0x7; // Dest. register
                uint16_t PCoffset9 = signExtend(instr & 0x1FF, 9);

                reg[dr] = lc3_memread(lc3_memread(reg[R_PC] + PCoffset9));
                updateFlags(dr);
            } break;
            case OP_STI: {

            } break;
            case OP_JMP: {

            } break;
            case OP_RES: {

            } break;
            case OP_LEA: {

            } break;
            case OP_TRAP: {
                switch (op & 0xFF) {
                    // TODO: Implement trap routines.
                    case TRAP_GETC:
                        break;
                    case TRAP_OUT:
                        break;
                    case TRAP_PUTS:
                        break;
                    case TRAP_IN:
                        break;
                    case TRAP_PUTSP:
                        break;
                    case TRAP_HALT:
                        break;
                }
            } break;
            default:
                // TODO: Implement bad opcode handling.
                break;
        }
    }
}
