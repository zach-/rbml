%{

/**
 * RBML assembly language parser
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#include "rbml.h"
#include "parser.h"
#include "parser_impl.h"
#include "rbml_parser.h"

#define YY_DECL int rbml_lex(YYSTYPE *yylval_param, YYLTYPE *yyloc, struct parser *parser, yyscan_t yyscanner)
#define YY_NO_INPUT
#define YY_USER_ACTION rbml_get_extra(yyscanner)->line_num = yylineno;

#define YY_EXTRA_TYPE struct parser *
#define YY_INPUT(buf, result, max_size) 	result = fread(buf, 1, max_size, rbml_get_extra(yyscanner)->fp)

#define OPCODE(_opcode, _type)	do { yylval->opcode = _opcode; return _type; } while (0)

%}

%option case-insensitive
%option batch
%option bison-bridge
%option reentrant
%option prefix="rbml_"
%option noyywrap
%option yylineno

ID	(0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15)
NUMBER	[+-]?0x[0-9A-F]+|[0-9]+
IDENT	[_A-Z][_A-Z0-9]*

%%

END		OPCODE(OP_END, INST0);
HALT		OPCODE(OP_HALT, INST0);
HERR		OPCODE(OP_HERR, INST0);

	/* instructions taking 1 address */
BRAN		OPCODE(OP_BRAN, INST1A);
BRGT		OPCODE(OP_BRGT, INST1A);
BRLT		OPCODE(OP_BRLT, INST1A);
BREQ		OPCODE(OP_BREQ, INST1A);
BRGE		OPCODE(OP_BRGE, INST1A);
BRLE		OPCODE(OP_BRLE, INST1A);

CALL		OPCODE(OP_CALL, INST1A);
CAGT		OPCODE(OP_CAGT, INST1A);
CALT		OPCODE(OP_CALT, INST1A);
CAEQ		OPCODE(OP_CAEQ, INST1A);
CAGE		OPCODE(OP_CAGE, INST1A);
CALE		OPCODE(OP_CALE, INST1A);

	/* instructions taking 1 register */
COMP		OPCODE(OP_COMP, INST1R);

	/* instructions taking 2 registers */
ADD		OPCODE(OP_ADD, INST2RR);
SUB		OPCODE(OP_SUB, INST2RR);
DIV		OPCODE(OP_DIV, INST2RR);
MULT		OPCODE(OP_MULT, INST2RR);
MOD		OPCODE(OP_MOD, INST2RR);
CJNT		OPCODE(OP_CJNT, INST2RR);
DJNT		OPCODE(OP_DJNT, INST2RR);
CMP		OPCODE(OP_CMP, INST2RR);
LSFT		OPCODE(OP_LSFT, INST2RR);
RSFT		OPCODE(OP_RSFT, INST2RR);
MOV		OPCODE(OP_MOV, INST2RR);

	/* instructions taking register and address */
STO		OPCODE(OP_STO, INST2RA);
LOD		OPCODE(OP_LOD, INST2RA);

	/* instructions taking register and number */
CRDR		OPCODE(OP_CRDR, INST2NR);
WRIT		OPCODE(OP_WRIT, INST2NR);
MOVA		OPCODE(OP_MOVA, INST2NR);

	/* word */
WORD		return WORD;

	/* register */
R{ID}		{ yylval->number = atoi(yytext + 1); return REG; }

	/* disk */
D{ID}		{ yylval->number = atoi(yytext + 1); return DISK; }

	/* number */
{NUMBER}	{ yylval->number = strtol(yytext, NULL, 0); return NUMBER; }

	/* identifier */
{IDENT}		{ yylval->ident = strdup(yytext); return IDENT; }

	/* control characters */
[:,\n]		return *yytext;

	/* skip comments */
;.*		;	/* skip comments */

	/* skip whitespace */
[ \t]		;

.		parser_error(parser, "Unexpected character");

%%

void _unused()
{
	yyunput(0, NULL, NULL);
}

/**
 * Init parser implementation
 */
void
parser_init(struct parser *p)
{
	rbml_lex_init_extra(p, &p->data);
}

/**
 * Destroy parser implementation
 */
void
parser_destroy(struct parser *p)
{
	rbml_lex_destroy(p->data);
}

/**
 * Parse file
 */
unsigned
parser_parse(struct parser *p, struct code *c)
{
	rbml_parse(p, c);
	return p->num_errors;
}
