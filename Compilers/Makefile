CC = gcc -g
LEX = flex
YACC = bison -y
OBJS = main.o lex.yy.o context.o generate.o y.tab.o tree.o out.o #typecheck.o

all: ada

ada: $(OBJS)
	$(CC) -o ada $(OBJS)

scantest: lex.yy.c scantest.c
	$(CC) -o scantest scantest.c

lex.yy.o : lex.yy.c 
lex.yy.c: ada.l
	$(LEX) ada.l

y.tab.o : y.tab.c tree.o
y.tab.c : ada.y
	$(YACC) ada.y
y.tab.h : ada.y
	$(YACC) -d ada.y
	
#typecheck.o : typecheck.c context.c
generate.o : generate.c tree.o out.o
tree.o : tree.c
out.o : out.o
main.o : main.c  tree.o generate.o #typecheck.o

clean:
	-@rm lex.yy.c scantest
	-@rm y.tab.c *.o
	-@rm ada
	-@rm tree.h.gch
	
allclean: clean
	-@rm y.tab.h y.output