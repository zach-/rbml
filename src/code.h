/**
 * RBML assembly language compiler: program code
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#ifndef _CODE_H_
#define _CODE_H_

#include <sys/types.h>

#include "rbml.h"
#include "tree.h"

struct symbol;
struct parser;

/**
 * Code
 */
struct code {
	rbml_word *text;	/**< program text */
	rbml_word size;		/**< current program size */
	size_t allocated;	/**< allocated program size */

	RB_HEAD(symbol_table, symbol) symbol_table;
				/**< program symbol table */
};

/**
 * Allocate code
 */
struct code *code_alloc();

/**
 * Free code
 */
void code_free(struct code *c);

/**
 * Lookup symbol
 */
struct symbol *code_symbol_lookup(struct code *c, const char *name);

/**
 * Add symbol
 */
struct symbol *code_symbol_add(struct code *c, const char *name, unsigned line_num);

/**
 * Reference a symbol
 */
struct symbol *code_symbol_ref(struct code *c, const char *name, unsigned line_num);

/**
 * Define previously referenced symbol
 */
void code_symbol_define(struct code *c, struct symbol *s, unsigned line_num);

/**
 * Check symbol references
 */
void code_check_refs(struct code *c, struct parser *p);

/**
 * Emit code
 */
void code_emit(struct code *c, rbml_word w);

/**
 * Write code to file
 */
void code_write(struct code *c, const char *output_file);

#endif /* _CODE_H_ */
