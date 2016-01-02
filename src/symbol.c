/**
 * RBML assembly language: symbol (label, variable)
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "rbmlc.h"
#include "symbol.h"
#include "parser.h"

/**
 * Symbol reference
 */
struct symbol_ref {
	unsigned line_num;	/** reference source code line */
	unsigned offset;	/** program text offset */
	SLIST_ENTRY(symbol_ref) link;
};

/**
 * Allocate symbol ref
 */
struct symbol_ref *
symbol_ref_alloc(unsigned line_num, unsigned offset)
{
	struct symbol_ref *ref;

	ref = calloc(1, sizeof(*ref));
	if (ref == NULL)
		die("Failed to allocate forward ref: %s", strerror(errno));
	ref->line_num = line_num;
	ref->offset = offset;

	return ref;
}

/**
 * Free symbol ref
 */
void
symbol_ref_free(struct symbol_ref *ref)
{
	if (ref == NULL)
		return;

	free(ref);
}

/**
 * Allocate symbol
 */
struct symbol *
symbol_alloc(const char *name, unsigned line_num, unsigned offset)
{
	struct symbol *s;
       
	s = calloc(1, sizeof(*s));
	if (s == NULL)
		die("Failed to allocate symbol: %s", strerror(errno));
	s->name = strdup(name);
	if (s->name == NULL)
		die("Failed to allocate symbol name: %s", strerror(errno));
	symbol_define(s, line_num, offset, NULL);

	return s;
}

/**
 * Free symbol
 */
void
symbol_free(struct symbol *s)
{
	struct symbol_ref *ref;

	if (s == NULL)
		return;

	if (s->name != NULL)
		free((void *) s->name);
	while ((ref = SLIST_FIRST(&s->symbol_refs)) != NULL) {
		symbol_ref_free(ref);
		SLIST_REMOVE_HEAD(&s->symbol_refs, link);
	}
	free(s);
}

/**
 * Compare symbols
 */
int
symbol_cmp(struct symbol *a, struct symbol *b)
{
	return strcmp(a->name, b->name);
}

/**
 * Reference symbol
 */
void
symbol_ref(struct symbol *s, unsigned line_num, unsigned offset)
{
	struct symbol_ref *ref;

	ref = symbol_ref_alloc(line_num, offset);
	SLIST_INSERT_HEAD(&s->symbol_refs, ref, link);
}

/**
 * Define referenced symbol
 */
void
symbol_define(struct symbol *s, unsigned line_num, unsigned offset, rbml_word *text)
{
	struct symbol_ref *ref;

	s->line_num = line_num;
	s->offset = offset;

	while ((ref = SLIST_FIRST(&s->symbol_refs)) != NULL) {
		RBML_SET_ARG3(text[ref->offset], s->offset);

		symbol_ref_free(ref);
		SLIST_REMOVE_HEAD(&s->symbol_refs, link);
	}
}

/**
 * Check symbol refs
 */
void
symbol_check_refs(struct symbol *s, struct parser *p)
{
	struct symbol_ref *ref;

	SLIST_FOREACH(ref, &s->symbol_refs, link) {
		unsigned line_num = p->line_num;
		p->line_num = ref->line_num;
		parser_error(p, "Unreferenced symbol `%s'", s->name);
		p->line_num = line_num;
	}
}
