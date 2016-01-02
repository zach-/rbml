/**
 * RBML assembly language: symbol (label, variable)
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "rbml.h"
#include "tree.h"
#include "queue.h"

struct symbol_ref;
struct parser;

/**
 * Symbol
 */
struct symbol {
	/* should be first member */
	const char *name;	/**< symbol name */
	unsigned line_num;	/**< source line where the symbol is defined
				     (0 for forward referenced symbols */

	unsigned offset;	/**< program text offset */

	RB_ENTRY(symbol) link;
	SLIST_HEAD(, symbol_ref) symbol_refs;
};

/**
 * Allocate symbol
 */
struct symbol *symbol_alloc(const char *name, unsigned line_num, unsigned offset);

/**
 * Free symbol
 */
void symbol_free(struct symbol *s);

/**
 * Compare symbols
 */
int symbol_cmp(struct symbol *a, struct symbol *b);

/**
 * Reference symbol
 */
void symbol_ref(struct symbol *s, unsigned line_num, unsigned offset);

/**
 * Define referenced symbol
 */
void symbol_define(struct symbol *s, unsigned line_num, unsigned offset, rbml_word *text);

/**
 * Check symbol refs
 */
void symbol_check_refs(struct symbol *s, struct parser *p);

#endif /* _SYMBOL_H_ */
