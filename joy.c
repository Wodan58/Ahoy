/*
    module  : joy.c
    version : 1.4
    date    : 05/31/21
*/
#if 0
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <setjmp.h>
#endif

#include <inttypes.h>
#include "my_libc.h"

#define my_isspace(ch)	(ch < '!')

#ifdef _MSC_VER
#pragma warning(disable : 4244 4996)
#endif

#define CORRECT_GARBAGE
#define READ_LIBRARY_ONCE

#define LOGFILE(s)

#if 0
#ifdef DEBUG
int debug = 1;
#undef LOGFILE
#define LOGFILE(s)	{ if (debug) { FILE *fp = fopen("joy.log",\
			strcmp(s, "main") ? "a" : "w");\
			fprintf(fp, "%s\n", s); fclose(fp); } }
#endif
#endif

typedef unsigned char boolean;

#define true  1
#define false 0

#define errormark	"%JOY"
#define lib_filename	"42minjoy.lib"
#define list_filename	"42minjoy.lst"

#define reslength	8
#if 0
#define emptyres	"        "
#endif
#define maxrestab	10

#define identlength	16
#if 0
#define emptyident	"                "
#endif
#define maxstdidenttab	32

typedef enum {
    lbrack, rbrack, semic, period, def_equal,
/* compulsory for scanutilities: */
    charconst, stringconst, numberconst, leftparenthesis, identifier
/* hyphen */
} symbol;

typedef enum {
    lib_, mul_, add_, sub_, div_, lss_, eql_, and_, body_, cons_, dip_,
    dup_, false_, get_, getch_, i_, index_, not_, nothing_, or_, pop_, put_,
    putch_, sametype_, select_, stack_, step_, swap_, true_, uncons_, unstack_,
    boolean_, char_, integer_, list_, unknownident
} standardident;

/* File: Included file for scan utilities */

#define maxincludelevel	  5
#define maxlinelength	  132
#define linenumwidth	  4

#define linenumspace	  "    "
#define linenumsep	  "    "
#define underliner	  "****    "
#if 0
#define tab_in_listing	  "    ----    "
#endif

#define maxoutlinelength  60
#define messagelength	  30

#define initial_alternative_radix  2

#if 0
#define maxchartab	  1000
#define maxstringtab	  100
#define maxnodtab	  1000
#endif

typedef char identalfa[identlength + 1];
typedef char resalfa[reslength + 1];

#if 0
typedef char message[messagelength + 1];

typedef struct toops {
    long symbols, types, strings, chars;
} toops;
#endif

typedef struct _REC_inputs {
    long fil;
    identalfa nam;
    long lastlinenumber;
} _REC_inputs;

typedef struct _REC_reswords {
    resalfa alf;
    symbol symb;
} _REC_reswords;

typedef struct _REC_stdidents {
    identalfa alf;
    standardident symb;
} _REC_stdidents;

static long listing;

static _REC_inputs inputs[maxincludelevel];
static long includelevel, adjustment, writelisting;
static boolean must_repeat_line;

static long scantimevariables['Z' + 1 - 'A'];
static long alternative_radix, linenumber;
static char line[maxlinelength + 1];
static int cc, ll;
static int my_ch;
static identalfa ident;
static standardident id;
static char specials_repeat[maxrestab + 1];
static symbol sym;
static long num;
static _REC_reswords reswords[maxrestab + 1];
static int lastresword;
static _REC_stdidents stdidents[maxstdidenttab + 1];
static int laststdident;
#if 0
static long trace;
static long stringtab[maxstringtab + 1];
static char chartab[maxchartab + 1];

static toops toop;
#endif

static long errorcount, outlinelength, statistics;
static clock_t start_clock, end_clock;

/* - - - - -   MODULE ERROR    - - - - - */

static void point_to_symbol(boolean repeatline, long f, char diag, char *mes)
{
    int i;

    LOGFILE(__func__);
    if (repeatline) {
	my_fprint(f, "%*ld%s", linenumwidth, linenumber, linenumsep);
	for (i = 0; i < ll; i++)
	    my_fputc(line[i], f);
	my_fputc('\n', f);
    }
    my_fputs(underliner, f);
    for (i = 0; i < cc - 2; i++)
	if (my_isspace(line[i]))
	    my_fputc(line[i], f);
	else
	    my_fputc(' ', f);
    my_fprint(f, "^\n%s-%c  %-*.*s\n", errormark, diag, messagelength,
	      messagelength, mes);
    if (diag == 'F')
	my_fprint(f, "execution aborted\n");
}  /* point_to_symbol */

static void point(char diag, char *mes)
{
    LOGFILE(__func__);
    if (diag != 'I')
	errorcount++;
    if (includelevel > 0)
	my_fprint(STDOUT, "INCLUDE file : \"%-*.*s\"\n", identlength,
		  identlength, inputs[includelevel - 1].nam);
    point_to_symbol(true, STDOUT, diag, mes);
    if (writelisting > 0) {
	point_to_symbol(must_repeat_line, listing, diag, mes);
	must_repeat_line = true;
    }
    if (diag == 'F')
        my_exit(0);
}  /* point */

/* - - - - -   MODULE SCANNER  - - - - - */

static void closelisting(void)
{
    my_close(listing);
}

