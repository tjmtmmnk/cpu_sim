/*
 *    Project-based Learning II (CPU)
 *
 *    Program:    instruction set simulator of the Educational CPU Board
 *    File Name:    main.c
 *    Descrioption:    main profram (command interpreter)
 */

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    "cpu_board.h"


void help(void);
void initializeBoard(Cpub *);
void cont(Cpub *, char *);
void display_regs(Cpub *);
void set_reg(Cpub *, char *, char *);
void display_mem(Cpub *, char *);
void display_mem_line(Cpub *, Addr);
void display_mem_all(Cpub *);
void set_mem(Cpub *, char *, char *);
void read_mem_file(Cpub *, char *);
void cmd_syntax_error(void);
void unknown_command(void);
int init_cpu(void);

#define    CL_BUFF_SIZE    160

/*=============================================================================
 *   CPU Board States
 *===========================================================================*/
Cpub    cpuboard[2];    /* CPU board state */


/*=============================================================================
 *   Command: Display a Help Menu
 *===========================================================================*/
void help(void)
{
    fprintf(stderr,"   i\t\t--- execute an instruction "
            "(one step execution)\n");
    fprintf(stderr,"   c [addr]\t--- continue(start) execution "
            "[to address(hex)]\n");
    fprintf(stderr,"   d\t\t--- display the contents of registers\n");
    fprintf(stderr,"   s reg data\t--- set data(hex) to the register\n"
            "\t\t\treg: pc,acc,ix,cf,vf,nf,zf,"
            "ibuf,if,obuf,of\n");
    fprintf(stderr,"   m [addr]\t--- dump memory or display data "
            "at memory address(hex)\n");
    fprintf(stderr,"   w addr data\t--- write data(hex) "
            "at memory address(hex)\n");
    fprintf(stderr,"   r file\t--- load a program into the main memory "
            "from the file\n");
    fprintf(stderr,"   t\t\t--- toggle current computer(context)\n");
    fprintf(stderr,"   h\t\t--- help (this menu)\n");
    fprintf(stderr,"   ?\t\t--- help (this menu)\n");
    fprintf(stderr,"   q\t\t--- quit\n");
}


/*=============================================================================
 *   Initialization for the System Organization
 *===========================================================================*/
int init_cpu(void)
{
    cpuboard[0].ibuf = &(cpuboard[1].obuf);
    cpuboard[1].ibuf = &(cpuboard[0].obuf);
    return 0;
}

void initializeBoard(Cpub *board){
    board->pc = PROGRAM_MEMORY_FRONT;
    for (int i=0; i<MEMORY_SIZE;i++) {
        board->mem[i] = 0x00;
    }
    board->acc = 0;
    board->ix = 0;
    board->calc_A.uword_A = 0;
    board->calc_A.sword_A = 0;
    board->calc_B.uword_B = 0;
    board->calc_B.sword_B = 0;
    board->cf = 0;
    board->vf = 0;
    board->nf = 0;
    board->zf = 0;
    board->immediate_reg = 0;
//    board->ibuf->flag = 0;
//    board->obuf->flag = 0;
}

/*=============================================================================
 *   Main Routine: Command Interpreter
 *===========================================================================*/
