#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "y86emul.h"

char * virtualMemory;
Flag flag;
int reg[8];     // 0 - eax, 1 - ebx, 2 - ecx, 3 - edx, 4 - esp, 5 - ebp, 6 - esi, 7 - edi
Status status = AOK;
const char delim[] = "\n\t";

/*
 *
 * MACHINE INSTRUCTIONS
 *
 * src - source
 * dst - destination
 * val - value
 * disp - displacement
 *
 */

void nop() {

}

void halt() {
    status = HLT;
}

void rrmovl(int src, int dst) {
    reg[dst] = reg[src];
}

void irmovl(int dst, int val) {
    reg[dst] = val;
}

void rmmovl(int src, int dst, int disp) {
    Long l;
    l.i = src;
    strncpy(&virtualMemory[dst+disp], &l.c[0], 4);
}

void mrmovl(int src, int dst, int disp) {
    int * i = (int*) &virtualMemory[dst+disp];
    reg[src] = *i;
}

void op1(int ins, int op, int dst) {
    int result;
    ins = ins & 0x0f;
    flag.OF = 0;
    flag.ZF = 0;
    flag.SF = 0;

    switch (ins) {
        case 0:     // addl
            result = reg[op] + reg[dst];
            if ((reg[op] > 0 && reg[dst] > 0 && result < 0) || (reg[op] < 0 && reg[dst] < 0 && result > 0)) {
                flag.OF = 1;
            }
            if (result == 0) {
                flag.ZF = 1;
            }
            if (result < 0) {
                flag.SF = 1;
            }
            reg[dst] = result;
            break;
        case 1:     // subl
            result = reg[dst] - reg[op];
            if ((reg[dst] > 0 && reg[op] < 0 && result < 0) || (reg[dst] < 0 && reg[op] > 0 && result > 0)) {
                flag.OF = 1;
            }
            if (result == 0) {
                flag.ZF = 1;
            }
            if (result < 0) {
                flag.SF = 1;
            }
            reg[dst] = result;
            break;
        case 2:     // andl
            reg[dst] &= reg[op];
            if (reg[dst] == 0) {
                flag.ZF = 1;
            }
            if (reg[dst] < 0) {
                flag.SF = 1;
            }
            break;
        case 3:     // xorl
            reg[dst] ^= reg[op];
            if (reg[dst] == 0) {
                flag.ZF = 1;
            }
            if (reg[dst] < 0) {
                flag.SF = 1;
            }
            break;
        case 4:     // mull
            result = reg[op] * reg[dst];
            if ((reg[dst] > 0 && reg[op] > 0 && result < 0) || (reg[dst] > 0 && reg[op] < 0 && result > 0) || (reg[dst] < 0 && reg[op] > 0 && result > 0) || (reg[dst] < 0 && reg[op] < 0 && result < 0)) {
                flag.OF = 0;
            }
            if (result == 0) {
                flag.ZF = 1;
            }
            if (result < 0) {
                flag.SF = 1;
            }
            reg[dst];
            break;
        case 5:     // cmpl
            result = reg[dst] - reg[op];
            if ((reg[dst] > 0 && reg[op] < 0 && result < 0) || (reg[dst] < 0 && reg[op] > 0 && result > 0)) {
                flag.OF = 1;
            }
            if (result == 0) {
                flag.ZF = 1;
            }
            if (result < 0) {
                flag.SF = 1;
            }
            break;
        default:
            status = INS;
            return;
    }
}

void jxx(int ins, int dst) {
    ins = ins & 0x0f;
    switch (ins) {
        case 0:     // jmp
            reg[6] = dst;
            break;
        case 1:     // jle
            if ((flag.SF^flag.OF)|flag.ZF) { reg[6] = dst; }
            else { reg[6] += 4; }
            break;
        case 2:     // jl
            if (flag.SF^flag.OF) { reg[6] = dst; }
            else { reg[6] += 4; }
            break;
        case 3:     // je
            if (flag.ZF) { reg[6] = dst; }
            else { reg[6] += 4; }
            break;
        case 4:     // jne
            if (~flag.ZF) { reg[6] = dst; }
            else { reg[6] += 4; }
            break;
        case 5:     // jge
            if (~(flag.SF^flag.OF)) { reg[6] = dst; }
            else { reg[6] += 4; }
            break;
        case 6:     // jg
            if (~(flag.SF^flag.OF)&~flag.ZF) { reg[6] = dst; }
            else { reg[6] += 4; }
            break;
        default:
            fprintf(stderr, "ERROR: Invalid opcode\n");
            status = INS;
            return;
    }
}

void call(int dst) {
printf("Calling %x\n", dst);
    pushl(6);
    reg[6] = dst;
}

void ret() {
    popl(6);
}