/*
    iniscanner - initialize global variables
*/
static void iniscanner(void)
{
    LOGFILE(__func__);
    if ((listing = my_open(list_filename, O_CREAT | O_RDWR, 0644)) == ERROR) {
	my_fprint(STDERR, "%s (not open for writing)\n", list_filename);
	my_exit(0);
    }
    my_atexit(closelisting);
#if 0
    writelisting = 0;
    my_ch = ' ';
    linenumber = 0;
    cc = 1;
    ll = 1;		    /* to enable fatal message during initialisation */
    memset(specials_repeat, 0, sizeof(specials_repeat));  /* def: no repeats */
    includelevel = 0;
    adjustment = 0;
#endif
    alternative_radix = initial_alternative_radix;
#if 0
    lastresword = 0;
    laststdident = 0;
    outlinelength = 0;
    memset(scantimevariables, 0, sizeof(scantimevariables));
    errorcount = 0;
    must_repeat_line = false;
#endif
}  /* iniscanner */

static void erw(char *a, symbol symb)
{
    LOGFILE(__func__);
    if (++lastresword > maxrestab)
	point('F', "too many reserved words");
    my_strncpy(reswords[lastresword].alf, a, reslength);
    reswords[lastresword].alf[reslength] = 0;
    reswords[lastresword].symb = symb;
}  /* erw */

static void est(char *a, standardident symb)
{
    LOGFILE(__func__);
    if (++laststdident > maxstdidenttab)
	point('F', "too many identifiers");
    my_strncpy(stdidents[laststdident].alf, a, identlength);
    stdidents[laststdident].alf[identlength] = 0;
    stdidents[laststdident].symb = symb;
}  /* est */

static void initinputs(void)
{
    int i;

    for (i = 0; i < maxincludelevel; i++)
	inputs[i].fil = ERROR;
}

static void release(void)
{
    int i;

    for (i = 0; i < maxincludelevel; i++)
	if (inputs[i].fil != ERROR)
	    my_close(inputs[i].fil);
}

static void newfile(char *a)
{
    static int init;
#if 0
    int i;
    char str[256];
#endif

    LOGFILE(__func__);
    if (!init) {
	init = 1;
	initinputs();
	my_atexit(release);
    }
    my_strncpy(inputs[includelevel].nam, a, identlength);
    inputs[includelevel].nam[identlength] = 0;
    inputs[includelevel].lastlinenumber = linenumber;
#if 0
    sprintf(str, "%-*.*s", identlength, identlength, a);
    for (i = strlen(str) - 1; isspace((int)str[i]); i--)
	;
    str[i + 1] = 0;
    for (i = 0; i < includelevel; i++)
	if (!my_strcmp(inputs[i].nam, inputs[includelevel].nam)) {
	    my_close(inputs[i].fil);
	    break;
	}
#endif
    if (inputs[includelevel].fil != ERROR)
	my_close(inputs[includelevel].fil);
    if ((inputs[includelevel].fil = my_open(a, O_RDWR, 0)) == ERROR) {
	my_fprint(STDERR, "%s (not open for reading)\n", a);
	my_exit(0);
    }
    adjustment = 1;
}  /* newfile */

static void getsym(void);

#if 0
#define emptydir	"                "
#endif
#define dirlength	16

static void perhapslisting(void)
{
    int i, j;

    LOGFILE(__func__);
    if (writelisting > 0) {
        my_fprint(listing, "%*ld", linenumwidth, linenumber);
        for (i = ll - 1; i > 0 && my_isspace(line[i]); i--)
            ;
        if (line[j = i + 1] != 0) {
            my_fprint(listing, "%s", linenumsep);
	    for (i = 0; i < j; i++)
		my_fputc(line[i], listing);
	}
	my_fputc('\n', listing);
	must_repeat_line = false;
	my_flush(listing);
    }
}

static void getch(void)
{
#if 0
    int c;
#endif
    long f;

    LOGFILE(__func__);
    if (cc == ll) {
	if (adjustment != 0) {
	    if (adjustment == -1)
		linenumber = inputs[includelevel - 1].lastlinenumber;
	    else
		linenumber = 0;
	    includelevel += adjustment;
	    adjustment = 0;
	}
	linenumber++;
	ll = 0;
	cc = 0;
	f = includelevel ? inputs[includelevel - 1].fil : STDIN;
	if (my_fgets(line, maxlinelength, f) == OK) {
	    ll = my_strlen(line);
	    perhapslisting();
	} else if (includelevel) {
	    my_close(f);
	    inputs[includelevel - 1].fil = ERROR;
	    adjustment = -1;
	}
	line[ll++] = ' ';
    }  /* IF */
    my_ch = line[cc++];
}  /* getch */

static long value(void)
{
    /* this is a  LL(0) parser */
    long result = 0;

    LOGFILE(__func__);
    do
	getch();
    while (my_isspace(my_ch));
    if (my_ch == '\'' || my_ch == '&' || my_isdigit(my_ch)) {
	getsym();
	result = num;
	goto einde;
    }
    if (my_isupper(my_ch)) {
	result = scantimevariables[my_ch - 'A'];
#if 0
	getsym();
#endif
	goto einde;
    }
    if (my_ch == '(') {
	result = value();
	while (my_isspace(my_ch))
	    getch();
	if (my_ch == ')')
	    getch();
	else
	    point('E', "right parenthesis expected");
	goto einde;
    }
    switch (my_ch) {

    case '+':
	result = value() + value();
	break;

    case '-':
	result = value();
	result -= value();
	break;

    case '*':
	result = value() * value();
	break;

    case '/':
	result = value();
	result /= value();
	break;

    case '=':
	result = value() == value();
	break;

    case '>':
	result = value();
	result = result > value();
	break;

    case '<':
	result = value();
	result = result < value();
	break;

    case '?':
#if 0
	if (scanf("%ld", &result) != 1)
	    result = 0;
#endif
	break;

    default:
	point('F', "illegal start of scan expr");
    }  /* CASE */
einde:
    return result;
}  /* value */

