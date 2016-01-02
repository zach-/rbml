/*
 * MPC-based RBML assembly language parser
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#include "rbmlc.h"
#include "parser.h"
#include "code.h"
#include "symbol.h"
#include "parser_impl.h"

#include "mpc.h"

/**
 * MPC parser data
 */
struct parser_data {
	mpc_parser_t *Program;
	mpc_parser_t *Line;
	mpc_parser_t *Label;
	mpc_parser_t *Instruction;
	mpc_parser_t *Word;
	mpc_parser_t *Inst0;
	mpc_parser_t *Inst1a;
	mpc_parser_t *Inst1r;
	mpc_parser_t *Inst2rr;
	mpc_parser_t *Inst2ra;
	mpc_parser_t *Reg;
	mpc_parser_t *Disk;
	mpc_parser_t *Ident;
	mpc_parser_t *Number;
	mpc_parser_t *Hex;
	mpc_parser_t *Dec;
};

/**
 * Init parser implementation
 */
void
parser_init(struct parser *p)
{
	struct parser_data *d;
	mpc_err_t *err;

	d = p->data = malloc(sizeof(*d));
	d->Program = mpc_new("program");
	d->Line = mpc_new("line");
	d->Label = mpc_new("label");
	d->Instruction = mpc_new("instruction");
	d->Word = mpc_new("word");
	d->Inst0 = mpc_new("inst0");
	d->Inst1a = mpc_new("inst1a");
	d->Inst1r = mpc_new("inst1r");
	d->Inst2rr = mpc_new("inst2rr");
	d->Inst2ra = mpc_new("inst2ra");
	d->Reg = mpc_new("reg");
	d->Disk = mpc_new("disk");
	d->Ident = mpc_new("ident");
	d->Number = mpc_new("number");
	d->Hex = mpc_new("hex");
	d->Dec = mpc_new("dec");
	err = mpca_lang(MPCA_LANG_DEFAULT,
	    "line: /^/ <label>? <instruction>? /$/;\n"
	    "label: <ident> ':';\n"
	    "instruction: (<word>|<inst0>|<inst1a>|<inst1r>|<inst2rr>|<inst2ra>);\n"
	    "word: \"WORD\" <number>;\n"
	    "inst0: (\"END\"|\"HALT\"|\"HERR\");\n"
	    "inst1a: (\"BRAN\"|\"BRGT\"|\"BRLT\"|\"BREQ\"|\"BRGE\"|\"BRLE\"|\"BRER\"|\"BROF\"|\"BRIO\"|\"BRDZ\"|\n"
	       "\"CALL\"|\"CAGT\"|\"CALT\"|\"CAEQ\"|\"CAGE\"|\"CALE\"|\"CAER\"|\"CAOF\"|\"CAIO\"|\"CADZ\")\n"
               "<ident>;\n"
	    "inst1r: (\"COMP\") <reg>;\n"
	    "inst2rr: (\"ADD\"|\"SUB\"|\"DIV\"|\"MULT\"|\"MOD\"|\"MOV\"|\"CJNT\"|\"DJNT\"|\"LSFT\"|\"RSFT\"|\"CMP\") <reg> ',' <reg>;\n"
	    "inst2ra: (\"STO\"|\"LOD\") <reg> ',' <ident>;\n"
	    "reg: /[rR][0-9]+/;\n"
	    "disk: /[dD][0-9]+/;\n"
	    "ident: /[_a-zA-Z][_a-zA-Z0-9]*/;\n"
	    "number: (<hex>|<dec>);\n"
	    "hex: /[+-]?0x[0-9a-fA-F]+/;\n"
	    "dec: /[+-]?[0-9]+/;\n",
	    d->Line,
	    d->Label,
	    d->Instruction,
	    d->Word,
	    d->Inst0,
	    d->Inst1a,
	    d->Inst1r,
	    d->Inst2rr,
	    d->Inst2ra,
	    d->Reg,
	    d->Disk,
	    d->Ident,
	    d->Number,
	    d->Hex,
	    d->Dec,
	    NULL);
	if (err != NULL) {
		mpc_err_print_to(err, stderr);
		mpc_err_delete(err);
		die(NULL);
	}
}

