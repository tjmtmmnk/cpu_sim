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
}

void setCF(Cpub *board, int flag){
    if((unsigned int)flag > 255 || ((signed int)flag > 127 || (signed int)flag < -128)){
        board->cf = 1;
    } else{
        board->cf = 0;
    }
}

void setVF(Cpub *board, int flag){
    if((unsigned int)flag > 127 || ((signed int)flag < -128) || (signed int)flag > 127){
        board->vf = 1;
    } else{
        board->vf = 0;
    }
}

void setNF(Cpub *board, int flag){
    board->vf = (flag < 0 ? 1 : 0);
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

int selectResister(Cpub *board, int load_register){
    int is_continue = RUN_STEP;
    switch (load_register) {
        case ACC:
            board->reg = board->acc;
            break;
        case IX:
            board->reg = board->ix;
            break;
        case IMMEDIATE:
            board->ir[1] = board->mem[board->pc++];
            board->reg = board->ir[1];
            break;
        case P_ABSOLUTLY:
            board->ir[1] = board->mem[board->pc++];
            board->reg = board->ir[1];
            break;
        case D_ABSOLUTLY:
            board->ir[1] = board->mem[board->pc++];
            board->reg = board->ir[1];
            break;
        case P_INDIRECTION:
            board->ir[1] = board->mem[board->pc++];
            board->reg = board->ir[1] + board->ix;
            break;
        case D_INDIRECTION:
            board->ir[1] = board->mem[board->pc++];
            board->reg = board->ir[1] + board->ix;
            break;
        default:
            fprintf(stderr, "None resister selected!\n");
            is_continue = RUN_HALT;
            break;
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
        NOP(board);
    } else if((board->ir[0] & 0xfc) == _HLT){
        HLT(board);
    } else if((board->ir[0] & 0xf8) == _OUT){
        OUT(board);
    } else if((board->ir[0] & 0xf8) == _IN){
        IN(board);
    } else if((board->ir[0] & 0xf8) == _RCF){
        RCF(board);
    } else if((board->ir[0] & 0xf8) == _SCF){
        SCF(board);
    } else if((board->ir[0] & 0xf) == _ADD){
        ADD(board);
    } else if((board->ir[0] & 0xf) == _ADC){
        ADC(board);
    } else if((board->ir[0] & 0xf) == _SUB){
        SUB(board);
    } else if((board->ir[0] & 0xf) == _SBC){
        SBC(board);
    } else if((board->ir[0] & 0xf) == _CMP){
        CMP(board);
    } else if((board->ir[0] & 0xf) == _AND){
        AND(board);
    } else if((board->ir[0] & 0xf) == _OR){
        OR(board);
    } else if((board->ir[0] & 0xf) == _EOR){
        EOR(board);
    } else if((board->ir[0] & 0xf) == _SSM){
        Ssm(board);
    } else if((board->ir[0] & 0xf) == _RSM){
        Rsm(board);
    } else if((board->ir[0] & 0xf) == _BBC){
        Bbc(board);
    } /*else if((board->ir[0] & 0xff) == _JAL){
        JAL(board);
    } else if((board->ir[0] & 0xff) == _JR){
        JR(board);
    } */else{
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
    //    int shift_mode = board->ir[0] & 0x03; //get low 2bit
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    int msb = *p_register & 0x80; //get most significant bit
    
    switch (board->shift_mode) {
        case RA:
            *p_register = (int)pow(*p_register, -msb);
            break;
        case LA:
            *p_register = (int)pow(*p_register, msb);
            break;
        case RL:
            *p_register >> msb;
            break;
        case LL:
            *p_register << msb;
            break;
        default:
            fprintf(stderr, "None shift mode!\n");
            break;
    }
    board->nf = (*p_register < 0  ? 1 : 0);
    board->zf = (*p_register == 0 ? 1 : 0);
    if(((msb == 1) && (*p_register < 0)) || ((msb == 0) && (*p_register > 0xff))){
        board->vf = 1;
    } else{
        board->vf = 0;
    }
    
    return RUN_STEP;
}

int Rsm(Cpub *board){
    //    int shift_mode = board->ir[0] & 0x03; //get low 2bit
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    int msb = *p_register & 0x80; //get most significant bit
    
    switch (board->shift_mode) {
        case RA:
            *p_register = (int)pow(*p_register, -msb);
            break;
        case LA:
            *p_register = (int)pow(*p_register, msb);
            break;
        case RL:
            *p_register >> msb;
            break;
        case LL:
            *p_register << msb;
            break;
        default:
            fprintf(stderr, "None shift mode!\n");
            break;
    }
    board->nf = (*p_register < 0  ? 1 : 0);
    board->zf = (*p_register == 0 ? 1 : 0);
    setCF(board, *p_register);
    setVF(board, *p_register);
    return RUN_STEP;
}

int LD(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register = board->reg;
    return RUN_STEP;
}

int ST(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    board->reg = *p_register;
    return RUN_STEP;
}

int ADD(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register += board->reg;
    return RUN_STEP;
}

int ADC(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register = *p_register + board->reg + board->cf;
    return RUN_STEP;
}

int SUB(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register -= board->reg;
    return RUN_STEP;
}

int SBC(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register = *p_register - board->reg - board->cf;
    return RUN_STEP;
}

int CMP(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register -= board->reg;
    setVF(board, *p_register);
    setNF(board, *p_register);
    setZF(board, *p_register);
    return RUN_STEP;
}

int AND(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register &= board->reg;
    return RUN_STEP;
}

int OR(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register |= board->reg;
    return RUN_STEP;
}

int EOR(Cpub *board){
    //    int register_mode = board->ir[0] & 0x08; //get 4th bit
    //    int load_register = board->ir[0] & 0x07; //get low 3bit
    if(!selectResister(board, board->load_register)){return RUN_HALT;}
    Uword *p_register;
    p_register = (board->register_mode ? &board->ix : &board->acc);
    *p_register ^= board->reg;
    return RUN_STEP;
}

int JAL(Cpub *board);
int JR(Cpub *board);
