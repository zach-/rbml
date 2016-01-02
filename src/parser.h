/**
 * RBML assembly language: parser
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>

struct code;

/**
 * Parser
 */
struct parser {
	const char *input_file;	/**< input file */
	FILE *fp;		/**< input file */

	unsigned line_num;	/**< current line number */
	unsigned num_errors;	/**< number of errors */
	unsigned num_warnings;	/**< number of warnings */

	void *data;		/**< parser data */
};

/**
 * Allocate parser
 */
struct parser *parser_alloc(const char *input_file);

/**
 * Free parser
 */
void parser_free(struct parser *p);

/**
 * Record parser error
 */
void parser_error(struct parser *p, const char *format, ...);

/**
 * Record parser warning
 */
void parser_warning(struct parser *p, const char *format, ...);

/**
 * Parse file
 *
 * @return number of parser errors
 */
unsigned parser_parse(struct parser *p, struct code *c);

#endif /* _PARSER_H_ */
