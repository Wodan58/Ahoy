#
#   module  : makefile
#   version : 1.3
#   date    : 09/21/20
#
#   42minjoy in assembly
#
#   joy.c is compiled to joy.s
#   joy.s is converted to joy.asm
#   joy.asm is assembled into joy.o
#   joy.o is linked to joy
#   The test program is then executed
#
.SUFFIXES:
.SUFFIXES: .asm .c .y .l .o .s

C = libc.a
CC = gcc

NORMAL = -fpic -Os -ffast-math -Wall -Wextra
EXTRA = -mno-sse -march=native -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-builtin -mno-red-zone -fno-align-functions -fno-align-loops -fno-align-jumps -fno-align-labels -fno-exceptions -fno-ident -fno-tree-vectorize -fomit-frame-pointer

CFLAGS  = $(NORMAL) $(EXTRA)
AFLAGS  = -S -masm=intel
LDFLAGS = -nostdlib -static --build-id=none -L. -lc
ARFLAGS = rvU

################################################################################

dummy: joy 42minjoy.lib tutorial.joy
	./joy 42minjoy.lib tutorial.joy

LIBC = $(C)(ctype.o) $(C)(exit.o) $(C)(fgets.o) $(C)(free.o) $(C)(itoa.o) \
       $(C)(malloc.o) $(C)(print.o) $(C)(string.o) $(C)(start.o) $(C)(setjmp.o)

joy: joy.o $(LIBC)
	ld -o$@ joy.o $(LDFLAGS)

joy.asm: joy.c my_lexer
ctype.asm: ctype.c my_lexer
exit.asm: exit.c my_lexer
fgets.asm: fgets.c my_lexer
free.asm: free.c my_lexer
itoa.asm: itoa.c my_lexer
malloc.asm: malloc.c my_lexer
print.asm: print.c my_lexer
string.asm: string.c my_lexer

my_lexer: my_lexer.o my_parse.o
	$(CC) -o$@ my_lexer.o my_parse.o

my_lexer.c: my_parse.c

joy.asm: joy.s
	cat joy.s
	./my_lexer $<
	mv $<.text $@

################################################################################

.s.asm:
	./my_lexer $<
	mv $<.text $@
	rm $<

.asm.o:
	nasm -o$@ -felf64 -l$*.lst $<

.c.o:
	$(CC) -o$@ $(CFLAGS) -c $<

.c.s:
	$(CC) -o$@ $(CFLAGS) $(AFLAGS) $<

.y.c:
	yacc -o$@ -d $<

.l.c:
	lex -o$@ $<

.s.o:
	gcc -o$@ $<

clean:
	rm -f *.a *.o *.s *.lst my_lexer my_lexer.c my_parse.c my_parse.h joy.asm ctype.asm exit.asm fgets.asm free.asm itoa.asm malloc.asm print.asm string.asm