void pushl(int src) {
    reg[4] -= 1;
    virtualMemory[reg[4]] = reg[src];
}

void popl(int dst) {
    reg[dst] = virtualMemory[reg[4]];
    reg[4] += 1;
}

void readx(int ins, int dst, int disp) {
    flag.ZF = 0;
    Long l;
    if (ins == 0) {
        scanf("%x", &l.i);
        virtualMemory[dst] = (char) l.i;
    } else if (ins == 1) {
        scanf("%x", &l.i);
        strncpy(&virtualMemory[dst], &l.c[0], 4);
    } else {
        fprintf(stderr, "ERROR: Invalid opcode\n");
        status = INS;
        return;
    }
}

void writex(int ins, int src, int disp) {
    if (ins == 0) {
        printf("%c", virtualMemory[reg[src]+disp]);
    } else if (ins == 1) {
        printf("%c%c%c%c", virtualMemory[reg[src]+disp], virtualMemory[reg[src]+disp+1], virtualMemory[reg[src]+disp+2], virtualMemory[reg[src]+disp+3]);
    } else {
        fprintf(stderr, "ERROR: Invalid opcode\n");
        status = INS;
        return;
    }
}

void movsbl(int src, int dst, int disp) {
    char c;
    int x;
    c = virtualMemory[reg[src]+disp];
    if (c&0x80) {
        x = 0xffffff00|c;
    } else {
        x = 0x000000ff&c;
    }
    reg[dst] = x;
}

/*
 *
 * Y86 DIRECTIVES
 *
 */

// processes .text directive
void textDir(int address, char * value) {
    int i=0, x;
    char c[] = " ";
    Byte byte;

    reg[6] = address;

    while (i<strlen(value)) {
        c[0] = value[i++];
        sscanf(c, "%x", &x);
        byte.ByteHalf.low = x;
        c[0] = value[i++];
        sscanf(c, "%x", &x);
        byte.ByteHalf.high = x;
        strncpy(&virtualMemory[address++], &byte.byte, 1);
    }
}

// processes .string directive
void strDir(int address, char * value) {
    int i = 0;
    if (!(value[i++] == '"')) {
        fprintf(stderr, "ERROR: String must start with double quotes\n");
        exit(0);
    }

    while (value[i] != '"') {
        virtualMemory[address++] = value[i++];
    }
    virtualMemory[address] = '\0';
}

// processes .byte directive
void byteDir(int address, char * value) {
    int x;
    sscanf(value, "%x", &x);
    virtualMemory[address] = (char) x;
}

// processes .long directive
void longDir(int address, char * value) {
    Long x;
    sscanf(value, "%x", &x.i);
    strncpy(&virtualMemory[address], &x.c[0], 4);
}

