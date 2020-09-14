%{
/*
    module  : my_parse.y
    version : 1.1
    date    : 09/12/20
*/
#include <stdio.h>
#include "my_parse.h"

extern unsigned char section;
extern FILE *textfp, *datafp, *bssfp;

#define TEXT	1
#define DATA	2

#define NAME	0
#define REG	1
#define MEM	2
#define SCALE	3

#define MAXSTR	100

#include "khash.h"

KHASH_MAP_INIT_STR(Used, char)

static khash_t(Used) *usedTab;	/* table with used symbols */

int yylex(void);
void yyerror(char *str);
char *print_num(long num);
char *print_reg(long num);
char *print_mem(struct mem mem, int size);
int copy_data(void);
char *print_str(char *str);
%}

%token <str> Name
%token <num> Number

%token PTR
%token <num> BYTE
%token <num> WORD
%token <num> DWORD
%token <num> QWORD

%token OP_ADD
%token OP_AND
%token OP_BT
%token OP_CALL
%token OP_CDQE
%token OP_CMOVE
%token OP_CMOVLE
%token OP_CMOVNE
%token OP_CMOVNS
%token OP_CMOVS
%token OP_CMP
%token OP_CWDE
%token OP_CQO
%token OP_DEC
%token OP_INC
%token OP_IDIV
%token OP_IMUL
%token OP_JA
%token OP_JAE
%token OP_JB
%token OP_JBE
%token OP_JC
%token OP_JE
%token OP_JG
%token OP_JGE
%token OP_JL
%token OP_JLE
%token OP_JMP
%token OP_JNB
%token OP_JNC
%token OP_JNE
%token OP_JNS
%token OP_JS
%token OP_LEA
%token OP_MOV
%token OP_MOVABS
%token OP_MOVSX
%token OP_MOVZX
%token OP_MUL
%token OP_DIV
%token OP_NEG
%token OP_NOP
%token OP_NOT
%token OP_OR
%token OP_POP
%token OP_PUSH
%token OP_REPZ
%token OP_CMPSB
%token OP_RET
%token OP_SAL
%token OP_SAR
%token OP_SBB
%token OP_SETA
%token OP_SETBE
%token OP_SETE
%token OP_SETG
%token OP_SETGE
%token OP_SETL
%token OP_SETLE
%token OP_SETNE
%token OP_SHR
%token OP_SUB
%token OP_TEST
%token OP_XOR

%token <num> REG_AH
%token <num> REG_AL
%token <num> REG_AX
%token <num> REG_BH
%token <num> REG_BL
%token <num> REG_BP
%token <num> REG_BPL
%token <num> REG_BX
%token <num> REG_CH
%token <num> REG_CL
%token <num> REG_CX
%token <num> REG_DH
%token <num> REG_DI
%token <num> REG_DIL
%token <num> REG_DL
%token <num> REG_DX
%token <num> REG_EAX
%token <num> REG_EBP
%token <num> REG_EBX
%token <num> REG_ECX
%token <num> REG_EDI
%token <num> REG_EDX
%token <num> REG_ESI
%token <num> REG_ESP
%token <num> REG_R10
%token <num> REG_R10B
%token <num> REG_R10D
%token <num> REG_R10W
%token <num> REG_R11
%token <num> REG_R11B
%token <num> REG_R11D
%token <num> REG_R11W
%token <num> REG_R12
%token <num> REG_R12B
%token <num> REG_R12D
%token <num> REG_R12W
%token <num> REG_R13
%token <num> REG_R13B
%token <num> REG_R13D
%token <num> REG_R13W
%token <num> REG_R14
%token <num> REG_R14B
%token <num> REG_R14D
%token <num> REG_R14W
%token <num> REG_R15
%token <num> REG_R15B
%token <num> REG_R15D
%token <num> REG_R15W
%token <num> REG_R8
%token <num> REG_R8B
%token <num> REG_R8D
%token <num> REG_R8W
%token <num> REG_R9
%token <num> REG_R9B
%token <num> REG_R9D
%token <num> REG_R9W
%token <num> REG_RAX
%token <num> REG_RBP
%token <num> REG_RBX
%token <num> REG_RCX
%token <num> REG_RDI
%token <num> REG_RDX
%token <num> REG_RIP
%token <num> REG_RSI
%token <num> REG_RSP
%token <num> REG_SI
%token <num> REG_SIL
%token <num> REG_SP
%token <num> REG_SPL
%token <num> REG_EIP
%token <num> REG_IP