static void directive(void)
{
    int i, j;
    char c, dir[dirlength + 1];

    LOGFILE(__func__);
    getch();
    i = 0;
#if 0
    strncpy(dir, emptydir, dirlength);
#endif
    do {
	if (i < dirlength)
	    dir[i++] = my_ch;
	getch();
    } while (my_ch == '_' || my_isupper(my_ch));
    dir[i] = 0;
    if (!my_strcmp(dir, "IF")) {
	if (value() < 1)
	    cc = ll;  /* readln */
    } else if (!my_strcmp(dir, "INCLUDE")) {
	if (includelevel == maxincludelevel)
	    point('F', "too many include files");
	while (my_isspace(my_ch))
	    getch();
	i = 0;
#if 0
	strncpy(ident, emptyident, identlength);
#endif
	do {
	    if (i < identlength)
		ident[i++] = my_ch;
	    getch();
	} while (!my_isspace(my_ch));
	ident[i] = 0;
	newfile(ident);
    } else if (!my_strcmp(dir, "PUT")) {
        for (i = ll - 1; i > 0 && my_isspace(line[i]); i--)
            ;
	for (j = i + 1, i = cc - 1; i < j; i++)
	    my_fputc(line[i], STDERR);
	my_fputc('\n', STDERR);
	cc = ll;
    } else if (!my_strcmp(dir, "SET")) {
	while (my_isspace(my_ch))
	    getch();
	if (!my_isupper(my_ch))
	    point('E', "\"A\" .. \"Z\" expected");
	c = my_ch;
	getch();
	while (my_isspace(my_ch))
	    getch();
	if (my_ch != '=')
	    point('E', "\"=\" expected");
	scantimevariables[c - 'A'] = value();
    } else if (!my_strcmp(dir, "LISTING")) {
	i = writelisting;
	writelisting = value();
	if (!i)
	    perhapslisting();
    } else if (!my_strcmp(dir, "STATISTICS"))
	statistics = value();
    else if (!my_strcmp(dir, "RADIX"))
	alternative_radix = value();
#if 0
    else if (!strncmp(dir, "TRACE           ", dirlength))
	trace = value();
#endif
    else
	point('F', "unknown directive");
    getch();
}  /* directive */

#if 0
#undef emptydir
#undef dirlength
#endif

static void getsym(void)
{
#if 0
    char c;
#endif
    boolean negated;
    long i, j, k, cmp;

    LOGFILE(__func__);
begin:
    ident[i = 0] = 0;
    while (my_isspace(my_ch))
	getch();
    switch (my_ch) {

    case '\'':
	getch();
	if (my_ch == '\\')
	    num = value();
	else {
	    num = my_ch;
	    getch();
	}
	if (my_ch == '\'')
	    getch();
	sym = charconst;
	break;

#if 0
    case '"':
	if (toop.strings == maxstringtab)
	    point('F', "too many strings              ");
	stringtab[num = ++toop.strings] = toop.chars + 1;
	getch();
	while (ch != '"') {
	    if ((c = ch) == '\\')
		c = value();
	    else
		getch();
	    if (++toop.chars > maxchartab)
		point('F', "too many characters in strings");
	    chartab[toop.chars] = c;
	}  /* WHILE */
	getch();
	stringtab[num + 1] = toop.chars;
	sym = stringconst;
	/* FOR i := stringtab[num] TO stringtab[num+1] DO
	    write(chartab[i]) */
	break;
#endif

    case '(':
	getch();
	if (my_ch == '*') {
	    getch();
	    do {
		while (my_ch != '*')
		    getch();
		getch();
	    } while (my_ch != ')');
	    getch();
	    goto begin;
	}
	sym = leftparenthesis;
	break;

    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	if (my_ch != '-')
	    negated = false;
	else {
	    getch();
	    if (!my_isdigit(my_ch)) {	
#if 0
		strncpy(res, emptyres, reslength);
		strncpy(ident, emptyident, identlength);
#endif
		ident[i++] = '-';
		ident[i] = 0;
		/* sym = hyphen; */
		goto einde;
	    }
	    negated = true;
	}
	sym = numberconst;
	num = 0;
	do {
	    num = num * 10 + my_ch - '0';
	    getch();
	} while (my_isdigit(my_ch));
	if (negated)
	    num = -num;
	break;

    case '&':  /* number in alternative radix */
	sym = numberconst;
	num = 0;
	getch();
	while (my_isdigit(my_ch) || my_isupper(my_ch)) {
	    if (my_isupper(my_ch))
		my_ch += '9' - 'A' + 1;
	    if (my_ch >= alternative_radix + '0')
		point('E', "exceeding alternative radix");
	    num = alternative_radix * num + my_ch - '0';
	    getch();
	}
	break;

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
	sym = identifier;
#if 0
	strncpy(ident, emptyident, identlength);
#endif
	do {
	    if (i < identlength)
		ident[i++] = my_ch;
	    getch();
	} while (my_ch == '_' || my_isalnum(my_ch));
	ident[i] = 0;
	break;

    case '%':
	directive();
	goto begin;

    default:
#if 0
	strncpy(res, emptyres, reslength);
	strncpy(ident, emptyident, identlength);
#endif
einde:
	if (my_isupper(my_ch))
	    do {
		if (i < identlength) {
		    ident[i++] = my_ch;
		    ident[i] = 0;
		}
		getch();
	    } while (my_ch == '_' || my_isalnum(my_ch));
	else if (!my_isspace(my_ch))
	    do {
		if (i < identlength) {
		    ident[i++] = my_ch;
		    ident[i] = 0;
		}
		getch();
	    } while (my_strchr(specials_repeat, my_ch));
	i = 1;
	j = lastresword;
	do {
	    k = (i + j) / 2;
	    cmp = my_strcmp(ident, reswords[k].alf);
	    if (cmp <= 0)
		j = k - 1;
	    if (cmp >= 0)
		i = k + 1;
	} while (i <= j);
	if (i - 1 > j)	/* OTHERWISE */
	    sym = reswords[k].symb;
	else
	    sym = identifier;
	break;
    }  /* CASE */
}  /* getsym */

