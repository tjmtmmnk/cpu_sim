/*
 *    Project-based Learning II (CPU)
 *
 *    Program:    instruction set simulator of the Educational CPU Board
 *    File Name:    cpuboard.c
 *    Descrioption:    simulation(emulation) of an instruction
 */
#include <stdio.h>
#include <math.h>
#include  "cpu_board.h"

void fetch(Cpub *board){
    for (int i=0; i<2; i++) {
        board->ir[i] = 0x00;
    }
    if((unsigned int)board->pc > MEMORY_SIZE){
        fprintf(stderr, "memory index error");
        return;
    }
    board->ir[0] = board->mem[board->pc++];
    printf("pc : %d\n",board->pc);
}

void setCF(Cpub *board, int mode){ //TODO:S/R系の実装
    switch (mode) {
        case _RCF:
            board->cf = 0;
            break;
        case _SCF:
            board->cf = 1;
            break;
        case _ADC:
        case _SBC:
            if((Uword)(board->calc_A.uword_A + board->calc_B.uword_B) < board->calc_A.uword_A){
                board->cf = 1;
            }
            break;
        default:
            break;
    }
}

void setVF(Cpub *board, int mode){ //TODO:S/R系の実装
    switch (mode) {
        case _ADD:
        case _ADC:
        case _SUB:
        case _SBC:
        case _CMP:
            if((board->calc_A.sword_A < 0 && board->calc_B.sword_B < 0 &&
                (Sword)(board->calc_A.sword_A + board->calc_B.sword_B) > 0 )||
               (board->calc_A.sword_A > 0 && board->calc_B.sword_B > 0 &&
                (Sword)(board->calc_A.sword_A + board->calc_B.sword_B) < 0)) {
                   board->vf = 1;
               }
            break;
        case _AND:
            board->vf = 0;
            break;
        case _OR:
            board->vf = 0;
            break;
        case _EOR:
            board->vf = 0;
            break;
        default:
            break;
    }
}

void setNF(Cpub *board, int flag){
    board->nf = (flag < 0 ? 1 : 0);
}

void setZF(Cpub *board, int flag){
    board->zf = (flag == 0 ? 1 : 0);
}

int blanchCondition(Cpub *board, Bit flag){
    int judge;
    
    switch (flag) {
        case A:
            judge = TRUE;
            break;
        case VF:
            judge = (board->vf ? TRUE : FALSE);
            break;
        case NZ:
            judge = (board->zf ? FALSE : TRUE);
            break;
        case Z:
            judge = (board->zf ? TRUE : FALSE);
            break;
        case ZP:
            judge = (board->nf ? FALSE : TRUE);
            break;
        case N:
            judge = (board->nf ? TRUE : FALSE);
            break;
        case P:
            judge = (board->vf | board->zf ? FALSE : TRUE);
            break;
        case ZN:
            judge = (board->vf | board->zf ? TRUE : FALSE);
            break;
        case NI:
            judge = (board->ibuf->flag ? FALSE : TRUE);
            break;
        case NO:
            judge = (board->obuf->flag ? TRUE : FALSE);
            break;
        case NC:
            judge = (board->cf ? FALSE : TRUE);
            break;
        case C:
            judge = (board->cf ? TRUE : FALSE);
            break;
        case GE:
            judge = (board->vf ^ board->nf ? FALSE : TRUE);
            break;
        case LT:
            judge = (board->vf ^ board->nf ? TRUE : FALSE);
            break;
        case GT:
            judge = ((board->vf ^ board->nf) | board->zf ? FALSE : TRUE);
            break;
        case LE:
            judge = ((board->vf ^ board->nf) | board->zf ? TRUE : FALSE);
            break;
        default:
            fprintf(stderr, "None flag selected!\n");
            judge = FALSE;
            break;
    }
    return flag;
}
int selectAResister(Cpub *board, int load_register){
    board->regA = (load_register ? &board->ix : &board->acc);
    board->calc_A.uword_A = (Uword)*board->regA;
    board->calc_A.sword_A = (Sword)*board->regA;
    return RUN_STEP;
}