int main()
{
    char    cmdline[CL_BUFF_SIZE];    /* command line buffer */
    char    cmd[CL_BUFF_SIZE], arg1[CL_BUFF_SIZE], arg2[CL_BUFF_SIZE], dummy[CL_BUFF_SIZE];
    Cpub    *board;            /* current CPU board state */
    int    cpub_id;        /* current CPU board ID */
    int    n;
    
    cpub_id = init_cpu();
    board = &(cpuboard[cpub_id]);
    initializeBoard(board);
    /*
     *   Interpret commands
     */
        while(1) {
            /*
             *   Prompt
             */
            fprintf(stderr,"CPU%d,PC=0x%x> ",cpub_id,board->pc);
            /* fflush(stderr); */

            /*
             *   Input a command line
             */
            if( fgets(cmdline,CL_BUFF_SIZE,stdin) == NULL ){
                return 0; /* exiting */
            }
            if( (n = sscanf(cmdline,"%s%s%s%s",cmd,arg1,arg2,dummy)) <= 0 ){
                continue; /* empty input, so retry */
            }

            /*
             *   Interpet a command
             */
            if( cmd[1] != '\0' ) {
                unknown_command();
                continue;
            }
            switch( cmd[0] ) {
                case 'i':
                    if( step(board) == RUN_HALT ) {
//                        fprintf(stderr,"Program Halted.\n");
                    }
                    break;
                case 'c':
                    switch( n ) {
                        case 1:    cont(board,NULL); break;
                        case 2:    cont(board,arg1); break;
                        default:    goto syntaxerr;
                    }
                    break;
                case 'd':
                    if( n != 1 ) goto syntaxerr;
                    display_regs(board);
                    break;
                case 's':
                    if( n != 3 ) goto syntaxerr;
                    set_reg(board,arg1,arg2);
                    break;
                case 'm':
                    switch( n ) {
                        case 1:    display_mem_all(board); break;
                        case 2:    display_mem(board,arg1); break;
                        default:    goto syntaxerr;
                    }
                    break;
                case 'w':
                    if( n != 3 ) goto syntaxerr;
                    set_mem(board,arg1,arg2);
                    break;
                case 'r':
                    if( n != 2 ) goto syntaxerr;
                    read_mem_file(board,arg1);
                    break;
                case 't':
                    cpub_id ^= 1;
                    board = &(cpuboard[cpub_id]);
                    break;
                case 'h':
                case '?':
                    help();
                    break;
                case 'q':
                    if( n != 1 )
                        goto syntaxerr;
                    else
                        return 0; /* exiting */
                    break; /* never reach here */
                default:
                    unknown_command();
                    break;
                syntaxerr:
                    cmd_syntax_error();
                    break;
            }
        }
    //    /* never reach here */
}


/*=============================================================================
 *   Command: Continue(Start) Execution
 *===========================================================================*/
void cont(Cpub *board, char *straddr)
{
#define    MAX_EXEC_COUNT    500
    int    addr;
    Addr    breakp;
    int    count;

    /*
     *   Check and set a break-point address
     */
    if( straddr == NULL )
        breakp = 0xffff;    /* impossible address (on purpose) */
    else {
        sscanf(straddr,"%x",&addr);
        if( addr < 0 || addr >= IMEMORY_SIZE ) {
            fprintf(stderr,"Invalid address: 0x%x\n",addr);
            return;
        }
        breakp = addr;
    }

    /*
     *   Execute a program
     */
    count = 1;
    do {
        if( step(board) == RUN_HALT ) {
            fprintf(stderr,"Program Halted.\n");
            return;
        }
        if( count++ > MAX_EXEC_COUNT ) {
            fprintf(stderr,"Too Many Instructions are Executed.\n");
            return;
        }
    } while( board->pc != breakp );
}


/*=============================================================================
 *   Command: Display Registers and Flags
 *===========================================================================*/
#define    DispRegVec(R)    (Uword)(R),(Sword)(R),(Uword)(R)

void display_regs(Cpub *board)
{
    fprintf(stderr,"\tacc=0x%02x(%d,%u)    ix=0x%02x(%d,%u)"
            "   cf=%d vf=%x nf=%x zf=%x\n",
            DispRegVec(board->acc),DispRegVec(board->ix),
            board->cf,board->vf,board->nf,board->zf);
    fprintf(stderr,"\tibuf=%x:0x%02x(%d,%u)\n",board->ibuf->flag,DispRegVec(board->ibuf->buf));
    fprintf(stderr,"\tobuf=%x:0x%02x(%d,%u)\n",board->obuf.flag,DispRegVec(board->obuf.buf));
}


/*=============================================================================
 *   Command: Set a Register/Flag
 *===========================================================================*/
void set_reg(Cpub *board, char *regname, char *strval)
{
    unsigned int    value, max;
    unsigned char    *reg;
    
    /*
     *   Check the register/flag name
     */
    if( !strcmp(regname,"pc") )     reg = &(board->pc), max = 0xff;
    else
        if( !strcmp(regname,"acc") )    reg = &(board->acc), max = 0xff;
        else
            if( !strcmp(regname,"ix") )    reg = &(board->ix), max = 0xff;
            else
                if( !strcmp(regname,"cf") )    reg = &(board->cf), max = 1;
                else
                    if( !strcmp(regname,"vf") )    reg = &(board->vf), max = 1;
                    else
                        if( !strcmp(regname,"nf") )    reg = &(board->nf), max = 1;
                        else
                            if( !strcmp(regname,"zf") )    reg = &(board->zf), max = 1;
                            else
                                if( !strcmp(regname,"ibuf") )    reg = &(board->ibuf->buf), max = 0xff,
                                    board->ibuf->flag = 1;
                                else
                                    if( !strcmp(regname,"if") )    reg = &(board->ibuf->flag), max = 1;
                                    else
                                        if( !strcmp(regname,"obuf") )    reg = &(board->obuf.buf), max = 0xff,
                                            board->obuf.flag = 1;
                                        else
                                            if( !strcmp(regname,"of") )    reg = &(board->obuf.flag), max = 1;
                                            else {
                                                fprintf(stderr,"Unknown register name: %s\n",regname);
                                                return;
                                            }
    
    /*
     *   Write to the register/flag
     */
    sscanf(strval,"%x",&value);
    if( value > max )
        fprintf(stderr,"Invalid value (out of range): 0x%x\n",value);
    else
        *reg = value;
    
    /*
     *   For confirmation
     */
    display_regs(board);
}


