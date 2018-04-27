/*
 *    Project-based Learning II (CPU)
 *
 *    Program:    instruction set simulator of the Educational CPU Board
 *    File Name:    cpuboard.h
 *    Descrioption:    resource definition of the educational computer board
 */

/*=============================================================================
 *   Architectural Data Types
 *===========================================================================*/
typedef signed char    Sword;
typedef unsigned char    Uword;
typedef unsigned short    Addr;
typedef unsigned char    Bit;


/*=============================================================================
 *   CPU Board Resources
 *===========================================================================*/
#define MEMORY_SIZE    256*2
#define IMEMORY_SIZE   256
#define PROGRAM_MEMORY_FRONT 0
#define DATA_MEMORY_FRONT 256

#define A  0b0000
#define VF 0b1000
#define NZ 0b0001
#define Z  0b1001
#define ZP 0b0010
#define N  0b1010
#define P  0b0011
#define ZN 0b1011
#define NI 0b0100
#define NO 0b1100
#define NC 0b0101
#define C  0b1101
#define GE 0b0110
#define LT 0b1110
#define GT 0b0111
#define LE 0b1111

#define ACC 0b000
#define IX 0b001
#define IMMEDIATE 0b010
#define P_ABSOLUTLY 0100
#define D_ABSOLUTLY 0b101
#define P_INDIRECTION 0b110
#define D_INDIRECTION 0b111

#define RA 0b00
#define LA 0b01
#define RL 0b10
#define LL 0b11

#define _NOP 0x00
#define _HLT 0x0c
#define _OUT 0x10
#define _IN  0x18
#define _RCF 0x20
#define _SCF 0x28
#define _LD  0x60
#define _ST  0x70
#define _ADD 0xb0
#define _ADC 0x90
#define _SUB 0xa0
#define _SBC 0x80
#define _CMP 0xf0
#define _AND 0xe0
#define _OR  0xd0
#define _EOR 0xc0

#define _RSM 0x40
#define _BBC 0x30
#define _JAL 0xfa
#define _JR  0xfb
//独自定義
#define _SRA 0x11
#define _SLA 0x12
#define _SRL 0x13
#define _SLL 0x14
#define _RRA 0x15
#define _RLA 0x16
#define _RRL 0x17
#define _RLL 0x19



#define TRUE 1
#define FALSE 0

typedef struct iobuf {
    Bit    flag;
    Uword    buf;
} IOBuf;

typedef struct cpuboard {
    Uword pc;
    Uword acc;
    Uword ix;
    Uword ir[2];
    Uword *regA;
    Uword *regB;
    Bit   cf, vf, nf, zf;
    IOBuf *ibuf;
    IOBuf *obuf;
    Uword dmem_add;
    Uword mem[MEMORY_SIZE];    /* 0XX:Program, 1XX:Data */
    Uword data_size;
    Uword register_mode;
    Uword load_register;
    Uword shift_mode;
    Uword immediate_reg;
    struct calculated_A{
        Uword uword_A;
        Sword sword_A;
    } calc_A;
    struct calculated_B{
        Uword uword_B;
        Sword sword_B;
    } calc_B;
} Cpub;

/*=============================================================================
 *   Top Function of an Instruction Simulation
 *===========================================================================*/
#define    RUN_HALT    0
#define    RUN_STEP    1
int selectAResister(Cpub *, int);
int selectBResister(Cpub *, int);
int step(Cpub *);
int HLT(Cpub *);
int NOP(Cpub *);
int OUT(Cpub *);
int IN(Cpub *);
int RCF(Cpub *);
int SCF(Cpub *);
int Bbc(Cpub *);
int Ssm(Cpub *);
int Rsm(Cpub *);
int LD(Cpub *);
int ST(Cpub *);
int JAL(Cpub *);
int JR(Cpub *);
int ADD(Cpub *);
int ADC(Cpub *);
int SUB(Cpub *);
int SBC(Cpub *);
int CMP(Cpub *);
int AND(Cpub *);
int OR(Cpub *);
int EOR(Cpub *);
int blanchCondition(Cpub *, Bit);
void pcUpdate(Cpub *);
















