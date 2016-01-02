%{

/**
 * RBML assembly language parser
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#include "symbol.h"
#include "parser.h"
#include "code.h"
#include "rbml_parser.h"

#define rbml_error(yyloc, parser, code, msg)	parser_error(parser, msg)
int rbml_lex(YYSTYPE *yylval_param, YYLTYPE *yyloc_param, struct parser *parser, void *scanner);

#define scanner parser->data

%}

%pure-parser
%name-prefix "rbml_"
%param 	{struct parser *parser}
%parse-param	{struct code *code}
%lex-param	{void *scanner}
%locations
%error-verbose

%union {
	int opcode;
	int number;
	char *ident;
};

%token WORD
%token<number> REG DISK NUMBER
%token<ident> IDENT
%token<opcode> INST0 INST1A INST1R INST2RR INST2RA INST2NR

%%

program:
	line_list
	;

line_list:
	line
	| line_list '\n' line
	;

line:
    	/* EMPTY */
	| label
	| instruction
	| label instruction
	| error
	;

label:
	IDENT ':' {
		struct symbol *s;
		char *ident = $1;

		/* check if symbol has been seen */
		if ((s = code_symbol_lookup(code, ident)) == NULL) {
			code_symbol_add(code, ident, parser->line_num);
			free(ident);
		} else {
			free(ident);

			/* check if symbol is already defined */
			if (s->line_num != 0) {
				parser_error(parser, "Symbol `%s' already defined at line %u",
				    s->name, s->line_num);
				YYERROR;
			}

			/* symbol was referenced before */
			code_symbol_define(code, s, parser->line_num);
		}
	}
	;

instruction:
	WORD NUMBER { code_emit(code, $2); }
	| INST0 { code_emit(code, RBML_INST($1, 0, 0, 0)); }
	| INST1A IDENT {
		struct symbol *s;
		char *ident = $2;

		s = code_symbol_ref(code, ident, parser->line_num);
		free(ident);
		code_emit(code, RBML_INST($1, 0, 0, s->offset));
	}
	| INST1R REG { code_emit(code, RBML_INST($1, $2, 0, 0)); }
	| INST2RR REG ',' REG { code_emit(code, RBML_INST($1, $2, $4, 0)); }
	| INST2RA REG ',' IDENT {
		struct symbol *s;
		char *ident = $4;

		s = code_symbol_ref(code, ident, parser->line_num);
		free(ident);
		code_emit(code, RBML_INST($1, $2, 0, s->offset));
	}
	| INST2NR NUMBER ',' REG { code_emit(code, RBML_INST($1, $2, $4, 0)); }
	;

%%
