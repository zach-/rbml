/**
 * RBML assembly language compiler: program code
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "code.h"
#include "symbol.h"
#include "rbmlc.h"

#define CODE_ALLOCATION_STEP	100

RB_GENERATE(symbol_table, symbol, link, symbol_cmp);

/**
 * Allocate code
 */
struct code *
code_alloc()
{
	struct code *c;

	c = calloc(1, sizeof(*c));
	if (c == NULL)
		die("Failed to allocate code: %s", strerror(errno));
	RB_INIT(&c->symbol_table);

	return c;
}

/**
 * Free code
 */
void
code_free(struct code *c)
{
	struct symbol *s, *s_next;

	if (c == NULL)
		return;

	if (c->text != NULL)
		free(c->text);
	RB_FOREACH_SAFE(s, symbol_table, &c->symbol_table, s_next)
		symbol_free(s);
	free(c);
}

/**
 * Lookup symbol
 */
struct symbol *
code_symbol_lookup(struct code *c, const char *name)
{
	return RB_FIND(symbol_table, &c->symbol_table, (struct symbol *) &name);
}

/**
 * Add symbol
 */
struct symbol *
code_symbol_add(struct code *c, const char *name, unsigned line_num)
{
	struct symbol *s;

	s = symbol_alloc(name, line_num, c->size);
	RB_INSERT(symbol_table, &c->symbol_table, s);

	return s;
}

/**
 * Reference a symbol
 */
struct symbol *
code_symbol_ref(struct code *c, const char *name, unsigned line_num)
{
	struct symbol *s;

	s = code_symbol_lookup(c, name);
	if (s == NULL)
		s = code_symbol_add(c, name, 0);

	if (s->line_num == 0)
		symbol_ref(s, line_num, c->size);

	return s;
}

/**
 * Define previously referenced symbol
 */
void
code_symbol_define(struct code *c, struct symbol *s, unsigned line_num)
{
	symbol_define(s, line_num, c->size, c->text);
}

/**
 * Check symbol references
 */
void
code_check_refs(struct code *c, struct parser *p)
{
	struct symbol *s;

	RB_FOREACH(s, symbol_table, &c->symbol_table) {
		symbol_check_refs(s, p);
	}
}

/**
 * Emit code
 */
void
code_emit(struct code *c, rbml_word w)
{
	if (c->allocated <= c->size) {
		c->text = realloc(c->text, (c->allocated + CODE_ALLOCATION_STEP) * sizeof(*c->text));
		if (c->text == NULL)
			die("Failed to allocate memory for program text: %s", strerror(errno));
		c->allocated += CODE_ALLOCATION_STEP;
	}

	c->text[c->size++] = w;
}

/**
 * Write code to file
 */
void
code_write(struct code *c, const char *output_file)
{
	FILE *fp;

	fp = fopen(output_file, "w");
	if (fp == NULL)
		die("Failed to open output file %s: %s", output_file, strerror(errno));
	fwrite(c->text, sizeof(*c->text), c->size, fp);
	if (ferror(fp)) {
		fclose(fp);
		unlink(output_file);
		die("Failed to write output file %s: %s", output_file, strerror(errno));
	}
	fclose(fp);
}
