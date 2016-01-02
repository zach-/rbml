//Rumbaugh-Bricker Machine Language Emulator
//copyright: 2015, Douglas Rumbaugh. All rights reserved.
//
//This is an emulator for a chipset capable of executing RBML.
//It represents a machine with a word length of 32 bits, 8 registers
//and an arbitrarily sized main memory. It takes advantage of C functions
//to enable file, display, and arthimatic operations. In a true machine
//this functionality would all be coded in RBML as subroutines.
//

#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rbml.h"

#define countof(a)	(sizeof(a) / sizeof((a)[0]))

//Machine Registers.
rbml_word accumulator;
rbml_word jump;
rbml_word reg[16];

rbml_word instructionRegister;
int instructionCounter;
//END Machine Registers

//Main Memory
rbml_word *memory;
rbml_word *callStack;

int callStackCounter;
size_t SIZE_OF_MMEM = 1024;
size_t SIZE_OF_CALLSTACK = 100;
//END Main Memory

//Machine flags
//Comparision Flags
rbml_word less;
rbml_word equal;
rbml_word greater;

//Error Flags
rbml_word overflow;
rbml_word ioerr;
rbml_word divzero;
//END Machine Flags

//Sequential Access Disks
FILE *disk[16];
//END SAD

int debug;

static void initInstructions();
static void evaluateInstruction();

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

	die("Usage: %s [-d] [-m <memory-size>] <program-file>\n"
	    "\n"
	    "-d			- debug\n"
	    "-m <memory-size>	- specify memory size (number of words)", program_name);
}

/**
 * Dump registers
 */
static void
dump_registers()
{
	int i;

	for (i = 0; i < countof(reg) / 2; i++) {
		fprintf(stderr, "r%d:\t0x%08x\tr%d:\t0x%08x\n",
		    i, reg[i], i + (int) countof(reg) / 2, reg[i + countof(reg) / 2]);
	}
	fprintf(stderr, "acc:\t0x%08x\tjump:\t0x%08x\n", accumulator, jump);
}