/* - - - - -   MODULE OUTPUT   - - - - - */

static void putch(char c)
{
    LOGFILE(__func__);
    my_fputc(c, STDOUT);
    if (writelisting > 0) {
	if (!outlinelength)
	    my_fprint(listing, "%s%s", linenumspace, linenumsep);
	my_fputc(c, listing);
    }
    if (c == '\n')
	outlinelength = 0;
    else
	outlinelength++;
}

static void writeline(void)
{
    LOGFILE(__func__);
    my_fputc('\n', STDOUT);
    if (writelisting > 0)
	my_fputc('\n', listing);
    outlinelength = 0;
}

static void writeident(char *a)
{
    int i, length;

    LOGFILE(__func__);
#if 0
    length = identlength;
    while (a[length - 1] <= ' ')
	length--;
#else
    length = my_strlen(a);
#endif
    if (outlinelength + length > maxoutlinelength)
	writeline();
    for (i = 0; i < length; i++)
	putch(a[i]);
}

#if 0
static void writeresword(char *a)
{
    long i, length;

    length = reslength;
    while (a[length - 1] <= ' ')
	length--;
    if (outlinelength + length > maxoutlinelength)
	writeline();
    for (i = 0; i < length; i++)
	putch(a[i]);
}
#endif

static void writenatural(unsigned long n)
{
    LOGFILE(__func__);
    if (n >= 10)
	writenatural(n / 10);
    putch(n % 10 + '0');
}

static void writeinteger(long i)
{
    LOGFILE(__func__);
    if (outlinelength + 12 > maxoutlinelength)
	writeline();
    if (i >= 0)
	writenatural(i);
    else {
	putch('-');
	writenatural(-i);
    }
}  /* writeinteger */

static void fin(long f)
{
    LOGFILE(__func__);
    if (errorcount > 0)
	my_fprint(f, "%ld error(s)\n", errorcount);
    end_clock = my_clock() - start_clock;
    my_fprint(f, "%ld microseconds CPU\n", end_clock);
}

static void finalise(void)
{
    LOGFILE(__func__);
    /* finalise */
    fin(STDERR);
    if (writelisting > 0)
	fin(listing);
    /* finalise */
}

static void initialise(void)
{
    unsigned i;

    LOGFILE(__func__);
    iniscanner();
    my_strcpy(specials_repeat, "=>");
    erw(".",	period);
    erw(";",	semic);
    erw("==",	def_equal);
    erw("[",	lbrack);
    erw("]",	rbrack);
    est("*",        mul_);
    est("+",        add_);
    est("-",        sub_);
    est("/",        div_);
    est("<",        lss_);
    est("=",        eql_);
    est("and",      and_);
    est("body",     body_);
    est("cons",     cons_);
    est("dip",      dip_);
    est("dup",      dup_);
    est("false",    false_);
    est("get",      get_);
    est("getch",    getch_);
    est("i",        i_);
    est("index",    index_);
    est("not",      not_);
    est("nothing",  nothing_);
    est("or",       or_);
    est("pop",      pop_);
    est("put",      put_);
    est("putch",    putch_);
    est("sametype", sametype_);
    est("select",   select_);
    est("stack",    stack_);
    est("step",     step_);
    est("swap",     swap_);
    est("true",     true_);
    est("uncons",   uncons_);
    est("unstack",  unstack_);
    for (i = mul_; i <= unstack_; i++)
	if (i != stdidents[i].symb)
	    point('F', "bad order in standard idents");
}  /* initialise */

#ifndef MAXTABLE
#define MAXTABLE	300
#endif
#ifndef MAXMEM
#define MAXMEM		1999
#endif

typedef short memrange;

typedef struct _REC_table {
    identalfa alf;
    memrange adr;
} _REC_table;

typedef struct _REC_m {
    intptr_t val;
    memrange nxt;
    unsigned char op;
    boolean marked;
} _REC_m;

static _REC_table table[MAXTABLE + 1];
static long lastlibloc, sentinel, lasttable, locatn;
static _REC_m m[MAXMEM + 1];
static memrange firstusernode, freelist, programme;
static memrange s,  /* stack */
		dump;

#if 0
static standardident last_op_executed;
#endif
static long stat_kons, stat_gc, stat_ops, stat_calls;
static clock_t stat_lib;