%type <num> register
%type <num> opt_offset
%type <num> opt_number
%type <num> opt_scale
%type <num> size

%type <jmp> jmp_name
%type <idx> opt_index
%type <mem> memory
%type <mem> lea_memory

%union {
    long num;
    char *str;
    struct idx {
	long reg;
	long scale;
    } idx;
    struct mem {
	long size;
	long tag;
	union {
	    struct {
		char *name;
		long offset;
	    };
	    struct {
		long reg;
		struct idx idx;
		long num;
	    };
	};
    } mem;
    struct jmp {
	int tag;
	union {
	    char *name;
	    long reg;
	    struct mem mem;
	};
    } jmp;
};

%%

program	: /* empty */
	  { fprintf(textfp, "\tbits 64\n\tdefault rel\n\tsection .text\n");
	    usedTab = kh_init(Used); }
	| program line
	;

line	: label
	| instr
	;

label	: Name ':'
	  { int unused; khiter_t key = kh_put(Used, usedTab, $<str>1, &unused);
	    kh_value(usedTab, key) = 1; if (section == TEXT) { if ($<str>1[0]
	    != '.') { copy_data(); fprintf(textfp, "\tglobal %s\n", $<str>1); }
	    fprintf(textfp, "%s:\n", print_str($<str>1)); } else {
	    if ($<str>1[0] != '.') fprintf(datafp, "\tglobal %s\n", $<str>1);
	    fprintf(datafp, "%s:\n", print_str($<str>1)); } }
	;

