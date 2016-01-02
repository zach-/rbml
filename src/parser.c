/**
 * RBML assembly language: parser
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rbmlc.h"
#include "parser.h"
#include "parser_impl.h"

/**
 * Allocate parser
 */
struct parser *
parser_alloc(const char *input_file)
{
	struct parser *p;

	p = calloc(1, sizeof(*p));
	if (p == NULL)
		die("Failed to allocate parser: %s", strerror(errno));
	p->input_file = strdup(input_file);
	if (p->input_file == NULL)
		die("Failed to allocate parser input file: %s", strerror(errno));
	p->fp= fopen(p->input_file, "r");
	if (p->fp == NULL)
		die("Failed to open input file %s: %s", p->input_file, strerror(errno));

	/* init parser */
	parser_init(p);

	return p;
}

/**
 * Free parser
 */
void
parser_free(struct parser *p)
{
	if (p == NULL)
		return;

	if (p->input_file != NULL)
		free((void *) p->input_file);
	if (p->fp != NULL)
		fclose(p->fp);

	/* destroy parser */
	parser_destroy(p);
}

/**
 * Record parser error
 */
void
parser_error(struct parser *p, const char *format, ...)
{
	va_list ap;

	p->num_errors++;

	va_start(ap, format);
	fprintf(stderr, "%s:%u: Error: ", p->input_file, p->line_num);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);
	va_end(ap);
}

/**
 * Record parser warning
 */
void
parser_warning(struct parser *p, const char *format, ...)
{
	va_list ap;

	p->num_warnings++;

	va_start(ap, format);
	fprintf(stderr, "%s:%u: Warning: ", p->input_file, p->line_num);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);
	va_end(ap);
}