#if 0
static char *standardident_NAMES[] = {
    "LIB", "*", "+", "-", "/", "<", "=", "and", "body", "cons", "dip", "dup",
    "false", "get", "getch", "i", "index", "not", "nothing", "or", "pop",
    "put", "putch", "sametype", "select", "stack", "step", "swap", "true",
    "uncons", "unstack", "BOOLEAN", "CHAR", "INTEGER", "LIST", "UNKNOWN"
};
#endif

#if 0
void DumpM(void)
{
    long i;
    FILE *fp = fopen("joy.dmp", "w");

    LOGFILE(__func__);
    fprintf(fp, "Table\n");
    fprintf(fp, "  nr %-*.*s  adr\n", identlength, identlength, "name");
    for (i = 1; i <= MAXTABLE && table[i].adr; i++)
	fprintf(fp, "%4ld %-*.*s %4ld\n", i, identlength, identlength,
		table[i].alf, (long)table[i].adr);
    fprintf(fp, "\nMemory\n");
    fprintf(fp, "  nr %-*.*s      value next M\n", identlength,
	    identlength, "name");
    for (i = 1; i <= MAXMEM && m[i].marked; i++)
	fprintf(fp, "%4ld %-*.*s %10ld %4ld %c\n", i, identlength, identlength,
		standardident_NAMES[m[i].op], m[i].val, (long)m[i].nxt,
		m[i].marked ? 'T' : 'F');
    fclose(fp);
}
#endif

static void lookup(void)
{
    long i, j, cmp;

    LOGFILE(__func__);
#ifdef READ_LIBRARY_ONCE
    if (!sentinel) {
	id = unknownident;
	return;
    }
#endif
    locatn = 0;
    if (sentinel > 0) {	 /* library has been read */
	my_strcpy(table[sentinel].alf, ident);
	locatn = lasttable;
	while (my_strcmp(table[locatn].alf, ident))
	    locatn--;
    }
    if (locatn > sentinel)
	id = lib_;
    else {
	i = 1;
	j = lastlibloc;
	do {
	    locatn = (i + j) / 2;
	    cmp = my_strcmp(ident, table[locatn].alf);
	    if (cmp <= 0)
		j = locatn - 1;
	    if (cmp >= 0)
		i = locatn + 1;
	} while (i <= j);
	if (i - 1 > j)
	    id = lib_;
	else {	/* binarysearch through standardidentifiers */
	    i = 1;
	    j = laststdident;
	    do {
		locatn = (i + j) / 2;
		cmp = my_strcmp(ident, stdidents[locatn].alf);
		if (cmp <= 0)
		    j = locatn - 1;
		if (cmp >= 0)
		    i = locatn + 1;
	    } while (i <= j);
	    if (i - 1 > j)
		id = stdidents[locatn].symb;
	    else {
#ifndef READ_LIBRARY_ONCE
		if (!sentinel)
		    id = unknownident;
		else {
#endif
		    if (lasttable == MAXTABLE)
			point('F', "too many library symbols");
		    my_strcpy(table[++lasttable].alf, ident);
		    table[locatn = lasttable].adr = 0;
		    id = lib_;
#ifndef READ_LIBRARY_ONCE
		}
#endif
	    }
	}  /* ELSE */
    }  /* ELSE */
#if 0
    if (writelisting > 4)
	fprintf(listing, "lookup : %-*.*s at %ld\n", identlength, identlength,
		standardident_NAMES[id], locatn);
#endif
}  /* lookup */

#if 0
static void wn(FILE *f, memrange n)
{
    LOGFILE(__func__);
#ifdef READ_LIBRARY_ONCE
    if (m[n].op == unknownident)
	fprintf(f, "%5ld %-*.*s %10ld %10ld %c", (long)n, identlength,
	    identlength, (char *)m[n].val, 0L, (long)m[n].nxt,
	    m[n].marked ? 'T' : 'F');
    else
#endif
	fprintf(f, "%5ld %-*.*s %10ld %10ld %c", (long)n, identlength,
	    identlength, standardident_NAMES[m[n].op], m[n].val,
	    (long)m[n].nxt, m[n].marked ? 'T' : 'F');
    if (m[n].op == lib_)
	fprintf(f, "   %-*.*s %4ld", identlength, identlength,
		table[m[n].val].alf, (long)table[m[n].val].adr);
    putc('\n', f);
}

static void writenode(memrange n)
{
    LOGFILE(__func__);
    wn(stdout, n);
    if (writelisting > 0) {
	putc('\t', listing);
	wn(listing, n);
    }
}  /* writenode */
#endif

static void mark(memrange n)
{
    LOGFILE(__func__);
    while (n > 0) {
#if 0
	if (writelisting > 4)
	    writenode(n);
#endif
	if (m[n].op == list_ && !m[n].marked)
	    mark(m[n].val);
	m[n].marked = true;
	n = m[n].nxt;
    }
}  /* mark */

