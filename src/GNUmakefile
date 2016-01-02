LD=		$(CC)
CFLAGS=		-c
CFLAGS+=	-g
CFLAGS+=	-Wall -Wimplicit-function-declaration -Werror
CFLAGS+=	-I../mpc

YACC=		bison
YFLAGS=		-d
LEX=		flex

VPATH=		../mpc
RBML_SRC=		rbml.c
RBMLC_SRC_COMMON=	rbmlc.c code.c symbol.c parser.c
RBMLC_SRC_PARSER=	rbml_lex.c rbml_parser.c
RBMLC_SRC_PARSER_MPC=	rbml_parser_mpc.c mpc.c

all:	rbml rbmlc rbmlc-mpc

rbml:		$(addsuffix .o, $(basename $(notdir $(RBML_SRC))))
	$(LD) -o $@ $(LDFLAGS) $^

rbmlc:		$(addsuffix .o, $(basename $(notdir $(RBMLC_SRC_COMMON) $(RBMLC_SRC_PARSER))))
	$(LD) -o $@ $(LDFLAGS) $^

rbmlc-mpc:	$(addsuffix .o, $(basename $(notdir $(RBMLC_SRC_COMMON) $(RBMLC_SRC_PARSER_MPC))))
	$(LD) -o $@ $(LDFLAGS) $^

clean:
	rm -f rbml rbmlc rbmlc-mpc *.o *.d rbml_parser.[ch] rbml_lex.c

-include $(addsuffix .d, $(basename $(notdir $(RBML_SRC))))
-include $(addsuffix .d, $(basename $(notdir $(RBMLC_SRC_COMMON) $(RBMLC_SRC_PARSER) $(RBMLC_SRC_PARSER_MPC))))

.SUFFIXES: .d

.c.d:
	$(CC) -MM -MG $(CFLAGS) -o $@ $<

.y.c:
	$(YACC) $(YFLAGS) -o $@ $<

.l.c:
	$(LEX) $(LFLAGS) -o $@ $<