int
main(int argc, char *argv[])
{
	int c;
	const char *argv0 = argv[0];

	struct stat sb;
	const char *program_file;
	FILE *fp;
	rbml_word program_size;

	/*
	 * parse command line arguments
	 */
	while ((c = getopt(argc, argv, "dm:h")) != -1) {
		switch (c) {
		case 'd':
			debug = 1;
			break;

		case 'm':
			SIZE_OF_MMEM = atoi(optarg);
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
	program_file = argv[0];

	// initialize call stack
	callStack = calloc(SIZE_OF_CALLSTACK, sizeof(*callStack));
	callStackCounter = 0;

	// initialize main memory
	memory = calloc(SIZE_OF_MMEM, sizeof(*memory));

	if (stat(program_file, &sb) < 0) {
		fprintf(stderr, "Failed to stat program %s: %s\n", program_file, strerror(errno));
		exit(1);
	}
	program_size = sb.st_size;
	if ((program_size % sizeof(rbml_word)) != 0) {
		fprintf(stderr, "Corrupted program %s: Not multiple of RBML word (%d bytes)\n",
		    program_file, (int) sizeof(rbml_word));
		exit(1);
	}

	// read program into main memory from file
	fp = fopen(program_file, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open program %s: %s\n", program_file, strerror(errno));
		exit(1);
	}
 
	// Program is loaded into main memory. If the length of the program file exceeds
	// that of main memory, only that which can fit is loaded. Note that this will
	// result in undefined, potentially hazardous, behavior during program execution.
	if (program_size < SIZE_OF_MMEM * sizeof(*memory))
		fread(memory, 1, program_size, fp);
	else
		fread(memory, sizeof(*memory), SIZE_OF_MMEM, fp);
	fclose(fp);
	// end read program into main memory from file

	initInstructions();
	while (1) {
		instructionRegister = memory[instructionCounter];
		if (debug)
			fprintf(stderr, "--> 0x%08x: %08x\n", instructionCounter, instructionRegister);
		evaluateInstruction();
		if (debug)
			dump_registers();

		instructionCounter++;
		if (instructionCounter >= SIZE_OF_MMEM) {
			fprintf(stderr, "Instruction counter out of memory region\n");
			break;
		}
	}

	free(memory);
	free(callStack);
}

// Instructions -- these function simulate each CPU instruction.
typedef void (*opcode_fn)(int arg1, int arg2, int arg3);
#define OPCODE(name) static void name(int arg1, int arg2, int arg3)

OPCODE(sto) { memory[arg3] = reg[arg1]; }
OPCODE(stoa) { memory[arg3] = accumulator; }
OPCODE(stoj) { memory[arg3] = jump; }

OPCODE(lod) { reg[arg1] = memory[arg3]; }
OPCODE(loda) { accumulator = memory[arg3]; }
OPCODE(lodj) { jump = memory[arg3]; }

OPCODE(mov) { reg[arg2] = reg[arg1]; }
OPCODE(mova) { if (arg1) reg[arg2] = accumulator; else accumulator = reg[arg2]; }
OPCODE(movj) { if (arg1) reg[arg2] = jump; else jump = reg[arg2]; }

// Generic method for writing characters to console
static void
cwrite(int fcontrol, rbml_word w)
{
	if (fcontrol) {
		int i;
		char *s = (char *) &w;

		for (i = sizeof(w) - 1; i >= 0; i--)
			putchar(s[i]);
	} else {
		printf("%d\n", w);
	}
}

OPCODE(writ) { cwrite(arg1, reg[arg2]); }
OPCODE(wrta) { cwrite(arg1, accumulator); }
OPCODE(wrtj) { cwrite(arg1, jump); }

// General read function--called by all console input instructions
// The fcontrol parameter controls the format of the output.
//	If fcontrol is high, then the characters will be kept in ASCII
//	If fcontrol is low, then the characters will be converted to 
//	numeric. 
//	This is important mainly for differentiating between input that is
//	intended to be kept in character format, and input that should be 
//	converted to a numeric format, when reading in numbers.
static rbml_word
cread(fcontrol)
{
	rbml_word w;

	if (fcontrol) {
		fgets((char *) &w, sizeof(w), stdin);
	} else {
		scanf("%d", &w);
	}

	return w;
}

OPCODE(crdm) { memory[arg3] = cread(arg1); }
OPCODE(crda) { accumulator = cread(arg1); }
OPCODE(crdj) { jump = cread(arg1); }
OPCODE(crdr) { reg[arg2] = cread(arg1); }

OPCODE(open) {
	char name[16];
	snprintf(name, sizeof(name), "disk%d.rbdi", arg1);

	ioerr = 0;
	disk[arg1] = fopen(name, "r+");
	if (disk[arg1] == NULL)
		ioerr = 1;
}
OPCODE(clos) {
	fclose(disk[arg1]);
	disk[arg1] = NULL;
}
OPCODE(frdm) { fprintf(stderr, "Instruction not implemented\n"); }
OPCODE(frdr) { fprintf(stderr, "Instruction not implemented\n"); }
OPCODE(frda) { fprintf(stderr, "Instruction not implemented\n"); }
OPCODE(frdj) { fprintf(stderr, "Instruction not implemented\n"); }
OPCODE(fwrt) { fprintf(stderr, "Instruction not implemented\n"); }
OPCODE(fwta) { fprintf(stderr, "Instruction not implemented\n"); }
OPCODE(fwtj) { fprintf(stderr, "Instruction not implemented\n"); }
OPCODE(repo) { fprintf(stderr, "Instruction not implemented\n"); }
OPCODE(pres) { fprintf(stderr, "Instruction not implemented\n"); }

static rbml_word
addition(rbml_word a, rbml_word b)
{
	overflow = 0;
	if ((a > 0 && b > RBML_WORD_MAX - a) || (a < 0 && b < RBML_WORD_MIN - a))
		overflow = 1;

	return a + b;
}

OPCODE(add) { accumulator = addition(reg[arg1], reg[arg2]); }
OPCODE(adda) { accumulator = addition(accumulator, reg[arg1]); }
OPCODE(addj) { accumulator = addition(jump, reg[arg1]); }

// Generic subtraction method, with overflow capture.
static rbml_word
subtraction(rbml_word a, rbml_word b)
{
	overflow = 0;
	if ((a > 0 && b > RBML_WORD_MAX - a) || (a < 0 && b < RBML_WORD_MIN - a))
		overflow = 1;

	return a - b;
}

OPCODE(sub) { accumulator = subtraction(reg[arg1], reg[arg2]); }
OPCODE(suba) { accumulator = subtraction(accumulator, reg[arg1]); }
OPCODE(subj) { accumulator = subtraction(jump, reg[arg1]); }

static rbml_word
multiplication(rbml_word a, rbml_word b)
{
	overflow = 0;
	if (a == 0 || b == 0)
		return 0;
	if ((a > 0 && b > RBML_WORD_MAX / a) || (a < 0 && b < RBML_WORD_MIN / a))
		overflow = 1;

	return a * b;
}

OPCODE(mult) { accumulator = multiplication(reg[arg1], reg[arg2]); }
OPCODE(mlta) { accumulator = multiplication(accumulator, reg[arg1]); }
OPCODE(mltj) { accumulator = multiplication(jump, reg[arg1]); }

static rbml_word
division(rbml_word a, rbml_word b)
{
	if (b == 0) {
		divzero = 1;
		return 0;
	}

	divzero = 0;
	return a / b;
}

OPCODE(div_) { accumulator = division(reg[arg1], reg[arg2]); }
OPCODE(diva) { accumulator = division(accumulator, reg[arg1]); }
OPCODE(divj) { accumulator = division(jump, reg[arg1]); }

static rbml_word
modulus(rbml_word a, rbml_word b)
{
	if (b == 0) {
		divzero = 1;
		return 0;
	}

	divzero = 0;
	return a % b;
}

OPCODE(mod) { accumulator = modulus(reg[arg1], reg[arg2]); }
OPCODE(moda) { accumulator = modulus(accumulator, reg[arg1]); }
OPCODE(modj) { accumulator = modulus(jump, reg[arg1]); }

OPCODE(cjnt) { accumulator = reg[arg1] & reg[arg2]; }
OPCODE(cjna) { accumulator = accumulator & reg[arg1]; }
OPCODE(cjnj) { accumulator = jump & reg[arg1]; }

OPCODE(djnt) { accumulator = reg[arg1] | reg[arg2]; }
OPCODE(djna) { accumulator = accumulator | reg[arg1]; }
OPCODE(djnj) { accumulator = jump | reg[arg1]; }

OPCODE(comp) { accumulator = ~reg[arg1]; }
OPCODE(compa) { accumulator = ~accumulator; }
OPCODE(compj) { accumulator = ~jump; }

OPCODE(lsft) { accumulator = reg[arg1] << reg[arg2]; }
OPCODE(lsfa) { accumulator = accumulator << reg[arg1]; }
OPCODE(lsfj) { accumulator = jump << reg[arg1]; }

OPCODE(rsft) { accumulator = reg[arg1] >> reg[arg2]; }
OPCODE(rsfa) { accumulator = accumulator >> reg[arg1]; }
OPCODE(rsfj) { accumulator = jump >> reg[arg1]; }

static void
compare(rbml_word a, rbml_word b)
{
	less = a < b;
	equal = a == b;
	greater = a > b;
}

OPCODE(cmp) { compare(reg[arg1], reg[arg2]); }
OPCODE(cmpa) { compare(accumulator, reg[arg1]); }
OPCODE(cmpj) { compare(jump, reg[arg1]); }
OPCODE(cmpm) { compare(reg[arg1], memory[arg3]); }

OPCODE(bran) { instructionCounter = arg3 - 1; }
OPCODE(brgt) { if (greater) bran(arg1, arg2, arg3); }
OPCODE(brlt) { if (less) bran(arg1, arg2, arg3); }
OPCODE(breq) { if (equal) bran(arg1, arg2, arg3); }
OPCODE(brge) { if (greater || equal) bran(arg1, arg2, arg3); }
OPCODE(brle) { if (greater || less) bran(arg1, arg2, arg3); }

OPCODE(call) {
	if (jump == 0)
		jump = instructionCounter;
	else {
 		 *(callStack + callStackCounter) = jump;
		callStackCounter = callStackCounter + 1;
		jump = instructionCounter;
	}
	instructionCounter = arg3 - 1;
}
OPCODE(cagt) { if (greater) call(arg1, arg2, arg3); }
OPCODE(calt) { if (less) call(arg1, arg2, arg3); }
OPCODE(caeq) { if (equal) call(arg1, arg2, arg3); }
OPCODE(cage) { if (greater || equal) call(arg1, arg2, arg3); }
OPCODE(cale) { if (less || equal) call(arg1, arg2, arg3); }
OPCODE(end) {
	instructionCounter = jump;
	if (callStackCounter == 0) {
		jump = callStack[0];
		callStack[0] = 0;
	} else {
		jump = callStack[--callStackCounter];
	}
}

OPCODE(halt) { exit(0); }
OPCODE(herr) { exit(2); }

struct opcode_def {
	uint8_t opcode;
	opcode_fn op;
};

static struct opcode_def opcode_defs[] = {
	{ OP_STO,	sto },
	{ OP_STOA,	stoa },
	{ OP_STOJ,	stoj },

	{ OP_LOD,	lod },
	{ OP_LODA,	loda },
	{ OP_LODJ,	lodj },

	{ OP_MOV,	mov },
	{ OP_MOVA,	mova },
	{ OP_MOVJ,	movj },

	{ OP_CRDM,	crdm },
	{ OP_CRDR,	crdr },
	{ OP_CRDA,	crda },
	{ OP_CRDJ,	crdj },

	{ OP_WRIT,	writ },
	{ OP_WRTA,	wrta },
	{ OP_WRTJ,	wrtj },

	{ OP_FRDM,	frdm },
	{ OP_FRDR,	frdr },
	{ OP_FRDA,	frda },
	{ OP_FRDJ,	frdj },
	{ OP_FWRT,	fwrt },
	{ OP_FWTA,	fwta },
	{ OP_FWTJ,	fwtj },
	{ OP_REPO,	repo },
	{ OP_PRES,	pres },
	{ OP_OPEN,	open },
	{ OP_CLOS,	clos },

	{ OP_ADD,	add },
	{ OP_ADDA,	adda },
	{ OP_ADDJ,	addj },

	{ OP_SUB,	sub },
	{ OP_SUBA,	suba },
	{ OP_SUBJ,	subj },

	{ OP_DIV,	div_ },
	{ OP_DIVA,	diva },
	{ OP_DIVJ,	divj },

	{ OP_MULT,	mult },
	{ OP_MLTA,	mlta },
	{ OP_MLTJ,	mltj },

	{ OP_MOD,	mod },
	{ OP_MODA,	moda },
	{ OP_MODJ,	modj },

	{ OP_CJNT,	cjnt },
	{ OP_CJNA,	cjna },
	{ OP_CJNJ,	cjnj },

	{ OP_DJNT,	djnt },
	{ OP_DJNA,	djna },
	{ OP_DJNJ,	djnj },

	{ OP_COMP,	comp },
	{ OP_COMPA,	compa },
	{ OP_COMPJ,	compj },

	{ OP_LSFT,	lsft },
	{ OP_LSFA,	lsfa },
	{ OP_LSFJ,	lsfj },

	{ OP_RSFT,	rsft },
	{ OP_RSFA,	rsfa },
	{ OP_RSFJ,	rsfj },

	{ OP_CMP,	cmp },
	{ OP_CMPA,	cmpa },
	{ OP_CMPJ,	cmpj },
	{ OP_CMPM,	cmpm },

	{ OP_BRAN,	bran },
	{ OP_BRGT,	brgt },
	{ OP_BRLT,	brlt },
	{ OP_BREQ,	breq },
	{ OP_BRGE,	brge },
	{ OP_BRLE,	brle },

	{ OP_CALL,	call },
	{ OP_CAGT,	cagt },
	{ OP_CALT,	calt },
	{ OP_CAEQ,	caeq },
	{ OP_CAGE,	cage },
	{ OP_CALE,	cale },
	{ OP_END,	end },

	{ OP_HALT,	halt },
	{ OP_HERR,	herr },

	{ 0,		NULL },
};

static opcode_fn opcodes[256];

static void
initInstructions()
{
	struct opcode_def *d = opcode_defs;

	for (d = opcode_defs; d->op != NULL; d++)
		opcodes[d->opcode] = d->op;
}

static void
evaluateInstruction()
{
	opcode_fn op;
	rbml_word w = instructionRegister;
	uint8_t opcode = RBML_OPCODE(w);

	op = opcodes[opcode];
	if (op == NULL) {
		fprintf(stderr, "Unknown opcode %x at %x\n", opcode, instructionCounter);
		return;
	}

	op(RBML_ARG1(w), RBML_ARG2(w), RBML_ARG3(w));
}