/*=============================================================================
 *   Command: Display the Contents of the Main Memory
 *===========================================================================*/
#define    MemLineBase(A)    ((A) & ~0xf)


void display_mem(Cpub *board, char *straddr)
{
    unsigned int    addr;
    
    sscanf(straddr,"%x",&addr);
    if( addr > MEMORY_SIZE ) {
        fprintf(stderr,"Invalid address (out of range): 0x%x\n",addr);
        return;
    }
    
    display_mem_line(board,(Addr)MemLineBase(addr));
}


void display_mem_line(Cpub *board, Addr addr)
{
    int    i, j;
    
    for( j = 0 ; j < 2 ; j++ ) {
        fprintf(stderr,"    | %03x: ",addr);
        for( i = 0 ; i < 8 ; i++ ) {
            fprintf(stderr," %02x",(Uword)board->mem[addr++]);
        }
    }
    fprintf(stderr,"\n");
}


void display_mem_all(Cpub *board)
{
    Addr    addr;
    
    for( addr = 0 ; addr < MEMORY_SIZE ; addr += 16 )
        display_mem_line(board,addr);
}


/*=============================================================================
 *   Command: Write a Word to a Memory Location
 *===========================================================================*/
void set_mem(Cpub *board, char *straddr, char *strval)
{
    unsigned int    addr, value;
    
    sscanf(straddr,"%x",&addr);
    if( addr > MEMORY_SIZE ) {
        fprintf(stderr,"Invalid address (out of range): 0x%x\n",addr);
        return;
    }
    
    sscanf(strval,"%x",&value);
    if( value > 0xff ) {
        fprintf(stderr,"Invalid value (out of range): 0x%x\n",value);
        return;
    }
    
    board->mem[addr] = value;
    display_mem_line(board,(Addr)MemLineBase(addr));
}


/*=============================================================================
 *   Command: Read a Program File
 *===========================================================================*/
void read_mem_file(Cpub *board, char *file)
{
#define    TOKENSIZE    160
    FILE        *fp;
    unsigned int    addr, word;
    Addr        area;
    char        token[TOKENSIZE];
    
    if( (fp = fopen(file,"r")) == NULL ) {
        fprintf(stderr,"Unable to open %s\n",file);
        return;
    }
    
    addr = 0;    /* default initial address */
    while( fscanf(fp,"%s",token) == 1 ) {
        if( token[0] == '.' ) {        /* directive */
            /*
             *   Check the directive type
             */
            if( !strcmp(token+1,"text") ) {
                area = 0x000; //write program area
            } else
                if( !strcmp(token+1,"data") ) {
                    area = 0x100; //write data area
                } else {
                    fprintf(stderr,"Unknown directive: %s\n",token);                goto error;
                }
            
            /*
             *   Change the current address
             */
            fscanf(fp,"%x",&addr);
            if( addr > 0xff ) {
                fprintf(stderr,"Invalid address: %s %x\n",
                        token,addr);
                goto error;
            }
            addr |= area;
        } else {            /* instruction word or data */
            sscanf(token,"%x",&word);
            if( word > 0xff ) {
                fprintf(stderr,"Invalid value at addr=0x%03x: "
                        "0x%x\n",addr,word);
                goto error;
            }
            board->mem[addr++] = word;
        }
    }
    board->data_size = addr;
    
error:
    fclose(fp);
}

/*=============================================================================
 *   Error Handling
 *===========================================================================*/
void cmd_syntax_error(void)
{
    fprintf(stderr,"Command syntax error. Type \'h\' for help.\n");
}

void  unknown_command(void)
{
    fprintf(stderr,"Unknown command. Type \'h\' for help.\n");
}