static memrange kons(standardident o, intptr_t v, memrange n)
{
    memrange i;
    long collected;

    LOGFILE(__func__);
    if (!freelist) {
	if (!sentinel)
	    goto einde;
#if 0
	fprintf(stderr, "gc, last_op_executed = %-*.*s\n", identlength,
		identlength, standardident_NAMES[last_op_executed]);
#endif
#if 0
	if (writelisting > 2) {
	    writeident("GC start");
	    writeline();
	}
#endif
	mark(programme);
	mark(s);
	mark(dump);
	/* mark parameters */
	mark(n);
	if (o == list_)
	    mark(v);
#if 0
	if (writelisting > 3) {
	    writeident("finished marking");
	    writeline();
	}
#endif
	collected = 0;
	for (i = firstusernode; i <= MAXMEM; i++) {
	    if (!m[i].marked) {
		m[i].nxt = freelist;
		freelist = i;
		collected++;
	    }
	    m[i].marked = false;
	    if (m[i].nxt == i)
		point('F', "internal error - selfreference");
	}
#if 0
	if (writelisting > 2) {
	    writeinteger(collected);
	    putch(' ');
	    writeident("nodes collected");
	    writeline();
	}
#endif
	if (!freelist)
einde:
	    point('F', "dynamic memory exhausted");
	stat_gc++;
    }
    i = freelist;
    if (o == list_ && v == i)
	point('F', "internal error - selfreference");
    if (i == n)
	point('F', "internal error - circular");
    freelist = m[i].nxt;
    m[i].op = o;
    m[i].val = v;
    m[i].nxt = n;
#if 0
    if (writelisting > 4)
	writenode(i);
#endif
    stat_kons++;
    return i;
}  /* kons */

static void readterm(memrange *);

static void readfactor(memrange *where)
{
    memrange here;

    LOGFILE(__func__);
    switch (sym) {

    case lbrack:
	getsym();
	*where = kons(list_, 0, 0);
	m[*where].marked = true;
	if (sym == lbrack || sym == identifier || sym == charconst ||
		sym == numberconst) {	/* sym == hyphen */
	    readterm(&here);
	    m[*where].val = here;
	}
	break;

    case identifier:
	lookup();
#ifdef READ_LIBRARY_ONCE
	if (id == unknownident)
	    *where = kons(id, (intptr_t)my_strdup(ident), 0);
	else
#endif
	    *where = kons(id, locatn, 0);
	break;

    case charconst:
	*where = kons(char_, num, 0);
	break;

    case numberconst:
	*where = kons(integer_, num, 0);
	break;

#if 0
    case hyphen:
	*where = kons(sub_, sub_, 0);
	break;
#endif

    default:
	point('F', "internal in readfactor");
    }  /* CASE */
    m[*where].marked = true;
}  /* readfactor */

static void readterm(memrange *first)
{   /* readterm */
    /* was forward */
    memrange i;

    LOGFILE(__func__);
    /* this is LL0 */
    readfactor(first);
    i = *first;
    getsym();
    while (sym == lbrack || sym == identifier || sym == charconst ||
		sym == numberconst) {	/* sym == hyphen */
	readfactor(&m[i].nxt);
	i = m[i].nxt;
	getsym();
    }
}  /* readterm */

static void writefactor(memrange n, boolean nl);

static void writeterm(memrange n, boolean nl)
{
    LOGFILE(__func__);
    while (n > 0) {
	writefactor(n, false);
	if (m[n].nxt > 0)
	    putch(' ');
	n = m[n].nxt;
    }
    if (nl)
	writeline();
}  /* writeterm */

static void writefactor(memrange n, boolean nl)
{   /* was forward */
    LOGFILE(__func__);
    if (n > 0) {
	switch (m[n].op) {

	case list_:
	    putch('[');
	    writeterm(m[n].val, false);
	    putch(']');
	    break;

	case boolean_:
	    if (m[n].val == 1)
		writeident("true");
	    else
		writeident("false");
	    break;

	case char_:
	    if (m[n].val == '\n')
		writeline();
	    else
		putch(m[n].val);
	    break;

	case integer_:
	    writeinteger(m[n].val);
	    break;

	case lib_:
	    writeident(table[m[n].val].alf);
	    break;

	case unknownident:
	    writeident((char *)m[n].val);
	    break;

	default:
	    writeident(stdidents[m[n].val].alf);
	    break;
	}  /* CASE */
    }
    if (nl)
	writeline();
}  /* writefactor */

#ifdef READ_LIBRARY_ONCE
static void patchfactor(memrange n);

static void patchterm(memrange n)
{
    LOGFILE(__func__);
    while (n > 0) {
	patchfactor(n);
	n = m[n].nxt;
    }
}  /* patchterm */

static void patchfactor(memrange n)
{   /* was forward */
    LOGFILE(__func__);
    if (n > 0) {
	switch (m[n].op) {

	case list_:
	    patchterm(m[n].val);
	    break;

	case unknownident:
	    my_strncpy(ident, (char *)m[n].val, identlength);
	    ident[identlength] = 0;
	    my_free((char *)m[n].val);
	    lookup();
	    m[n].op = id;
	    m[n].val = locatn;
	    break;
	}  /* CASE */
    }
}  /* patchfactor */
#endif

