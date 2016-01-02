/**
 * RBML machine definition
 *
 * @author Zachary Bricker <zbricker@my.harrisburgu.edu>
 */

#ifndef _RBML_H_
#define _RBML_H_

#include <stdint.h>

#ifdef _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

/**
 * RBML word
 */
typedef int32_t rbml_word;

/**
 * Generate RBML instruction
 */
#define RBML_INST(op, arg1, arg2, arg3)		\
	(((op) & 0xff) << 24 | ((arg1) & 0xf) << 20 | ((arg2) & 0xf) << 16 | (arg3) & 0xffff)

/**
 * Get RBML instruction opcode
 */
#define RBML_OPCODE(w)	(((w) >> 24) & 0xff)

/**
 * Get RBML instruction arg1
 */
#define RBML_ARG1(w)	(((w) >> 20) & 0xf)

/**
 * Get RBML instruction arg2
 */
#define RBML_ARG2(w)	(((w) >> 16) & 0xf)

/**
 * Get RBML instruction arg3
 */
#define RBML_ARG3(w)	((w) & 0xffff)

/**
 * Set RBML instruction arg3
 */
#define RBML_SET_ARG3(w, arg3)			\
	do {					\
		(w) &= ~0xffff;			\
		(w) |= (arg3) & 0xffff;		\
	} while (0)

/**
 * Max RBML word integer value
 */
#define RBML_WORD_MIN INT32_MIN

/**
 * Min RBML word integer value
 */
#define RBML_WORD_MAX INT32_MAX

#define OP_STO		0x00
#define OP_STOA		0x01
#define OP_STOJ		0x02

#define OP_LOD		0x03
#define OP_LODA		0x04
#define OP_LODJ		0x05

#define OP_MOV		0x06
#define OP_MOVA		0x07
#define OP_MOVJ		0x08

#define OP_CRDM		0xb0
#define OP_CRDR		0xb1
#define OP_CRDA		0xb2
#define OP_CRDJ		0xb3

#define OP_WRIT		0xb4
#define OP_WRTA		0xb5
#define OP_WRTJ		0xb6

#define OP_FRDM		0xb7
#define OP_FRDR		0xb8
#define OP_FRDA		0xb9
#define OP_FRDJ		0xbA
#define OP_FWRT		0xbb
#define OP_FWTA		0xbc
#define OP_FWTJ		0xbd
#define OP_REPO		0xbe
#define OP_PRES		0xbf
#define OP_OPEN		0xc0
#define OP_CLOS		0xc1

#define OP_ADD		0xa0
#define OP_ADDA		0xa1
#define OP_ADDJ		0xa2

#define OP_SUB		0xa3
#define OP_SUBA		0xa4
#define OP_SUBJ		0xa5

#define OP_DIV		0xa6
#define OP_DIVA		0xa7
#define OP_DIVJ		0xa8

#define OP_MULT		0xa9
#define OP_MLTA		0xaa
#define OP_MLTJ		0xab

#define OP_MOD		0xac
#define OP_MODA		0xad
#define OP_MODJ		0xae

#define OP_CJNT		0x10
#define OP_CJNA		0x11
#define OP_CJNJ		0x12

#define OP_DJNT		0x13
#define OP_DJNA		0x14
#define OP_DJNJ		0x15

#define OP_COMP		0x16
#define OP_COMPA	0x17
#define OP_COMPJ	0x18

#define OP_LSFT		0x19
#define OP_LSFA		0x1a
#define OP_LSFJ		0x1b

#define OP_RSFT		0x1c
#define OP_RSFA		0x1d
#define OP_RSFJ		0x1e

#define OP_CMP		0x20
#define OP_CMPA		0x21
#define OP_CMPJ		0x22
#define OP_CMPM		0x23

#define OP_BRAN		0x30
#define OP_BRGT		0x31
#define OP_BRLT		0x32
#define OP_BREQ		0x33
#define OP_BRGE		0x34
#define OP_BRLE		0x35

#define OP_CALL		0x40
#define OP_CAGT		0x41
#define OP_CALT		0x42
#define OP_CAEQ		0x43
#define OP_CAGE		0x44
#define OP_CALE		0x45
#define OP_END		0x4f

#define OP_HALT		0xff
#define OP_HERR		0xee

#endif /* _RBML_H_ */