int selectBResister(Cpub *board, int load_register){
    int is_continue = RUN_STEP;
    switch (load_register) {
        case ACC:
            printf("ACC load mode\n");
            board->regB = &board->acc;
            break;
        case IX:
            printf("IX load mode\n");
            board->regB = &board->ix;
            break;
        case IMMEDIATE:
            printf("IMMEDIATE load mode\n");
            board->ir[1] = board->mem[board->pc++];
            board->regB = &board->mem[board->ir[1] + DATA_MEMORY_FRONT];
            break;
        case P_ABSOLUTLY:
            printf("P_ABSOLUTLY load mode\n");
            board->ir[1] = board->mem[board->pc++];
            board->regB = &board->mem[board->ir[1] + DATA_MEMORY_FRONT];
            break;
        case D_ABSOLUTLY:
            printf("D_ABSOLUTLY load mode\n");
            board->ir[1] = board->mem[board->pc++];
            board->regB = &board->mem[board->ir[1] + DATA_MEMORY_FRONT];
            break;
        case P_INDIRECTION:
            printf("P_INDIRECTION load mode\n");
            board->ir[1] = board->mem[board->pc++];
            board->regB = &board->mem[board->ir[1] + board->ix + DATA_MEMORY_FRONT];
            break;
        case D_INDIRECTION:
            printf("D_INDIRECTION load mode\n");
            board->ir[1] = board->mem[board->pc++];
            board->regB = &board->mem[board->ir[1] + board->ix + DATA_MEMORY_FRONT];
            break;
        default:
            fprintf(stderr, "None resister selected!\n");
            is_continue = RUN_HALT;
            break;
    }
    if(is_continue){
        board->calc_B.uword_B = (Uword)*board->regB;
        board->calc_B.sword_B = (Sword)*board->regB;
    }
    
    return is_continue;
}

/*=============================================================================
 *   Simulation of a Single Instruction
 *===========================================================================*/
int step(Cpub *board)
{
    fetch(board);
    
    board->shift_mode    = board->ir[0] & 0x03; //get low 2bit
    board->register_mode = board->ir[0] & 0x08; //get 4th bit
    board->load_register = board->ir[0] & 0x07; //get low 3bit
    
    if((board->ir[0] & 0xf8) == _NOP){
        printf("NOP mode\n");
        NOP(board);
    } else if((board->ir[0] & 0xfc) == _HLT){
        printf("HLT mode\n");
        HLT(board);
    } else if((board->ir[0] & 0xf8) == _OUT){
        printf("OUT mode\n");
        OUT(board);
    } else if((board->ir[0] & 0xf8) == _IN){
        printf("IN mode\n");
        IN(board);
    } else if((board->ir[0] & 0xf8) == _RCF){
        printf("RCF mode\n");
        RCF(board);
    } else if((board->ir[0] & 0xf8) == _SCF){
        printf("SCF mode\n");
        SCF(board);
    } else if((board->ir[0] & 0xf0) == _ADD){
        printf("ADD mode\n");
        ADD(board);
    } else if((board->ir[0] & 0xf0) == _ADC){
        printf("ADC mode\n");
        ADC(board);
    } else if((board->ir[0] & 0xf0) == _LD){
        printf("LD mode\n");
        LD(board);
    }  else if((board->ir[0] & 0xf0) == _ST){
        printf("ST mode\n");
        ST(board);
    }  else if((board->ir[0] & 0xf0) == _SUB){
        printf("SUB mode\n");
        SUB(board);
    } else if((board->ir[0] & 0xf0) == _SBC){
        printf("SBC mode\n");
        SBC(board);
    } else if((board->ir[0] & 0xf0) == _CMP){
        printf("CMP mode\n");
        CMP(board);
    } else if((board->ir[0] & 0xf0) == _AND){
        printf("AND mode\n");
        AND(board);
    } else if((board->ir[0] & 0xf0) == _OR){
        printf("OR mode\n");
        OR(board);
    } else if((board->ir[0] & 0xf0) == _EOR){
        printf("EOR mode\n");
        EOR(board);
    } else if(((board->ir[0] & 0xf0) == _SSM) && ((board->ir[0] & 0x04) == 0)){
        printf("SSM mode\n");
        Ssm(board);
    } else if(((board->ir[0] & 0xf0) == _RSM) && ((board->ir[0] & 0x04) == 1)){
        printf("RSM mode\n");
        Rsm(board);
    } else if((board->ir[0] & 0xf0) == _BBC){
        printf("BBC mode\n");
        Bbc(board);
    } /*else if((board->ir[0] & 0xff) == _JAL){
       JAL(board);
       } else if((board->ir[0] & 0xff) == _JR){
       JR(board);
       } */else{
           printf("ir[0] : %d\n", board->ir[0]);
           fprintf(stderr, "None command error!\n");
       }
    return RUN_HALT;
}

