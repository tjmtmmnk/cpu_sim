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
    printf("pc : %d\n", board->pc);
    board->ir[0] = board->mem[board->pc++];
}

void setCF(Cpub *board, int mode){ //符号無しの判定
    switch (mode) {
        case _RCF:
            board->cf = 0;
            break;
        case _SCF:
            board->cf = 1;
            break;
        case _SRA:
        case _SRL:
        case _RRA:
        case _RRL:
            board->cf = board->calc_A.sword_A & 1;
            break;
        case _SLA:
        case _SLL:
        case _RLA:
        case _RLL:
            board->cf = (board->calc_A.sword_A>>7) & 1;
            break;
        case _ADC:
        case _SBC:
            if((Uword)(board->calc_A.uword_A + board->calc_B.uword_B) < board->calc_A.uword_A){
                board->cf = 1;
            } else{
                board->cf = 0;
            }
            break;
        default:
            break;
    }
}

void setVF(Cpub *board, int mode){ //符号付きの判定
    switch (mode) {
        case _ADD:
        case _ADC:
        case _SUB:
        case _SBC:
        case _CMP:
            if((board->calc_A.sword_A <= 0 && board->calc_B.sword_B <= 0 &&
                (Sword)(board->calc_A.sword_A + board->calc_B.sword_B) >= 0 )||
               (board->calc_A.sword_A >= 0 && board->calc_B.sword_B >= 0 &&
                (Sword)(board->calc_A.sword_A + board->calc_B.sword_B) <= 0)) {
                   board->vf = 1;
               }else{
                   board->vf = 0;
               }
            if(board->calc_A.sword_A == 0 && board->calc_B.sword_B == 0 &&
               (Sword)(board->calc_A.sword_A + board->calc_B.sword_B) == 0){
                board->vf = 0;
            }
            break;
        case _SLA:
        case _RLA:
            board->vf = (board->calc_A.sword_A>>7) & 1;
            break;
        case _AND:
        case _OR:
        case _EOR:
        case _SRA:
        case _SRL:
        case _SLL:
        case _RRA:
        case _RRL:
        case _RLL:
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
            printf("A flag\n");
            judge = TRUE;
            break;
        case VF:
            printf("VF flag\n");
            judge = (board->vf ? TRUE : FALSE);
            break;
        case NZ:
            printf("NZ flag\n");
            judge = (board->zf ? FALSE : TRUE);
            break;
        case Z:
            printf("Z flag\n");
            judge = (board->zf ? TRUE : FALSE);
            break;
        case ZP:
            printf("ZP flag\n");
            judge = (board->nf ? FALSE : TRUE);
            break;
        case N:
            printf("N flag\n");
            judge = (board->nf ? TRUE : FALSE);
            break;
        case P:
            printf("P flag\n");
            judge = (board->vf | board->zf ? FALSE : TRUE);
            break;
        case ZN:
            printf("ZN flag\n");
            judge = (board->vf | board->zf ? TRUE : FALSE);
            break;
        case NI:
            printf("NI flag\n");
            judge = (board->ibuf->flag ? FALSE : TRUE);
            break;
        case NO:
            printf("NO flag\n");
            judge = (board->obuf->flag ? TRUE : FALSE);
            break;
        case NC:
            printf("NC flag\n");
            judge = (board->cf ? FALSE : TRUE);
            break;
        case C:
            printf("C flag\n");
            judge = (board->cf ? TRUE : FALSE);
            break;
        case GE:
            printf("GE flag\n");
            judge = (board->vf ^ board->nf ? FALSE : TRUE);
            break;
        case LT:
            printf("LT flag\n");
            judge = (board->vf ^ board->nf ? TRUE : FALSE);
            break;
        case GT:
            printf("GT flag\n");
            judge = ((board->vf ^ board->nf) | board->zf ? FALSE : TRUE);
            break;
        case LE:
            printf("LE flag\n");
            judge = ((board->vf ^ board->nf) | board->zf ? TRUE : FALSE);
            break;
        default:
            fprintf(stderr, "None flag selected!\n");
            judge = FALSE;
            break;
    }
    return judge;
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
            if(board->ir[0] == _JAL){
                board->regB = &board->mem[board->pc++];
                break;
            }
            printf("IMMEDIATE load mode\n");
            board->ir[1] = board->mem[board->pc++];
            board->immediate_reg = board->ir[1];
            board->regB = &board->immediate_reg;
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
    board->register_mode = (board->ir[0]>>3) & 1; //get 4th bit
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
    } else if(((board->ir[0] & 0xf0) == _RSM) && ((board->ir[0]>>2 & 1) == 0)){
        printf("SSM mode\n");
        Ssm(board);
    } else if(((board->ir[0] & 0xf0) == _RSM) && ((board->ir[0]>>2 & 1) == 1)){
        printf("RSM mode\n");
        Rsm(board);
    } else if((board->ir[0] & 0xf0) == _BBC){
        printf("BBC mode\n");
        Bbc(board);
    } else if((board->ir[0] & 0xff) == _JAL){
        printf("JAL mode\n");
        JAL(board);
    } else if((board->ir[0] & 0xff) == _JR){
        printf("JR mode\n");
        JR(board);
    } else{
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
        board->ir[1] = board->mem[board->pc];
        board->pc = board->ir[1];
        printf("next pc : %d\n",board->pc);
    } else{
        board->pc++;
    }
    return RUN_STEP;
}