/**
 * Destroy parser implementation
 */
void
parser_destroy(struct parser *p)
{
	struct parser_data *d = p->data;

	mpc_cleanup(1, d->Program);
	mpc_cleanup(1, d->Line);
	mpc_cleanup(1, d->Label);
	mpc_cleanup(1, d->Instruction);
	mpc_cleanup(1, d->Word);
	mpc_cleanup(1, d->Inst0);
	mpc_cleanup(1, d->Inst1a);
	mpc_cleanup(1, d->Inst1r);
	mpc_cleanup(1, d->Inst2rr);
	mpc_cleanup(1, d->Inst2ra);
	mpc_cleanup(1, d->Reg);
	mpc_cleanup(1, d->Disk);
	mpc_cleanup(1, d->Ident);
	mpc_cleanup(1, d->Number);
	mpc_cleanup(1, d->Hex);
	mpc_cleanup(1, d->Dec);
}

/**
 * Read next input file line
 */
static int
parser_read_line(struct parser *p, char *buf, size_t buf_size)
{
	char *s;

	if (fgets(buf, buf_size, p->fp) == NULL)
		return 0;

	p->line_num++;

	/* strip LF */
	s = strrchr(buf, '\n');
	if (s == NULL) {
		if (feof(p->fp)) {
			parser_warning(p, "No newline at end of file");
		} else {
			/* skip to next LF */
			parser_error(p, "Line too long");
			while (fgetc(p->fp) != '\n')
				/* DO NOTHING */;

			buf[0] = '\0';
			return 1;
		}
	} else {
		*s = '\0';
	}

	/* skip comments */
	s = strchr(buf, ';');
	if (s != NULL)
		*s = '\0';
	return 1;
}

/**
 * Parse line
 */
static void
parser_parse_line(struct parser *p, struct code *c, char *line)
{
	int i;
	mpc_result_t r;
	mpc_ast_t *ast;
	struct parser_data *d = p->data;

	/* printf("--> parsing [%s]\n", p->buf); */
	if (!mpc_parse(p->input_file, line, d->Line, &r)) {
		p->num_errors++;
		r.error->state.row = p->line_num - 1;
		mpc_err_print(r.error);
		mpc_err_delete(r.error);
		return;
	}

	/* mpc_ast_print(r.output); */
	ast = r.output;
	for (i = 0; i < ast->children_num; i++) {
		mpc_ast_t *a = ast->children[i];

		if (strcmp(a->tag, "label|>") == 0) {
			struct symbol *s;
			const char *name = a->children[0]->contents;
		
			/* printf("label [%s]\n", name); */
			if ((s = code_symbol_lookup(c, name)) != NULL) {
				parser_error(p, "Symbol `%s' already defined at line %u",
				    name, s->line_num);
				continue;
			}

			code_symbol_add(c, name, p->line_num);
		} else if (strcmp(a->tag, "word|>") == 0) {
			const char *value = a->children[1]->contents;

			/* printf("value [%s]\n", value); */
			code_emit(c, strtol(value, NULL, 0));
		} else if (strcmp(a->tag, "regex") == 0) {
			continue;
		} else if (strcmp(a->tag, "instruction|inst0|string") == 0) {
		} else {
			mpc_ast_print(a);
		}
	}
	mpc_ast_delete(r.output);
}

/**
 * Parse file
 */
unsigned
parser_parse(struct parser *p, struct code *c)
{
	char line[1024];

	while (parser_read_line(p, line, sizeof(line))) {
		parser_parse_line(p, c, line);
	}
	if (ferror(p->fp)) {
		fclose(p->fp);
		die("Failed to read input file %s: %s", p->input_file, strerror(errno));
	}

	return p->num_errors;
}
