/**
 * RBML assembly language compiler
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#include <limits.h>
#include <unistd.h>

#include <mpc.h>
#include "parser.h"
#include "code.h"
#include "rbmlc.h"
#include "rbml.h"

static int compile(const char *output_file, const char *input_file);

/**
 * Print message and die
 */
void
die(const char *format, ...)
{
	va_list ap;

	if (format) {
		va_start(ap, format);
		vfprintf(stderr, format, ap);
		fputc('\n', stderr);
		va_end(ap);
	}

	exit(1);
}

/**
 * Print usage and exit
 */
static void
usage(const char *argv0)
{
	const char *program_name;

	if ((program_name = strrchr(argv0, PATH_SEP)) != NULL)
		program_name++;
	else
		program_name = argv0;

	die("Usage: %s [-o <output-file>] <input-file>", program_name);
}

/**
 * Main entry point
 */
int
main(int argc, char *argv[])
{
	int c;
	const char *argv0 = argv[0];
	const char *input_file;
	const char *output_file;
	char buf[PATH_MAX];

	/*
	 * Parse command line arguments
	 */
	while ((c = getopt(argc, argv, "o:h")) != -1) {
		switch (c) {
		case 'o':
			output_file = optarg;
			break;
		case 'h':
		default:
			usage(argv0);
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		usage(argv0);
		/* NOTREACHED */
	}
	input_file = argv[0];
	if (output_file == NULL) {
		char *p;
		char buf2[PATH_MAX];

		/* base name */
		p = strrchr(input_file, PATH_SEP);
		snprintf(buf2, sizeof(buf2), "%s", p != NULL ? p + 1 : input_file);

		/* strip extension */
		if ((p = strrchr(buf2, '.')) != NULL)
			*p = '\0';

		/* make output file name */
		snprintf(buf, sizeof(buf), "%s.rbml", buf2);
		output_file = buf;
	}

	/*
	 * Compile
	 */
	if (!compile(output_file, input_file))
		exit(1);

	exit(0);
}

static int
compile(const char *output_file, const char *input_file)
{
	unsigned num_errors;
	struct code *c;
	struct parser *p;

	/*
	 * parse input file
	 */
	c = code_alloc();

	p = parser_alloc(input_file);
	parser_parse(p, c);
	code_check_refs(c, p);
	num_errors = p->num_errors;
	parser_free(p);

	/*
	 * write output file
	 */
	if (num_errors == 0) {
		code_write(c, output_file);
	} else {
		printf("Total %d errors\n", num_errors);
	}

	code_free(c);

	return num_errors == 0;
}