instr	: OP_MUL register
	  { fprintf(textfp, "\tmul\t%s\n", print_reg($<num>2)); }
	| OP_INC register
	  { fprintf(textfp, "\tinc\t%s\n", print_reg($<num>2)); }
	| OP_INC memory
	  { fprintf(textfp, "\tinc\t%s\n", print_mem($<mem>2, 1)); }
	| OP_DEC register
	  { fprintf(textfp, "\tdec\t%s\n", print_reg($<num>2)); }
	| OP_DEC memory
	  { fprintf(textfp, "\tdec\t%s\n", print_mem($<mem>2, 1)); }
	| OP_ADD register ',' register
	  { fprintf(textfp, "\tadd\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_ADD register ',' memory
	  { fprintf(textfp, "\tadd\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 0)); }
	| OP_ADD register ',' Number
	  { fprintf(textfp, "\tadd\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_ADD memory ',' register
	  { fprintf(textfp, "\tadd\t%s, %s\n", print_mem($<mem>2, 0),
	    print_reg($<num>4)); }
	| OP_ADD memory ',' Number
	  { fprintf(textfp, "\tadd\t%s, %s\n", print_mem($<mem>2, 1),
	    print_num($<num>4)); }
	| OP_AND register ',' register
	  { fprintf(textfp, "\tand\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_AND register ',' Number
	  { fprintf(textfp, "\tand\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_CMP register ',' register
	  { fprintf(textfp, "\tcmp\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_CMP register ',' memory
	  { fprintf(textfp, "\tcmp\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 0)); }
	| OP_CMP register ',' Number
	  { fprintf(textfp, "\tcmp\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_CMP memory ',' register
	  { fprintf(textfp, "\tcmp\t%s, %s\n", print_mem($<mem>2, 0),
	    print_reg($<num>4)); }
	| OP_CMP memory ',' Number
	  { fprintf(textfp, "\tcmp\t%s, %s\n", print_mem($<mem>2, 1),
	    print_num($<num>4)); }
	| OP_IMUL register ',' register
	  { fprintf(textfp, "\timul\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_IMUL register ',' memory
	  { fprintf(textfp, "\timul\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 0)); }
	| OP_IMUL register ',' register ',' Number
	  { fprintf(textfp, "\timul\t%s, %s, %s\n", print_reg($<num>2),
	    print_reg($<num>4), print_num($<num>6)); }
	| OP_IMUL register ',' memory ',' Number
	  { fprintf(textfp, "\timul\t%s, %s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 0), print_num($<num>6)); }
	| OP_IMUL memory ',' Number
	  { fprintf(textfp, "\timul\t%s, %s\n", print_mem($<mem>2, 1),
	    print_num($<num>4)); }
	| OP_MOV register ',' register
	  { fprintf(textfp, "\tmov\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_MOV register ',' memory
	  { fprintf(textfp, "\tmov\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 0)); }
	| OP_MOV register ',' Number
	  { fprintf(textfp, "\tmov\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_MOV memory ',' register
	  { fprintf(textfp, "\tmov\t%s, %s\n", print_mem($<mem>2, 0),
	    print_reg($<num>4)); }
	| OP_MOV memory ',' Number
	  { fprintf(textfp, "\tmov\t%s, %s\n", print_mem($<mem>2, 1),
	    print_num($<num>4)); }
	| OP_MOVABS register ',' Number
	  { fprintf(textfp, "\tmov\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_MOVSX register ',' register
	  { fprintf(textfp, "\tmovsx\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_MOVSX register ',' memory
	  { fprintf(textfp, "\tmovsx\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 1)); }
	| OP_MOVZX register ',' register
	  { fprintf(textfp, "\tmovzx\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_MOVZX register ',' memory
	  { fprintf(textfp, "\tmovzx\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 1)); }
	| OP_OR register ',' register
	  { fprintf(textfp, "\tor\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_OR register ',' Number
	  { fprintf(textfp, "\tor\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_SUB register ',' register
	  { fprintf(textfp, "\tsub\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_SUB register ',' memory
	  { fprintf(textfp, "\tsub\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 0)); }
	| OP_SUB register ',' Number
	  { fprintf(textfp, "\tsub\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_SUB memory ',' register
	  { fprintf(textfp, "\tsub\t%s, %s\n", print_mem($<mem>2, 1),
	    print_reg($<num>4)); }
	| OP_SUB memory ',' Number
	  { fprintf(textfp, "\tsub\t%s, %s\n", print_mem($<mem>2, 1),
	    print_num($<num>4)); }
	| OP_BT register ',' register
	  { fprintf(textfp, "\tbt\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_CMOVE register ',' memory
	  { fprintf(textfp, "\tcmove\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 0)); }
	| OP_CMOVLE register ',' register
	  { fprintf(textfp, "\tcmovle\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_CMOVNE register ',' register
	  { fprintf(textfp, "\tcmovne\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_CMOVNS register ',' register
	  { fprintf(textfp, "\tcmovns\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_CMOVS register ',' register
	  { fprintf(textfp, "\tcmovs\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_CALL jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tcall\t%s\n",
	    print_str($<jmp>2.name)); else if ($<jmp>2.tag == MEM)
	    fprintf(textfp, "\tcall\t%s\n", print_mem($<jmp>2.mem, 0)); else
	    fprintf(textfp, "\tcall\t%s\n", print_reg($<jmp>2.reg)); }
	| OP_JA jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tja\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tja\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JAE jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjae\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjae\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JB jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjb\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjb\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JBE jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjbe\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjbe\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JC jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjc\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjc\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JE jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tje\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tje\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JG jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjg\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjg\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JGE jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjge\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjge\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JL jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjl\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjl\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JLE jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjle\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjle\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JMP jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjmp\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjmp\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JNB jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjnb\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjnb\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JNC jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjnc\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjnc\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JNE jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjne\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjne\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JNS jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjns\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjns\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_JS jmp_name
	  { if ($<jmp>2.tag == NAME) fprintf(textfp, "\tjs\t%s\n",
	    print_str($<jmp>2.name)); else fprintf(textfp, "\tjs\t%s\n",
	    print_reg($<jmp>2.reg)); }
	| OP_CWDE
	  { fprintf(textfp, "\tcwde\n"); }
	| OP_CDQE
	  { fprintf(textfp, "\tcdqe\n"); }
	| OP_CQO
	  { fprintf(textfp, "\tcqo\n"); }
	| OP_REPZ OP_CMPSB
	  { fprintf(textfp, "\trepz cmpsb\n"); }
	| OP_RET
	  { fprintf(textfp, "\tret\n"); }
	| OP_NOP
	  { fprintf(textfp, "\tnop\n"); }
	| OP_IDIV register
	  { fprintf(textfp, "\tidiv\t%s\n", print_reg($<num>2)); }
	| OP_DIV register
	  { fprintf(textfp, "\tdiv\t%s\n", print_reg($<num>2)); }
	| OP_NEG register
	  { fprintf(textfp, "\tneg\t%s\n", print_reg($<num>2)); }
	| OP_NEG memory
	  { fprintf(textfp, "\tneg\t%s\n", print_mem($<mem>2, 1)); }
	| OP_NOT register
	  { fprintf(textfp, "\tnot\t%s\n", print_reg($<num>2)); }
	| OP_SAR register
	  { fprintf(textfp, "\tsar\t%s, 1\n", print_reg($<num>2)); }
	| OP_SAR register ',' Number
	  { fprintf(textfp, "\tsar\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_LEA register ',' lea_memory
	  { fprintf(textfp, "\tlea\t%s, %s\n", print_reg($<num>2),
	    print_mem($<mem>4, 0)); }
	| OP_SAL register ',' Number
	  { fprintf(textfp, "\tsal\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_POP register
	  { fprintf(textfp, "\tpop\t%s\n", print_reg($<num>2)); }
	| OP_SETA register
	  { fprintf(textfp, "\tseta\t%s\n", print_reg($<num>2)); }
	| OP_SETBE register
	  { fprintf(textfp, "\tsetbe\t%s\n", print_reg($<num>2)); }
	| OP_SETE register
	  { fprintf(textfp, "\tsete\t%s\n", print_reg($<num>2)); }
	| OP_SETG register
	  { fprintf(textfp, "\tsetg\t%s\n", print_reg($<num>2)); }
	| OP_SETGE register
	  { fprintf(textfp, "\tsetge\t%s\n", print_reg($<num>2)); }
	| OP_SETL register
	  { fprintf(textfp, "\tsetl\t%s\n", print_reg($<num>2)); }
	| OP_SETLE register
	  { fprintf(textfp, "\tsetle\t%s\n", print_reg($<num>2)); }
	| OP_SETNE register
	  { fprintf(textfp, "\tsetne\t%s\n", print_reg($<num>2)); }
	| OP_PUSH register
	  { fprintf(textfp, "\tpush\t%s\n", print_reg($<num>2)); }
	| OP_PUSH memory
	  { fprintf(textfp, "\tpush\t%s\n", print_mem($<mem>2, 1)); }
	| OP_PUSH Number
	  { fprintf(textfp, "\tpush\t%s\n", print_num($<num>2)); }
	| OP_SBB register ',' register
	  { fprintf(textfp, "\tsbb\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_XOR register ',' register
	  { fprintf(textfp, "\txor\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_SHR register ',' register
	  { fprintf(textfp, "\tshr\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_SHR register ',' Number
	  { fprintf(textfp, "\tshr\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_TEST register ',' register
	  { fprintf(textfp, "\ttest\t%s, %s\n", print_reg($<num>2),
	    print_reg($<num>4)); }
	| OP_TEST register ',' Number
	  { fprintf(textfp, "\ttest\t%s, %s\n", print_reg($<num>2),
	    print_num($<num>4)); }
	| OP_TEST memory ',' Number
	  { fprintf(textfp, "\ttest\t%s, %s\n", print_mem($<mem>2, 1),
	    print_num($<num>4)); }
	;

register
	: REG_AH
	| REG_AL
	| REG_AX
	| REG_BH
	| REG_BL
	| REG_BP
	| REG_BPL
	| REG_BX
	| REG_CH
	| REG_CL
	| REG_CX
	| REG_DH
	| REG_DI
	| REG_DIL
	| REG_DL
	| REG_DX
	| REG_EAX
	| REG_EBP
	| REG_EBX
	| REG_ECX
	| REG_EDI
	| REG_EDX
	| REG_ESI
	| REG_ESP
	| REG_R10
	| REG_R10B
	| REG_R10D
	| REG_R10W
	| REG_R11
	| REG_R11B
	| REG_R11D
	| REG_R11W
	| REG_R12
	| REG_R12B
	| REG_R12D
	| REG_R12W
	| REG_R13
	| REG_R13B
	| REG_R13D
	| REG_R13W
	| REG_R14
	| REG_R14B
	| REG_R14D
	| REG_R14W
	| REG_R15
	| REG_R15B
	| REG_R15D
	| REG_R15W
	| REG_R8
	| REG_R8B
	| REG_R8D
	| REG_R8W
	| REG_R9
	| REG_R9B
	| REG_R9D
	| REG_R9W
	| REG_RAX
	| REG_RBP
	| REG_RBX
	| REG_RCX
	| REG_RDI
	| REG_RDX
	| REG_RIP
	| REG_RSI
	| REG_RSP
	| REG_SI
	| REG_SIL
	| REG_SP
	| REG_SPL
	| REG_EIP
	| REG_IP
	;

size	: BYTE
	| WORD
	| DWORD
	| QWORD
	;

opt_offset
	: /* empty */
	  { $<num>$ = 0; }
	| Number
	| '+' Number
	  { $<num>$ = $2; }
	;

opt_number
	: /* empty */
	  { $<num>$ = 0; }
	| Number
	;

opt_scale
	: /* empty */
	  { $<num>$ = 0; }
	| '*' Number
	  { $<num>$ = $<num>2; }
	;

opt_index
	: /* empty */
	  { $<idx>$.reg = 0; $<idx>$.scale = 0; }
	| '+' register opt_scale
	  { $<idx>$.reg = $<num>2; $<idx>$.scale = $<num>3; }
	;

memory	: size PTR Name '[' REG_RIP opt_offset ']'
	  { int unused; khiter_t key = kh_put(Used, usedTab, $<str>3, &unused);
	    if (unused) kh_value(usedTab, key) = 0; $<mem>$.size = $<num>1;
	    $<mem>$.tag = NAME; $<mem>$.name = $<str>3; $<mem>$.offset =
	    $<num>6; }
	| size PTR opt_number '[' register opt_index ']'
	  { $<mem>$.size = $<num>1; $<mem>$.tag = REG; $<mem>$.reg = $<num>5;
	    $<mem>$.idx = $<idx>6; $<mem>$.num = $<num>3; }
	;

lea_memory
	: Name '[' REG_RIP opt_offset ']'
	  { int unused; khiter_t key = kh_put(Used, usedTab, $<str>1, &unused);
	    if (unused) kh_value(usedTab, key) = 0; $<mem>$.tag = NAME;
	    $<mem>$.name = $<str>1; $<mem>$.offset = $<num>4; }
	| opt_number '[' register opt_index ']'
	  { $<mem>$.tag = REG; $<mem>$.reg = $<num>3; $<mem>$.idx = $<idx>4;
	    $<mem>$.num = $<num>1; }
	| opt_number '[' Number '+' register opt_scale ']'
	  { $<mem>$.tag = SCALE; $<mem>$.idx.reg = $<num>5;
	    $<mem>$.idx.scale = $<num>6; $<mem>$.num = $<num>1 + $<num>3; }
	;

jmp_name
	: Name
	  { int unused; khiter_t key = kh_put(Used, usedTab, $<str>1, &unused);
	    if (unused) kh_value(usedTab, key) = 0; $<jmp>$.tag = NAME;
	    $<jmp>$.name = $1; }
	| register
	  { $<jmp>$.tag = REG; $<jmp>$.reg = $1; }
	| '[' memory ']'
	  { $<jmp>$.tag = MEM; $<jmp>$.mem = $2; }
	;

%%

void print_extern(void)
{
    khiter_t key;
    const char *str;

    for (key = kh_begin(usedTab); key != kh_end(usedTab); key++)
	if (kh_exist(usedTab, key)) {
	    str = kh_key(usedTab, key);
	    if (*str != '.' && !kh_value(usedTab, key))
		fprintf(textfp, "extern %s\n", str);
	}
}

void declare_bss(char *ptr, int leng)
{
    char *str;
    khiter_t key;

    if ((str = malloc(leng + 1)) != 0) {
	strncpy(str, ptr, leng);
	str[leng] = 0;

	key = kh_put(Used, usedTab, str, &leng);
	kh_value(usedTab, key) = 1;
    }
}

char *print_num(long num)
{
    char str[MAXSTR];

    sprintf(str, "%ld", num);
    return strdup(str);
}

char *print_reg(long num)
{
    switch (num) {
    case REG_AH   : return "ah";
    case REG_AL   : return "al";
    case REG_AX   : return "ax";
    case REG_BH   : return "bh";
    case REG_BL   : return "bl";
    case REG_BP   : return "bp";
    case REG_BPL  : return "bpl";
    case REG_BX   : return "bx";
    case REG_CH   : return "ch";
    case REG_CL   : return "cl";
    case REG_CX   : return "cx";
    case REG_DH   : return "dh";
    case REG_DI   : return "di";
    case REG_DIL  : return "dil";
    case REG_DL   : return "dl";
    case REG_DX   : return "dx";
    case REG_EAX  : return "eax";
    case REG_EBP  : return "ebp";
    case REG_EBX  : return "ebx";
    case REG_ECX  : return "ecx";
    case REG_EDI  : return "edi";
    case REG_EDX  : return "edx";
    case REG_ESI  : return "esi";
    case REG_ESP  : return "esp";
    case REG_R10  : return "r10";
    case REG_R10B : return "r10b";
    case REG_R10D : return "r10d";
    case REG_R10W : return "r10w";
    case REG_R11  : return "r11";
    case REG_R11B : return "r11b";
    case REG_R11D : return "r11d";
    case REG_R11W : return "r11w";
    case REG_R12  : return "r12";
    case REG_R12B : return "r12b";
    case REG_R12D : return "r12d";
    case REG_R12W : return "r12w";
    case REG_R13  : return "r13";
    case REG_R13B : return "r13b";
    case REG_R13D : return "r13d";
    case REG_R13W : return "r13w";
    case REG_R14  : return "r14";
    case REG_R14B : return "r14b";
    case REG_R14D : return "r14d";
    case REG_R14W : return "r14w";
    case REG_R15  : return "r15";
    case REG_R15B : return "r15b";
    case REG_R15D : return "r15d";
    case REG_R15W : return "r15w";
    case REG_R8   : return "r8";
    case REG_R8B  : return "r8b";
    case REG_R8D  : return "r8d";
    case REG_R8W  : return "r8w";
    case REG_R9   : return "r9";
    case REG_R9B  : return "r9b";
    case REG_R9D  : return "r9d";
    case REG_R9W  : return "r9w";
    case REG_RAX  : return "rax";
    case REG_RBP  : return "rbp";
    case REG_RBX  : return "rbx";
    case REG_RCX  : return "rcx";
    case REG_RDI  : return "rdi";
    case REG_RDX  : return "rdx";
    case REG_RIP  : return "rip";
    case REG_RSI  : return "rsi";
    case REG_RSP  : return "rsp";
    case REG_SI   : return "si";
    case REG_SIL  : return "sil";
    case REG_SP   : return "sp";
    case REG_SPL  : return "spl";
    case REG_EIP  : return "eip";
    case REG_IP   : return "ip";
    default       : return "?";
    }
}

char *print_size(long size)
{
    switch (size) {
    case BYTE  : return "byte";
    case WORD  : return "word";
    case DWORD : return "dword";
    case QWORD : return "qword";
    default    : return "?";
    }
}

char *print_mem(struct mem mem, int size)
{
    char str[MAXSTR];

    if (size)
	sprintf(str, "%s [", print_size(mem.size));
    else
	strcpy(str, "[");
    if (mem.tag == NAME) {
	sprintf(str + strlen(str), "%s", mem.name[0] == '.' ? mem.name + 1 :
		mem.name);
	if (mem.offset > 0)
	    sprintf(str + strlen(str), "+%ld", mem.offset);
	else if (mem.offset < 0)
	    sprintf(str + strlen(str), "%ld", mem.offset);
    } else if (mem.tag ==  REG) {
	sprintf(str + strlen(str), "%s", print_reg(mem.reg));
	if (mem.idx.reg) {
	    sprintf(str + strlen(str), "+%s", print_reg(mem.idx.reg));
	    if (mem.idx.scale)
		sprintf(str + strlen(str), "*%ld", mem.idx.scale);
	}
	if (mem.num > 0)
	    sprintf(str + strlen(str), "+%ld", mem.num);
	else if (mem.num < 0)
	    sprintf(str + strlen(str), "%ld", mem.num);
    } else {
	sprintf(str + strlen(str), "+%s", print_reg(mem.idx.reg));
	if (mem.idx.scale)
	    sprintf(str + strlen(str), "*%ld", mem.idx.scale);
	if (mem.num > 0)
	    sprintf(str + strlen(str), "+%ld", mem.num);
	else if (mem.num < 0)
	    sprintf(str + strlen(str), "%ld", mem.num);
    }
    strcat(str, "]");
    return strdup(str);
}

char *print_str(char *str)
{
    return *str == '.' ? str + 1 : str;
}
