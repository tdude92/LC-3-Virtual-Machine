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

// Memory mapped registers
enum MM_Reg {
    MR_KBSR = 0xFE00, // Keyboard status
    MR_KBDR = 0xFE02  // Keyboard data
};

// Trap codes
enum TrapCode {
    TRAP_GETC   = 0x20, // Get character from keyboard, don't echo onto terminal.
    TRAP_OUT    = 0x21, // Output a character.
    TRAP_PUTS   = 0x22, // Output a word string.
    TRAP_IN     = 0x23, // Get character from keyboard, echo onto terminal.
    TRAP_PUTSP  = 0x24, // Output a byte string.
    TRAP_HALT   = 0x25  // Halt the program.
};

// Condition flags
enum Flag {
    FL_POS = 1,        // P
    FL_ZRO = 1 << 1,   // Z
    FL_NEG = 1 << 2,   // N
};

#endif