int HLT(Cpub *board){
    return RUN_HALT;
}

int NOP(Cpub *board){
    return RUN_STEP;
}

int OUT(Cpub *board){
    board->obuf->buf = board->acc;
    return RUN_STEP;
}

int IN(Cpub *board){
    board->acc = board->ibuf->buf;
    return RUN_STEP;
}

int RCF(Cpub *board){
    board->cf = 0;
    return RUN_STEP;
}

int SCF(Cpub *board){
    board->cf = 1;
    return RUN_STEP;
}

int Bbc(Cpub *board){
    int condition = blanchCondition(board, board->ir[0] & 0x0f); //get low 4bit
    if(condition){
        board->pc = board->ir[1];
    }
    return RUN_STEP;
}

int Ssm(Cpub *board){
    selectAResister(board, board->register_mode);
    int msb = *board->regA & 0x80; //get most significant bit
    printf("msb : %d\n",msb);
    
    switch (board->shift_mode) {
        case RA:
            printf("RA mode\n");
            *board->regA = (int)pow(*board->regA, -msb);
            break;
        case LA:
            printf("LA mode\n");
            *board->regA = (int)pow(*board->regA, msb);
            break;
        case RL:
            printf("RL mode\n");
            *board->regA = *board->regA >> 1;
            break;
        case LL:
            printf("LL mode\n");
            *board->regA = *board->regA << 1;
            break;
        default:
            fprintf(stderr, "None shift mode!\n");
            break;
    }
    
    setCF(board, *board->regA);
    setNF(board, *board->regA);
    setVF(board, *board->regA);
    setZF(board, *board->regA);
    
    return RUN_STEP;
}

int Rsm(Cpub *board){
    selectAResister(board, board->register_mode);
    int msb = *board->regA & 0x80; //get most significant bit
    
    switch (board->shift_mode) {
        case RA:
            printf("RA mode\n");
            *board->regA = (int)pow(*board->regA, -msb);
            break;
        case LA:
            printf("LA mode\n");
            *board->regA = (int)pow(*board->regA, msb);
            break;
        case RL:
            printf("RL mode\n");
            *board->regA = *board->regA >> 1;
            break;
        case LL:
            printf("LL mode\n");
            *board->regA = *board->regA << 1;
            break;
        default:
            fprintf(stderr, "None shift mode!\n");
            break;
    }
    
    setCF(board, *board->regA);
    setNF(board, *board->regA);
    setVF(board, *board->regA);
    setZF(board, *board->regA);
    
    return RUN_STEP;
}

int LD(Cpub *board){ //多分ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);
    *board->regA = *board->regB;
    return RUN_STEP;
}

int ST(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);
    *board->regB = *board->regA;
    return RUN_STEP;
}

int ADD(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);
    *board->regA += *board->regB;
    setVF(board, _ADD);
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    return RUN_STEP;
}

int ADC(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);
    *board->regA = *board->regA + *board->regB + board->cf;
    setCF(board, _ADC);
    setVF(board, _ADC);
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    return RUN_STEP;
}

int SUB(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);    *board->regA -= *board->regB;
    setVF(board, _SUB);
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    return RUN_STEP;
}

int SBC(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    board->regA = (board->register_mode ? &board->ix : &board->acc);
    *board->regA = *board->regA - *board->regB - board->cf;
    setCF(board, _SBC);
    setVF(board, _SBC);
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    return RUN_STEP;
}

int CMP(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);
    *board->regA -= *board->regB;
    setVF(board, _CMP);
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    return RUN_STEP;
}

int AND(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);
    *board->regA &= *board->regB;
    setVF(board, _AND);
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    return RUN_STEP;
}

int OR(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);
    *board->regA |= *board->regB;
    setVF(board, _OR);
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    return RUN_STEP;
}

int EOR(Cpub *board){ //ok
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    selectAResister(board, board->register_mode);
    *board->regA ^= *board->regB;
    setVF(board, _EOR);
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    return RUN_STEP;
}

int JAL(Cpub *board);
int JR(Cpub *board);