int Ssm(Cpub *board){//多分ok
    selectAResister(board, board->register_mode);
    
    switch (board->shift_mode) {
        case RA:
            printf("RA mode\n");
            *board->regA = *board->regA >> 1;
            setCF(board, _SRA);
            setVF(board, _SRA);
            break;
        case LA:
            printf("LA mode\n");
            *board->regA = *board->regA << 1;
            setCF(board, _SLA);
            setVF(board, _SLA);
            break;
        case RL:
            printf("RL mode\n");
            *board->regA = *board->regA >> 1;
            setCF(board, _SRL);
            setVF(board, _SRL);
            break;
        case LL:
            printf("LL mode\n");
            *board->regA = *board->regA << 1;
            setCF(board, _SLL);
            setVF(board, _SLL);
            break;
        default:
            fprintf(stderr, "None shift mode!\n");
            break;
    }
    
    setNF(board, *board->regA);
    setZF(board, *board->regA);
    
    return RUN_STEP;
}

int Rsm(Cpub *board){ //多分ok
    selectAResister(board, board->register_mode);
    
    switch (board->shift_mode) {
        case RA:
            printf("RA mode\n");
            *board->regA = *board->regA >> 1;
            setCF(board, _RRA);
            setVF(board, _RRA);
            *board->regA |= ((*board->regA>>7) | board->cf) << 7;
            break;
        case LA:
            printf("LA mode\n");
            *board->regA = *board->regA << 1;
            setCF(board, _RLA);
            setVF(board, _RLA);
            *board->regA |= ((*board->regA&0x01) | board->cf);
            break;
        case RL:
            printf("RL mode\n");
            Bit lsb = *board->regA & 0x01;
            *board->regA = *board->regA >> 1;
            setCF(board, _RRL);
            setVF(board, _RRL);
            *board->regA |= ((*board->regA>>7) | lsb) << 7;
            break;
        case LL:
            printf("LL mode\n");
            Bit msb = (*board->regA>>7) & 1;
            *board->regA = *board->regA << 1;
            setCF(board, _RLL);
            setVF(board, _RLL);
            *board->regA |= ((*board->regA&0x01) | msb);
            break;
        default:
            fprintf(stderr, "None shift mode!\n");
            break;
    }
    
    setNF(board, *board->regA);
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
    selectAResister(board, board->register_mode);
    *board->regA -= *board->regB;
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

int JAL(Cpub *board){
    if(!selectBResister(board, board->load_register)){return RUN_HALT;}
    board->acc = board->pc;
    board->pc = *board->regB;
    step(board);
    return RUN_STEP;
}

int JR(Cpub *board){
    board->pc = board->acc;
    return RUN_STEP;
}
