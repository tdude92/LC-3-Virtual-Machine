#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <netinet/in.h>
#include "enums.hpp"

const uint16_t PC_START = 0x3000;
uint16_t mem[UINT16_MAX];   // Array addressable with a 16-bit uint with 16-bit width.
uint16_t reg[R_COUNT];      // Array containing all 16-bit registers.

// TODO: Write tests for utility functions.

uint16_t ltob(uint16_t x) {
    // Convert to big endian using network functions.
    return htons(x);
}

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

void lc3_memset(uint16_t addr, uint16_t val) {
    mem[addr] = val;
}

uint16_t lc3_memread(uint16_t addr) {
    return mem[addr];
}


// Reading and executing LC3 .obj files.
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

    uint16_t instruction, op;
    bool running = true;
    while (running) {
        instruction = lc3_memread(reg[R_PC]++); // Read instruction from loaded program.
        op = instruction >> 12;                 // TODO: Why tf do we need this.

        switch(op) {
            // TODO: Implement instructions.
            case OP_TRAP:
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
                break;
            default:
                // TODO: Implement bad opcode handling.
                break;
        }
    }
}
