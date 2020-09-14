#ifndef LC3_VM_ENUMS_HPP
#define LC3_VM_ENUMS_HPP

// Registers
enum Register {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,       // Program Counter
    R_COND,     // Condition Flag Register
    R_COUNT     // No. of Registers
};

// Instruction set
enum Operation {
    OP_BR = 0, // Branch
    OP_ADD,    // Add
    OP_LD,     // Load
    OP_ST,     // Store
    OP_JSR,    // Jump Register
    OP_AND,    // Bitwise And
    OP_LDR,    // Load Register
    OP_STR,    // Store Register
    OP_RTI,    // Unused
    OP_NOT,    // Bitwise Not
    OP_LDI,    // Load Indirect
    OP_STI,    // Store Indirect
    OP_JMP,    // Jump
    OP_RES,    // Reserved (Unused)
    OP_LEA,    // Load Effective Address
    OP_TRAP,   // Execute Trap
};

// Condition flags
enum Flag {
    FL_POS = 1,        // P
    FL_ZRO = 1 << 1,   // Z
    FL_NEG = 1 << 2,   // N
};

#endif