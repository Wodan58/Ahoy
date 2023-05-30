%{
/*
    module  : my_lexer.l
    version : 1.5
    date    : 05/30/23
*/
#include <unistd.h>
#include <ctype.h>
#include "my_struc.h"
#include "my_parse.h"

#define TEXT	1
#define DATA	2
#define BSS	3

#define MAXSTR	100

static int size;
static unsigned char local;

unsigned char section;
FILE *textfp, *datafp, *bssfp;

void print_extern(void);
void declare_bss(char *ptr, int leng);
char *strip_dot(char *str);
char *print_str(char *str);
int copy_data(void);
void print_bss(char *str, int leng, int mode);
%}

%option yylineno
%option noyywrap
%option pointer

firstchar		[.@A-Z_a-z]
nextchar		[.0-9@A-Z_a-z]
symbol			{firstchar}{nextchar}*
integer			(0|-?[1-9][0-9]*)
escape			\\(["'\\btnvfr]|[01][0-9][0-9]|2[0-5][0-5])
string			\"([^"]|\\\")*\"

%%

PTR			return PTR;
BYTE			{ yylval.num = BYTE; return BYTE; }
WORD			{ yylval.num = WORD; return WORD; }
DWORD			{ yylval.num = DWORD; return DWORD; }
QWORD			{ yylval.num = QWORD; return QWORD; }

add			{ yylval.num = OP_ADD; return OP_ADD; }
and			{ yylval.num = OP_AND; return OP_AND; }
bt			{ yylval.num = OP_BT; return OP_BT; }
call			{ yylval.num = OP_CALL; return OP_CALL; }
cdqe			{ yylval.num = OP_CDQE; return OP_CDQE; }
cmove			{ yylval.num = OP_CMOVE; return OP_CMOVE; }
cmovle			{ yylval.num = OP_CMOVLE; return OP_CMOVLE; }
cmovne			{ yylval.num = OP_CMOVNE; return OP_CMOVNE; }
cmovns			{ yylval.num = OP_CMOVNS; return OP_CMOVNS; }
cmovs			{ yylval.num = OP_CMOVS; return OP_CMOVS; }
cmp			{ yylval.num = OP_CMP; return OP_CMP; }
cwde			{ yylval.num = OP_CWDE; return OP_CWDE; }
cqo			{ yylval.num = OP_CQO; return OP_CQO; }
dec			{ yylval.num = OP_DEC; return OP_DEC; }
idiv			{ yylval.num = OP_IDIV; return OP_IDIV; }
imul			{ yylval.num = OP_IMUL; return OP_IMUL; }
inc			{ yylval.num = OP_INC; return OP_INC; }
ja			{ yylval.num = OP_JA; return OP_JA; }
jae			{ yylval.num = OP_JAE; return OP_JAE; }
jb			{ yylval.num = OP_JB; return OP_JB; }
jbe			{ yylval.num = OP_JBE; return OP_JBE; }
jc			{ yylval.num = OP_JC; return OP_JC; }
je			{ yylval.num = OP_JE; return OP_JE; }
jg			{ yylval.num = OP_JG; return OP_JG; }
jge			{ yylval.num = OP_JGE; return OP_JGE; }
jl			{ yylval.num = OP_JL; return OP_JL; }
jle			{ yylval.num = OP_JLE; return OP_JLE; }
jmp			{ yylval.num = OP_JMP; return OP_JMP; }
jnb			{ yylval.num = OP_JNB; return OP_JNB; }
jnc			{ yylval.num = OP_JNC; return OP_JNC; }
jne			{ yylval.num = OP_JNE; return OP_JNE; }
jns			{ yylval.num = OP_JNS; return OP_JNS; }
js			{ yylval.num = OP_JS; return OP_JS; }
lea			{ yylval.num = OP_LEA; return OP_LEA; }
mov			{ yylval.num = OP_MOV; return OP_MOV; }
movabs			{ yylval.num = OP_MOVABS; return OP_MOVABS; }
movsx			{ yylval.num = OP_MOVSX; return OP_MOVSX; }
movzx			{ yylval.num = OP_MOVZX; return OP_MOVZX; }
mul			{ yylval.num = OP_MUL; return OP_MUL; }
nop			{ yylval.num = OP_NOP; return OP_NOP; }
div			{ yylval.num = OP_DIV; return OP_DIV; }
neg			{ yylval.num = OP_NEG; return OP_NEG; }
not			{ yylval.num = OP_NOT; return OP_NOT; }
or			{ yylval.num = OP_OR; return OP_OR; }
pop			{ yylval.num = OP_POP; return OP_POP; }
push			{ yylval.num = OP_PUSH; return OP_PUSH; }
repz			{ yylval.num = OP_REPZ; return OP_REPZ; }
cmpsb			{ yylval.num = OP_CMPSB; return OP_CMPSB; }
ret			{ yylval.num = OP_RET; return OP_RET; }
sal			{ yylval.num = OP_SAL; return OP_SAL; }
sar			{ yylval.num = OP_SAR; return OP_SAR; }
sbb			{ yylval.num = OP_SBB; return OP_SBB; }
seta			{ yylval.num = OP_SETA; return OP_SETA; }
setbe			{ yylval.num = OP_SETBE; return OP_SETBE; }
sete			{ yylval.num = OP_SETE; return OP_SETE; }
setg			{ yylval.num = OP_SETG; return OP_SETG; }
setge			{ yylval.num = OP_SETGE; return OP_SETGE; }
setl			{ yylval.num = OP_SETL; return OP_SETL; }
setle			{ yylval.num = OP_SETLE; return OP_SETLE; }
setne			{ yylval.num = OP_SETNE; return OP_SETNE; }
shr			{ yylval.num = OP_SHR; return OP_SHR; }
sub			{ yylval.num = OP_SUB; return OP_SUB; }
test			{ yylval.num = OP_TEST; return OP_TEST; }
xor			{ yylval.num = OP_XOR; return OP_XOR; }

ah			{ yylval.num = REG_AH; return REG_AH; }
al			{ yylval.num = REG_AL; return REG_AL; }
ax			{ yylval.num = REG_AX; return REG_AX; }
bh			{ yylval.num = REG_BH; return REG_BH; }
bl			{ yylval.num = REG_BL; return REG_BL; }
bp			{ yylval.num = REG_BP; return REG_BP; }
bpl			{ yylval.num = REG_BPL; return REG_BPL; }
bx			{ yylval.num = REG_BX; return REG_BX; }
ch			{ yylval.num = REG_CH; return REG_CH; }
cl			{ yylval.num = REG_CL; return REG_CL; }
cx			{ yylval.num = REG_CX; return REG_CX; }
dh			{ yylval.num = REG_DH; return REG_DH; }
di			{ yylval.num = REG_DI; return REG_DI; }
dil			{ yylval.num = REG_DIL; return REG_DIL; }
dl			{ yylval.num = REG_DL; return REG_DL; }
dx			{ yylval.num = REG_DX; return REG_DX; }
eax			{ yylval.num = REG_EAX; return REG_EAX; }
ebp			{ yylval.num = REG_EBP; return REG_EBP; }
ebx			{ yylval.num = REG_EBX; return REG_EBX; }
ecx			{ yylval.num = REG_ECX; return REG_ECX; }
edi			{ yylval.num = REG_EDI; return REG_EDI; }
edx			{ yylval.num = REG_EDX; return REG_EDX; }
esi			{ yylval.num = REG_ESI; return REG_ESI; }
esp			{ yylval.num = REG_ESP; return REG_ESP; }
r10			{ yylval.num = REG_R10; return REG_R10; }
r10b			{ yylval.num = REG_R10B; return REG_R10B; }
r10d			{ yylval.num = REG_R10D; return REG_R10D; }
r10w			{ yylval.num = REG_R10W; return REG_R10W; }
r11			{ yylval.num = REG_R11; return REG_R11; }
r11b			{ yylval.num = REG_R11B; return REG_R11B; }
r11d			{ yylval.num = REG_R11D; return REG_R11D; }
r11w			{ yylval.num = REG_R11W; return REG_R11W; }
r12			{ yylval.num = REG_R12; return REG_R12; }
r12b			{ yylval.num = REG_R12B; return REG_R12B; }
r12d			{ yylval.num = REG_R12D; return REG_R12D; }
r12w			{ yylval.num = REG_R12W; return REG_R12W; }
r13			{ yylval.num = REG_R13; return REG_R13; }
r13b			{ yylval.num = REG_R13B; return REG_R13B; }
r13d			{ yylval.num = REG_R13D; return REG_R13D; }
r13w			{ yylval.num = REG_R13W; return REG_R13W; }
r14			{ yylval.num = REG_R14; return REG_R14; }
r14b			{ yylval.num = REG_R14B; return REG_R14B; }
r14d			{ yylval.num = REG_R14D; return REG_R14D; }
r14w			{ yylval.num = REG_R14W; return REG_R14W; }
r15			{ yylval.num = REG_R15; return REG_R15; }
r15b			{ yylval.num = REG_R15B; return REG_R15B; }
r15d			{ yylval.num = REG_R15D; return REG_R15D; }
r15w			{ yylval.num = REG_R15W; return REG_R15W; }
r8			{ yylval.num = REG_R8; return REG_R8; }
r8b			{ yylval.num = REG_R8B; return REG_R8B; }
r8d			{ yylval.num = REG_R8D; return REG_R8D; }
r8w			{ yylval.num = REG_R8W; return REG_R8W; }
r9			{ yylval.num = REG_R9; return REG_R9; }
r9b			{ yylval.num = REG_R9B; return REG_R9B; }
r9d			{ yylval.num = REG_R9D; return REG_R9D; }
r9w			{ yylval.num = REG_R9W; return REG_R9W; }
rax			{ yylval.num = REG_RAX; return REG_RAX; }
rbp			{ yylval.num = REG_RBP; return REG_RBP; }
rbx			{ yylval.num = REG_RBX; return REG_RBX; }
rcx			{ yylval.num = REG_RCX; return REG_RCX; }
rdi			{ yylval.num = REG_RDI; return REG_RDI; }
rdx			{ yylval.num = REG_RDX; return REG_RDX; }
rip			{ yylval.num = REG_RIP; return REG_RIP; }
rsi			{ yylval.num = REG_RSI; return REG_RSI; }
rsp			{ yylval.num = REG_RSP; return REG_RSP; }
si			{ yylval.num = REG_SI; return REG_SI; }
sil			{ yylval.num = REG_SIL; return REG_SIL; }
sp			{ yylval.num = REG_SP; return REG_SP; }
spl			{ yylval.num = REG_SPL; return REG_SPL; }
eip			{ yylval.num = REG_EIP; return REG_EIP; }
ip			{ yylval.num = REG_IP; return REG_IP; }

{integer}		{ yylval.num = strtol(yytext, 0, 0); return Number; }
{string}		{ char *str; fputc('\'', datafp); for (str = yytext +
			  1; *str != '"'; str++) if (*str == '\\') if (*++str
			  == 'n') fprintf(datafp, "',10,'"); else fputc(*str,
			  datafp); else fputc(*str, datafp); fprintf(datafp,
			  "',0\n"); }
\.align.*		;
\.byte.*		fprintf(datafp, "\tdb\t%s\n", strip_dot(yytext + 6));
\.cfi_startproc.*	;
\.cfi_def_cfa_offset.*	;
\.cfi_offset.*		;
\.cfi_restore.*		;
\.cfi_endproc.*		;
\.cfi_remember.*	;
\.def.*			;
\.zero.*		;
\.lcomm.*		{ int leng; char *ptr, *str = yytext + 6; while
			  (isspace((int)*str)) str++; ptr = str; while
			  (*ptr != ',') ptr++; leng = ptr - str;
			  local = 1; print_bss(str, leng, 1);
			  declare_bss(str, leng); local = 0; }
\.comm.*		{ int leng; char *ptr, *str = yytext + 5; while
			  (isspace((int)*str)) str++; ptr = str; while
			  (*ptr != ',') ptr++; leng = ptr - str;
			  print_bss(str, leng, 1); declare_bss(str, leng);
			  local = 0; }
\.file.*		;
\.globl.*		;
\.intel_syntax.*	;
\.linkonce.*		;
\.local.*		local = 1;
\.long.*		fprintf(datafp, "\tdd\t%s\n", strip_dot(yytext + 6));
\.p2align.*		;
\.quad.*		fprintf(datafp, "\tdq\t%s\n", strip_dot(yytext + 6));
\.section.*		{ if (strstr(yytext + 8, ".text")) section = TEXT; else
			  section = DATA; }
\.space.*		{ fprintf(bssfp, "\tresb\t%d\n", atoi(yytext + 7)); }
\.size.*		{ char *ptr; if (section == BSS)
			  if ((ptr = strchr(yytext, ',')) != 0)
			  size = atoi(ptr + 1); }
\.ascii			|
\.string		fprintf(datafp, "\tdb\t");
\.text.*		section = TEXT;
\.rdata.*		|
\.data.*		section = DATA;
\.bss.*			section = BSS;
\.type.*		;
\.word.*		fprintf(datafp, "\tdw\t%s\n", strip_dot(yytext + 6));

rep			;
{symbol}		{ char *ptr; if ((ptr = strchr(yytext, '@')) != 0)
			  *ptr = 0; if (section == BSS) { print_bss(yytext,
			  yyleng, 0); declare_bss(yytext, yyleng); local = 0; }
			  else { yylval.str = strdup(yytext); return Name; } }
[:]			if (section != BSS) return yytext[0];
[*+,[\]]		return yytext[0];
\n			|
.			;

%%

int main(int argc, char *argv[])
{
    int rv, ch;
    char str[MAXSTR + 1];

    if (argc != 2) {
	fprintf(stderr, "Use: %s file\n", argv[0]);
	return 1;
    }
    if ((yyin = fopen(argv[1], "r")) == 0) {
	fprintf(stderr, "Could not read %s\n", argv[1]);
	return 1;
    }
    snprintf(str, MAXSTR, "%s.text", argv[1]);
    if ((textfp = fopen(str, "w")) == 0) {
	fprintf(stderr, "Could not write %s\n", str);
	return 1;
    }
    snprintf(str, MAXSTR, "%s.data", argv[1]);
    if ((datafp = fopen(str, "w+")) == 0) {
	fprintf(stderr, "Could not write %s\n", str);
	return 1;
    }
    snprintf(str, MAXSTR, "%s.bss", argv[1]);
    if ((bssfp = fopen(str, "w+")) == 0) {
	fprintf(stderr, "Could not write %s\n", str);
	return 1;
    }
    rv = yyparse();
    if (!copy_data()) {
	fprintf(stderr, "not yet done\n");
	copy_data();
    }
    rewind(bssfp);
    fprintf(textfp, "\tsection .bss\n");
    while ((ch = fgetc(bssfp)) != EOF)
	fputc(ch, textfp);
    print_extern();
    snprintf(str, MAXSTR, "%s.data", argv[1]);
    if (remove(str))
	rv++;
    snprintf(str, MAXSTR, "%s.bss", argv[1]);
    if (remove(str))
	rv++;
    return rv;
}

int yyerror(char *str)
{
    fprintf(stderr, "%s in line %d near %.*s\n", str, yylineno, yyleng, yytext);
    return 0;
}

int copy_data(void)
{
    static long new_offset, old_offset;
    int ch;

    if (!new_offset) {
	old_offset = new_offset;
	new_offset = ftell(datafp);
	return 0;
    }
    if (fseek(datafp, old_offset, 0))
	return 1;
    while (ftell(datafp) != new_offset) {
	ch = fgetc(datafp);
	fputc(ch, textfp);
    }
    old_offset = new_offset;
    if (fseek(datafp, 0, 2))
	return 1;
    new_offset = ftell(datafp);
    return new_offset == old_offset;
}

char *strip_dot(char *str)
{
    int i, j;

    for (j = i = 0; str[i]; j++, i++) {
	if (str[i] == '.')
	    i++;
	str[j] = str[i];
    }
    str[j] = 0;
    if (*str != 'L')
	fprintf(bssfp, "extern %s\n", str);
    return str;
}

void print_bss(char *str, int leng, int mode)
{
    if (!local)
	fprintf(bssfp, "\tglobal %.*s\n", leng, str);
    if (mode)
	fprintf(bssfp, "%.*s:\n\tresb\t%d\n", leng, str, atoi(str + leng + 1));
    else if (size)
	fprintf(bssfp, "%.*s:\n\tresb\t%d\n", leng, str, size);
    else
	fprintf(bssfp, "%.*s:\n", leng, str);
    size = 0;
}
