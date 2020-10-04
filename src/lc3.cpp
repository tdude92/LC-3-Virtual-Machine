//#define LC3_DEBUG_BUILD       // Defining this will write the register states
                                // before each instruction execution to a log file.
#ifdef LC3_DEBUG_BUILD
    #define DEBUG(x) do{x}while(0)
#else
    #define DEBUG(x) do{}while(0)
#endif

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <csignal>
#include <netinet/in.h>
#include "input_buffering.h"
#include "enums.hpp"

#include <fstream>

const uint16_t PC_START = 0x3000;
uint16_t mem[UINT16_MAX];   // Array addressable with a 16-bit uint with 16-bit width.
uint16_t reg[R_COUNT];      // Array containing all 16-bit registers.


// Utility
uint16_t lc3_memread(uint16_t addr) {
    if (addr == MR_KBSR) {
        if (check_key()) {
            mem[MR_KBSR] = 1 << 15;
            mem[MR_KBDR] = getchar();
        } else {
            mem[MR_KBSR] = 0;
        }
    }
    return mem[addr];
}

void lc3_memset(uint16_t addr, uint16_t x) {
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
    
    // Set console to unbuffered input.
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    reg[R_PC] = PC_START; // Set PC register to starting address.

    // Set up file ostream to write instructions and register values.
    std::fstream log;
    DEBUG(
        log.open("execution.log", std::ios::out | std::ios::trunc);
        std::cout << "Writing instruction execution and register states to log file." << std::endl;
    );

    uint16_t instr, op;
    bool running = true;
    while (running) {
        instr = lc3_memread(reg[R_PC]++); // Read instruction from loaded program.

        DEBUG(
            // Write register values.
            log     << "Instr: " << std::hex << instr << "\n";
            log     << "R0:" << std::hex << reg[R_R0] << " "
                    << "R1:" << std::hex << reg[R_R1] << " "
                    << "R2:" << std::hex << reg[R_R2] << " "
                    << "R3:" << std::hex << reg[R_R3] << " "
                    << "R4:" << std::hex << reg[R_R4] << " "
                    << "R5:" << std::hex << reg[R_R5] << " "
                    << "R6:" << std::hex << reg[R_R6] << " "
                    << "R7:" << std::hex << reg[R_R7] << " "
                    << "PC:" << std::hex << reg[R_PC] - 1 << " "
                    << "COND: " << reg[R_COND] << "\n" << std::endl;
        );

        op = instr >> 12;
        switch(op) {
            case OP_BR: {
                /*
                  Test NEG, ZRO, POS conditions.
                  bit 11, n: Test for NEG
                  bit 10, z: Test for ZRO
                  bit 9,  p:  Test for POS
                  If any conditions are true, goto R_PC + PCoffset9.
                */
                uint16_t nzp = (instr >> 9) & 0x7;
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
                    reg[dr] = reg[r1] + signExtend(instr & 0x1F, 5);
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
                
                lc3_memset(reg[R_PC] + PCoffset9, reg[r1]);
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
                /*
                  Perform a bitwise-AND operation on
                  If bit 5 is 0: r1 and r2
                  If bit 5 is 1: r1 and number in bits 0 to 4.
                  Store result in dr.
                */
                uint16_t dr = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                if ((instr >> 5) & 0x1) {
                    reg[dr] = reg[r1] & signExtend(instr & 0x1F, 5);
                } else {
                    uint16_t r2 = instr & 0x7;
                    reg[dr] = reg[r1] & reg[r2];
                }
                updateFlags(dr);
            } break;
            case OP_LDR: {
                /*
                  Retrieve value stored in memory and store into dr.
                  The address is the value in a register r1 + offset6
                */
                uint16_t dr = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset6 = signExtend(instr & 0x3F, 6);

                reg[dr] = lc3_memread(reg[r1] + offset6);
                updateFlags(dr);
            } break;
            case OP_STR: {
                /*
                  The value in r1 is stored into memory at
                  the address with the value (r2 + offset6)
                */
                uint16_t r1 = (instr >> 9) & 0x7;
                uint16_t r2 = (instr >> 6) & 0x7;
                uint16_t offset6 = signExtend(instr & 0x3F, 6);

                lc3_memset(reg[r2] + offset6, reg[r1]);
            } break;
            case OP_RTI: {
                /*
                  Unused. Throws error.
                */
                std::cerr << "OP_RTI: operation not implemented." << std::endl;
                exit(1);
            } break;
            case OP_NOT: {
                /*
                  Store the bitwise complement of the value in r1 into dr.
                */
                uint16_t dr = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                
                reg[dr] = ~reg[r1];
                updateFlags(dr);
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
                /*
                  The value in r1 is stored at the memory location
                  pointed to by the memory location R_PC + PCoffset9.
                */
               uint16_t r1 = (instr >> 9) & 0x7;
               uint16_t PCoffset9 = signExtend(instr & 0x1FF, 9);

               lc3_memset(lc3_memread(reg[R_PC] + PCoffset9), reg[r1]);
            } break;
            case OP_JMP: {
                /*
                  Set R_PC to the value specified in r1.
                */
                uint16_t r1 = (instr >> 6) & 0x7;

                reg[R_PC] = reg[r1];
            } break;
            case OP_RES: {
                /*
                  Unused. Throws error.
                */
                std::cerr << "OP_RES: operation not implemented." << std::endl;
                exit(1);
            } break;
            case OP_LEA: {
                /*
                    Load the address given by R_PC + PCoffset9 into dr.
                */
                uint16_t dr = (instr >> 9) & 0x7;
                uint16_t PCoffset9 = signExtend(instr & 0x1FF, 9);

                reg[dr] = reg[R_PC] + PCoffset9;
                updateFlags(dr);
            } break;
            case OP_TRAP: {
                switch (instr & 0xFF) {
                    case TRAP_GETC: {
                        /*
                        Read an ASCII char
                        */
                        reg[R_R0] = (uint16_t)getchar();
                    } break;
                    case TRAP_OUT: {
                        /*
                          Write a character in R_R0[7:0]
                        */
                        std::cout << (char)reg[R_R0];
                        std::flush(std::cout);
                    } break;
                    case TRAP_PUTS: {
                        /*
                          Write a sequence of characters in adjacent memory locations
                          to the output. Writing stops at a location with value 0x0000.
                        */
                        uint16_t* c = mem + reg[R_R0];
                        while (*c) {
                            std::cout << (char)*c;
                            ++c;
                        }
                        std::flush(std::cout);
                    } break;
                    case TRAP_IN: {
                        /*
                          Prompt for a character input. Input is echoed and stored into R_R0.
                        */
                        std::cout << "Enter a character: " << std::flush;
                        char c = getchar();
                        std::cout << c << std::flush;
                        reg[R_R0] = (uint16_t)c; 
                    } break;
                    case TRAP_PUTSP: {
                        /*
                          Same as PUTS, but two chars are stored in a memory location.
                        */
                        uint16_t* c = mem + reg[R_R0];
                        while (*c) {
                            char c1 = (*c) & 0xFF;
                            char c2 = (*c) >> 8;

                            std::cout << c1;
                            if (c2)
                                std::cout << c2;

                            ++c;
                        }
                        std::flush(std::cout);
                    } break;
                    case TRAP_HALT: {
                        /*
                          Halts the program.
                        */
                        std::cout << "HALT" << std::endl;
                        running = false;
                    } break;
                }
            } break;
            default:
                // TODO: Implement bad opcode handling.
                break;
        }
    }

    restore_input_buffering();
    return 0;
}
