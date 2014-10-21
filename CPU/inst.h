#ifndef __INST_H__
#define __INST_H__

typedef enum {
	I_FORM,
	B_FORM,
	SC_FORM,
	D_FORM,
	DS_FORM,
	// DQ_FORM,
	X_FORM,
	// XS_FORM,
	// XO_FORM
	XL_FORM
} inst_form;

typedef struct {
	int opcd; // unused
	int li;
	int aa;
	int lk;
} i_form_inst;

typedef struct {
	int opcd;
	int bo;
	int bi;
	int bd;
	int aa;
	int lk;
} b_form_inst;

typedef struct {
	int opcd;
	int lev;
	int aa;
} sc_form_inst;

typedef struct {
	int opcd;
	int rt;
	int ra;
	int d;
} d_form_inst;

typedef struct {
	int opcd;
	int rt;
	int ra;
	int ds;
	int xo;
} ds_form_inst;

/*
typedef struct {
	int opcd;
	int rt;
	int ra;
	int dq;
} dq_form_inst;
*/

typedef struct {
	int opcd;
	int rt;
	int ra;
	int rb;
	int xo;
	int rc;
} x_form_inst;

/*
typedef struct {
	int opcd;
	int rt;
	int ra;
	int rb;
	int xo;
	int sh;
	int rc;
} xs_form_inst;
*/

/*
typedef struct {
	int opcd;
	int rt;
	int ra;
	int rb;
	int oe;
	int xo;
	int rc;
} xo_form_inst;
*/


typedef struct {
	int bt;
	int ba;
	int bb;
	int xo;
	int lk;
} xl_form_inst;


typedef struct {
	int opcd;
	union {
		i_form_inst i;
		b_form_inst b;
		sc_form_inst sc;
		d_form_inst d;
		ds_form_inst ds;
		// dq_form_inst dq;
		x_form_inst x;
		// xs_form_inst xs;
		// xo_form_inst xo;
		xl_form_inst xl;
	} inst;

	word encoding;
	char *source_line;
	inst_form form;
} instruction;

#define OPCD(INST) (INST)->opcd

#define ENCODING(INST) (INST)->encoding


#define BIN_REG(V, L, R) (((V)>>(32-(R))) & ((1<<((R)-(L)))-1))

#define BIN_OPCD(V) (BIN_REG((V), 0, 6))

#define BIN_LI(V) (BIN_REG((V), 6, 30))
#define BIN_AA(V) (BIN_REG((V), 30, 31))
#define BIN_LK(V) (BIN_REG((V), 31, 32))

#define BIN_LEV(V) (BIN_REG((V), 20, 27))

#define BIN_RT(V) (BIN_REG((V), 6, 11))
#define BIN_RA(V) (BIN_REG((V), 11, 16))
#define BIN_RB(V) (BIN_REG((V), 16, 21))
#define BIN_X_XO(V) (BIN_REG((V), 21, 31))
#define BIN_RC(V) (BIN_LK(V))

#define BIN_BT(V) (BIN_RT(V))
#define BIN_BA(V) (BIN_RA(V))
#define BIN_BB(V) (BIN_RB(V))

#define BIN_D(V) (BIN_REG((V), 16, 32))

#define BIN_DS(V) (BIN_REG((V), 16, 30))
#define BIN_DS_XO(V) (BIN_REG((V), 30, 32))

#define BIN_BO(V) (BIN_RT(V))
#define BIN_BI(V) (BIN_RA(V))
#define BIN_BD(V) (BIN_DS(V))


instruction *decode_inst(word val);
void free_inst(instruction *inst);

#endif // __INST_H__