static void readlibrary(char *str)
{
    int loc;

    LOGFILE(__func__);
#if 0
    if (writelisting > 5)
	fprintf(listing, "first pass through library:\n");
#endif
    newfile(str);
    lastlibloc = 0;
    getsym();
#if 0
    do {
#else
    while (sym != period) {
#endif
#if 0
	if (writelisting > 8)
	    fprintf(listing, "seen : %-*.*s\n", identlength, identlength,
		    ident);
#endif
	if (lastlibloc > 0)
	    if (my_strcmp(ident, table[lastlibloc].alf) <= 0)
		point('F', "bad order in library");
	if (lastlibloc == MAXTABLE)
	    point('F', "too many library symbols");
	my_strcpy(table[++lastlibloc].alf, ident);
#ifdef READ_LIBRARY_ONCE
	loc = lastlibloc;
#else
	do
	    getsym();
	while (sym != semic && sym != period);
	if (sym == semic)
	    getsym();
#if 0
    } while (sym != period);
#else
    }
#endif
    if (writelisting > 5)
	fprintf(listing, "second pass through library:\n");
    newfile(str);
#if 0
    do {
	getsym();
#else
    getsym();
    while (sym != period) {
#endif
	if (sym != identifier)
	    point('F', "pass 2: identifier expected");
	lookup();
	loc = locatn;
#endif
	getsym();
	if (sym != def_equal)
	    point('F', "pass 2: \"==\" expected");
	getsym();
	readterm(&table[loc].adr);
#if 0
	if (writelisting > 8)
	    writeterm(table[loc].adr, true);
#endif
#if 0
    } while (sym != period);
#else
	if (sym != period)
	    getsym();
    }
#endif
    firstusernode = freelist;
#if 0
    if (writelisting > 5)
	fprintf(listing, "firstusernode = %ld,  total memory = %ld\n",
		(long)firstusernode, (long)MAXMEM);
#endif
    cc = ll;
#ifdef READ_LIBRARY_ONCE
    sentinel = lastlibloc + 1;
    lasttable = sentinel;
    for (loc = 1; loc < lasttable; loc++)
	patchterm(table[loc].adr);
    adjustment = -1;
#else
    adjustment = -2;  /* back to file "input" */
#endif
}  /* readlibrary */

static long JL10[8];

static memrange ok(memrange x)
{
    if (x < 1)
	point('F', "null address being referenced");
    return x;
}  /* ok */

static standardident o(memrange x)
{
    return m[ok(x)].op;
}

static long i(memrange x)
{
    if (o(x) == integer_)
	return m[x].val;
    point('R', "integer value required");
    my_longjmp(JL10, 1);
    return 1;
}  /* i */

static memrange l(memrange x)
{
    if (o(x) == list_)
	return m[x].val;
    point('R', "list value required");
    my_longjmp(JL10, 1);
    return 0;
}  /* l */

static memrange n(memrange x)
{
    if (m[ok(x)].nxt >= 0)
	return m[x].nxt;
    point('R', "negative next value");
    my_longjmp(JL10, 1);
    return 0;
}  /* n */

static long v(memrange x)
{
    return m[ok(x)].val;
}

static boolean b(memrange x)
{
    return (boolean)(v(x) > 0);
}

static void binary(standardident o, long v)
{
    s = kons(o, v, n(n(s)));
}

static void joy(memrange nod)
{
    memrange temp1, temp2;

    LOGFILE(__func__);
    while (nod > 0) {  /* WHILE */
#if 0
	if (writelisting > 3) {
	    writeident("joy:");
	    putch(' ');
	    writefactor(nod, true);
	}
	if (writelisting > 4) {
	    writeident("stack:");
	    putch(' ');
	    writeterm(s, true);
	    writeident("dump:");
	    putch(' ');
	    writeterm(dump, true);
	}
	last_op_executed = m[nod].op;
#endif
	switch (m[nod].op) {

	case nothing_:
	case char_:
	case integer_:
	case list_:
	    s = kons(m[nod].op, m[nod].val, s);
	    break;

	case true_:
	case false_:
	    s = kons(boolean_, m[nod].op == true_, s);
	    break;

	case pop_:
	    s = n(s);
	    break;

	case dup_:
	    s = kons(o(s), v(s), s);
	    break;

	case swap_:
	    s = kons(o(n(s)), v(n(s)), kons(o(s), v(s), n(n(s))));
	    break;

	case stack_:
	    s = kons(list_, s, s);
	    break;

	case unstack_:
	    s = l(s);
	    break;

	/* OPERATIONS: */
	case not_:
	    s = kons(boolean_, !b(s), n(s));
	    break;

	case mul_:
	    binary(integer_, i(n(s)) * i(s));
	    break;

	case add_:
	    binary(integer_, i(n(s)) + i(s));
	    break;

	case sub_:
	    binary(integer_, i(n(s)) - i(s));
	    break;

	case div_:
	    binary(integer_, i(n(s)) / i(s));
	    break;

	case and_:
	    binary(boolean_, b(n(s)) & b(s));
	    break;

	case or_:
	    binary(boolean_, b(n(s)) | b(s));
	    break;

	case lss_:
	    if (o(s) == lib_)
		binary(boolean_, my_strcmp(table[v(n(s))].alf, table[v(s)].alf)
			< 0);
	    else
		binary(boolean_, v(n(s)) < v(s));
	    break;

	case eql_:
	    binary(boolean_, v(n(s)) == v(s));
	    break;

	case sametype_:
	    binary(boolean_, o(n(s)) == o(s));
	    break;

	case cons_:
	    if (o(n(s)) == nothing_)
		s = kons(list_, l(s), n(n(s)));
	    else
		s = kons(list_, kons(o(n(s)), v(n(s)), v(s)), n(n(s)));
	    break;

	case uncons_:
	    if (!v(s))
		s = kons(list_, 0, kons(nothing_, nothing_, n(s)));
	    else
		s = kons(list_, n(l(s)), kons(o(l(s)), v(l(s)), n(s)));
	    break;

	case select_:
	    temp1 = l(s);
	    while (o(l(temp1)) != o(n(s)))
		temp1 = n(temp1);
	    s = kons(list_, n(l(temp1)), n(s));
	    break;

	case index_:
	    if (v(n(s)) < 1)
		s = kons(o(l(s)), v(l(s)), n(n(s)));
	    else
		s = kons(o(n(l(s))), v(n(l(s))), n(n(s)));
	    break;

	case body_:
	    s = kons(list_, table[v(s)].adr, n(s));
	    break;

	case put_:
	    writefactor(s, false);
	    s = n(s);
	    break;

	case putch_:
	    putch(v(s));
	    s = n(s);
	    break;

	case get_:
	    getsym();
	    readfactor(&temp1);
	    s = kons(o(temp1), v(temp1), s);
	    break;

	case getch_:
	    getch();
	    s = kons(integer_, my_ch, s);
	    break;

	/* COMBINATORS: */
	case i_:
#ifdef CORRECT_GARBAGE
	    dump = kons(o(s), l(s), dump);
#endif
	    temp1 = s;
	    s = n(s);
	    joy(l(temp1));
#ifdef CORRECT_GARBAGE
	    dump = n(dump);
#endif
	    break;

	case dip_:
	    dump = kons(o(n(s)), v(n(s)), dump);
	    dump = kons(list_, l(s), dump);
	    s = n(n(s));
	    joy(l(dump));
	    dump = n(dump);
	    s = kons(o(dump), v(dump), s);
	    dump = n(dump);
	    break;

	case step_:
	    dump = kons(o(s), l(s), dump);
	    dump = kons(o(n(s)), l(n(s)), dump);
	    temp1 = l(s);
	    temp2 = l(n(s));
	    s = n(n(s));
	    while (temp2 > 0) {
		s = kons(m[temp2].op, m[temp2].val, s);
		joy(temp1);
		temp2 = m[temp2].nxt;
	    }
	    dump = n(n(dump));
	    break;

	case lib_:
	    joy(table[m[nod].val].adr);
	    break;

	default:
	    point('F', "internal error in interpreter");
	}  /* CASE */
	stat_ops++;
	nod = m[nod].nxt;
    }
    stat_calls++;
}  /* joy */

