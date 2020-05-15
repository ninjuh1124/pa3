#ifndef Y86EMUL
#define Y86EMUL

// struct gives access to upper and lower half bits
typedef union Byte {
    unsigned char byte;
    struct ByteHalf {
        unsigned int high:4;
        unsigned int low:4;
    } ByteHalf;
}Byte;

// emulator status codes
typedef enum Status {
    AOK,    // continue
    HLT,    // halt
    ADR,    // invalid address
    INS     // invalid instruction
} Status;

// emulator processor registers
typedef struct Register {
    int eax;   // general registers
    int ecx;
    int edx;
    int ebx;
    int esp;   // stack pointer
    int ebp;   // frame pointer
    int esi;   // instruction pointer
    int edi;   // destination address
} Register;

// emulator condition flags
typedef struct Flag {
    unsigned int OF:1;  // overflow
    unsigned int ZF:1;  // zero
    unsigned int SF:1;  // negative
} Flag;

typedef union Long {
    unsigned int i;
    char c[4];
} Long;

void nop();
void halt();
void rrmovl(int, int);
void irmovl(int, int);
void rmmovl(int, int, int);
void mrmovl(int, int, int);
void op1(int, int, int);
void jxx(int, int);
void call(int);
void ret();
void pushl(int);
void popl(int);
void readx(int, int, int);
void writex(int, int, int);
void movsbl(int, int, int);
void textDir(int, char*);
void strDir(int, char*);
void byteDir(int, char*);
void longDir(int, char*);

#endif