int main(int argc, char**argv) {
    // for directives
    const int linelength = 1000;
    char line[linelength];
    char * token;
    int size = 0;                   // virtual memory size
    int textDirective = 0;          // was there a text directive?
    char directive[10];
    int address;
    char value[linelength];
    // for machine instructions
    char instruction;
    int src, dst, disp;
    int * l;                        // long, would have actually used the word long if it weren't already taken

    // check args
    if (argc != 2) {
        fprintf(stderr, "ERROR: Invalid argument. Use -h for instructions.\n");
        return 0;
    }
    if (strcmp(argv[1], "-h") == 0) {
        printf("Usage: ./y86emul <filename>\n");
        return 0;
    }
    
    // open file
    FILE * fptr;
    fptr = fopen(argv[1], "r");

    // check for fnf
    if (fptr == NULL) {
        fprintf(stderr, "ERROR: File not found.\n");
        return 0;
    } else {
        fgets(line, linelength, fptr);
    }

    // allocate memory
    token = strtok(line, delim);     // set token to ".size" (hopefully)
    if (strcmp(token, ".size") == 0) {
        token = strtok(NULL, delim);    // set token to .size parameter
        sscanf(token, "%x", &size);
        virtualMemory = (char*) malloc(sizeof(char)*size);
    } else {
        fprintf(stderr, "ERROR: First line must be '.size' directive\n");
        return 0;
    }

    // load program into memory
    while (fgets(line, linelength, fptr) != NULL) {
        token = strtok(line, delim);    // sets token to directive
        strcpy(directive, token);
        token = strtok(NULL, delim);    // sets token to address
        sscanf(token, "%x", &address);
        token = strtok(NULL, delim);    // sets token to value
        strcpy(value, token);

        if (strcmp(directive, ".size") == 0) {
            fprintf(stderr, "ERROR: Multiple '.size' directives found.\n");
            return 0;
        } else if (strcmp(directive, ".text") == 0 && textDirective == 0) {
            textDir(address, value);
            textDirective = 1;
            continue;
        } else if (strcmp(directive, ".text") == 0 && textDirective == 1) {
            fprintf(stderr, "ERROR: Multiple '.text' directives found.\n");
            return 0;
        } else if (strcmp(directive, ".byte") == 0) {
            byteDir(address, value);
            continue;
        } else if (strcmp(directive, ".string") == 0) {
            strDir(address, value);
            continue;
        } else if (strcmp(directive, ".long") == 0) {
            longDir(address, value);
            continue;
        } else {
            fprintf(stderr, "ERROR: Unknown directive found: %s\n", directive);
            return 0;
        }
    }
    /*
    printf("Memory Loaded\n");
    for(address=0; address<size; address++) {
        printf("%x\t", (unsigned char) virtualMemory[address]);
        if (address%8==0&&address!=0) { printf("\n"); }
    }
    printf("\n esi = %x\n", reg[6]);
    halt();
    */
    if (textDirective == 0) {
        fprintf(stderr, "ERROR: Text directive not found\n");
    }

    // begin process
    reg[4] = size - 1;
    while (status == AOK) {
        instruction = virtualMemory[reg[6]];
        reg[6] += 1;
        switch ((instruction>>4)&0x0f) {
            case 0:     // nop
                nop();
                break;
            case 1:     // halt
                halt();
                break;
            case 2:     // rrmovl
                src = (virtualMemory[reg[6]] >> 4) & 0x0f;
                dst = virtualMemory[reg[6]] & 0x0f;
                rrmovl(src, dst);
                reg[6] += 1;
                break;
            case 3:     // irmovl
                dst = virtualMemory[reg[6]] & 0x0f;
                reg[6] += 1;
                l = (int*) &virtualMemory[reg[6]];
                src = *l;
                irmovl(dst, src);
                reg[6] += 4;
                break;
            case 4:     // rmmovl
                src = (virtualMemory[reg[6]] >> 4) & 0x0f;
                dst = virtualMemory[reg[6]] & 0x0f;
                reg[6] += 1;
                l = (int*) &virtualMemory[reg[6]];
                disp = *l;
                rmmovl(src, dst, disp);
                reg[6] += 4;
                break;
            case 5:     // mrmovl
                dst = (virtualMemory[reg[6]] >> 4) & 0x0f;
                src = virtualMemory[reg[6]] & 0x0f;
                reg[6] += 1;
                l = (int*) &virtualMemory[reg[6]];
                disp = *l;
                rmmovl(src, dst, disp);
                reg[6] += 4;
                break;
            case 6:      // op1
                src = (virtualMemory[reg[6]] >> 4) & 0x0f;
                dst = virtualMemory[reg[6]] & 0x0f;
                op1(instruction&0x0f, src, dst);
                reg[6] += 1;
                break;
            case 7:      // jxx
                l = (int*) &virtualMemory[reg[6]];
                dst = *l;
                jxx(instruction&0x0f, dst);
                break;
            case 8:      // call
                l = (int*) &virtualMemory[reg[6]];
                dst = *l;
                call(dst);
                break;
            case 9:      // ret
                ret();
            case 10:     // pushl
                src = (virtualMemory[reg[6]] >> 4) & 0x0f;
                pushl(src);
                reg[6] += 1;
                break;
            case 11:     // popl
                dst = (virtualMemory[reg[6]] >> 4) & 0x0f;
                popl(dst);
                reg[6] += 1;
                break;
            case 12:     // readx
                src = (virtualMemory[reg[6]] >> 4) & 0x0f;
                reg[6] += 1;
                l = (int*) &virtualMemory[reg[6]];
                disp = *l;
                readx(instruction&0x0f, src, disp);
                reg[6] += 4;
                break;
            case 13:     // writex
                src = (virtualMemory[reg[6]] >> 4) & 0x0f;
                reg[6] += 1;
                l = (int*) &virtualMemory[reg[6]];
                disp = *l;
                writex(instruction&0x0f, src, disp);
                reg[6] += 4;
                break;
            case 14:     // movsbl
                dst = (virtualMemory[reg[6]] >> 4) & 0x0f;
                src = virtualMemory[reg[6]] & 0x0f;
                reg[6] += 1;
                l = (int*) &virtualMemory[reg[6]];
                disp = *l;
                movsbl(src, dst, disp);
                reg[6] += 4;
                break;
            default:
                fprintf(stderr, "ERROR: Invalid instruction");
                status = INS;
        }
    }

    printf("\nProgram stopped. Status code: ");
    switch (status) {
        case 0: printf("AOK\n"); break; // if this happens something went fucky
        case 1: printf("HLT\n"); break;
        case 2: printf("ADR\n"); break;
        case 3: printf("INS\n"); break;
    }    
    free(virtualMemory);
    return 0;
}