static void writestatistics(long f)
{
    LOGFILE(__func__);
    my_fprint(f, "%lu microseconds CPU to read library\n", stat_lib);
    my_fprint(f, "%lu microseconds CPU to execute\n", end_clock - stat_lib);
    my_fprint(f, "%lu user nodes available\n", MAXMEM - firstusernode + 1L);
    my_fprint(f, "%lu garbage collections\n", stat_gc);
    my_fprint(f, "%lu nodes used\n", stat_kons);
    my_fprint(f, "%lu calls to joy interpreter\n", stat_calls);
    my_fprint(f, "%lu operations executed\n", stat_ops);
}  /* writestatistics */

static void perhapsstatistics(void);

int main(int argc, char *argv[])
{  /* main */
    memrange i;
    int k = 1;

    LOGFILE(__func__);
    start_clock = my_clock();
    initialise();
    my_atexit(perhapsstatistics);
    my_atexit(finalise);
    for (freelist = i = 1; i < MAXMEM; i++)
	m[i].nxt = i + 1;
#if 0
    for (i = 1; i <= MAXMEM; i++) {
	m[i].marked = false;
	m[i].nxt = i + 1;
    }
    freelist = 1;
    m[MAXMEM].nxt = 0;
    writelisting = 0;
    stat_kons = 0;
    stat_gc = 0;
    stat_ops = 0;
    stat_calls = 0;
    sentinel = 0;
    firstusernode = 0;
#endif
    if (argc == 1)
	readlibrary(lib_filename);
    else {
	argc--;
	readlibrary(argv[k++]);
    }
    stat_lib = my_clock() - start_clock;
#if 0
    if (writelisting > 2)
	for (j = 1; j <= lastlibloc; j++) {
	    fprintf(listing, "\"%-*.*s\" :\n", identlength, identlength,
		    table[j].alf);
	    writeterm(table[j].adr, true);
	}
#endif
    sentinel = lastlibloc + 1;
    lasttable = sentinel;
#if 0
    s = 0;
    dump = 0;
    DumpM();
#endif
    if (argc > 1)
	newfile(argv[k]);
    my_setjmp(JL10);
    do {
	getsym();
	if (sym != period) {
#if 0
	    last_op_executed = get_;
#endif
	    programme = 0;
	    readfactor(&programme);
#if 0
	    if (writelisting > 2) {
		writeident("interpreting:");
		writeline();
		writefactor(programme, true);
	    }
#endif
	    if (dump != 0) {
		my_fprint(STDOUT, "dump error: should be empty!\n");
		writeterm(dump, true);
		dump = 0;
	    }
	    outlinelength = 0;
	    joy(m[programme].val);
	    if (outlinelength > 0)
		writeline();
#if 0
	    if (writelisting > 2) {
		writeident("stack:");
		writeline();
		writeterm(s, true);
	    }
#endif
	}  /* IF */
    } while (sym != period);
    my_exit(0);
    return 0;
}

static void perhapsstatistics(void)
{
    LOGFILE(__func__);
    if (statistics > 0) {
	writestatistics(STDERR);
	if (writelisting > 0)
	    writestatistics(listing);
    }
}
