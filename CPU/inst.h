#ifndef __INST_H__
#define __INST_H__

typedef enum {
	I_FORM,
	B_FORM,
	SC_FORM,
	D_FORM,
	DS_FORM,
	// DQ_FORM,
	X_FORM, // or XO_FORM
	// XS_FORM,
	// XO_FORM
	XL_FORM,
	M_FORM,
	MD_FORM
} inst_form;

typedef struct {
	int li;
	int aa;
	int lk;
} i_form_inst;

typedef struct {
	int bo;
	int bi;
	int bd;
	int aa;
	int lk;
} b_form_inst;

typedef struct {
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
	int rt;
	int ra;
	int ds;
	int xo;
} ds_form_inst;

/*
typedef struct {
	int rt;
	int ra;
	int dq;
} dq_form_inst;
*/

typedef struct {
	int rt;
	int ra;
	int rb;
	int xo;
	int rc;
} x_form_inst;

/*
typedef struct {
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
	int rs;
	int ra;
	int rb;
	int mb;
	int me;
	int rc;
} m_form_inst;

typedef struct {
	int rs;
	int ra;
	int sh;
	int mb;
	int xo;
	int rc;
} md_form_inst;

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
		m_form_inst m;
		md_form_inst md;
	} inst;

	word encoding;
	char *source_line;
	inst_form form;
} instruction;

#define OPCD(INST) (INST)->opcd
#define FORM(INST) (INST)->form

#define ENCODING(INST) (INST)->encoding


#define BIN(V, L, R) (((V)>>(32-(R))) & ((1<<((R)-(L)))-1))
#define SET_BIN(V, W, R) ((V) |= (W)<<(32-R))

#define BIN_OPCD(V) (BIN((V), 0, 6))
#define SET_BIN_OPCD(V, W) SET_BIN(V, W, 6)

#define BIN_LI(V) (BIN((V), 6, 30))
#define SET_BIN_LI(V, W) SET_BIN(V, W, 30)
#define BIN_AA(V) (BIN((V), 30, 31))
#define SET_BIN_AA(V, W) SET_BIN(V, W, 31)
#define BIN_LK(V) (BIN((V), 31, 32))
#define SET_BIN_LK(V, W) SET_BIN(V, W, 32)

#define BIN_LEV(V) (BIN((V), 20, 27))
#define SET_BIN_LEV(V, W) SET_BIN(V, W, 27)

#define BIN_RT(V) (BIN((V), 6, 11))
#define SET_BIN_RT(V, W) SET_BIN(V, W, 11)
#define BIN_RA(V) (BIN((V), 11, 16))
#define SET_BIN_RA(V, W) SET_BIN(V, W, 16)
#define BIN_RB(V) (BIN((V), 16, 21))
#define SET_BIN_RB(V, W) SET_BIN(V, W, 21)
#define BIN_X_XO(V) (BIN((V), 21, 31))
#define SET_BIN_X_XO(V, W) SET_BIN(V, W, 31)
#define BIN_RC(V) (BIN_LK(V))
#define SET_BIN_RC(V, W) SET_BIN_LK(V, W)

#define BIN_BT(V) (BIN_RT(V))
#define SET_BIN_BT(V, W) SET_BIN_RT(V, W)
#define BIN_BA(V) (BIN_RA(V))
#define SET_BIN_BA(V, W) SET_BIN_RA(V, W)
#define BIN_BB(V) (BIN_RB(V))
#define SET_BIN_BB(V, W) SET_BIN_RB(V, W)
#define BIN_XL_XO(V) (BIN_X_XO(V))
#define SET_BIN_XL_XO(V, W) SET_BIN_X_XO(V, W)

#define BIN_D(V) (BIN((V), 16, 32))
#define SET_BIN_D(V, W) SET_BIN(V, W, 32)

#define BIN_DS(V) (BIN((V), 16, 30))
#define SET_BIN_DS(V, W) SET_BIN(V, W, 30)
#define BIN_DS_XO(V) (BIN((V), 30, 32))
#define SET_BIN_DS_XO(V, W) SET_BIN(V, W, 32)

#define BIN_BO(V) (BIN_RT(V))
#define SET_BIN_BO(V, W) SET_BIN_RT(V, W)
#define BIN_BI(V) (BIN_RA(V))
#define SET_BIN_BI(V, W) SET_BIN_RA(V, W)
#define BIN_BD(V) (BIN_DS(V))
#define SET_BIN_BD(V, W) SET_BIN_DS(V, W)

#define BIN_RS(V) (BIN_RT(V))
#define SET_BIN_RS(V, W) SET_BIN_RT(V, W)
#define BIN_MB(V) (BIN((V), 21, 26))
#define SET_BIN_MB(V, W) SET_BIN(V, W, 26)
#define BIN_ME(V) (BIN((V), 26, 31))
#define SET_BIN_ME(V, W) SET_BIN(V, W, 31)

#define BIN_MD_MB(V) (BIN((V), 21, 27))
#define SET_BIN_MD_MB(V, W) SET_BIN(V, W, 27)
#define BIN_MD_XO(V) (BIN((V), 27, 30))
#define SET_BIN_MD_XO(V, W) SET_BIN(V, W, 30)
#define BIN_SH2(V) (BIN((V), 21, 26)<<1 | BIN((V), 30, 31))
#define SET_BIN_SH2(V, W) SET_BIN(V, (((W)&0x3e)<<4)|((W)&0x1), 31)


instruction *decode_inst(word val);
void load_i_form_inst(instruction *inst, int *li, int *aa, int *lk);
void load_d_form_inst(instruction *inst, int *rt, int *ra, int *d);
void load_xl_form_inst(instruction *inst, int *bt, int *ba, int *bb, int *xo, int *lk);
void load_sc_form_inst(instruction *inst, int *lev, int *aa);
void load_ds_form_inst(instruction *inst, int *rt, int *ra, int *ds, int *xo);
void load_x_form_inst(instruction *inst, int *rt, int *ra, int *rb, int *xo, int *rc);
void load_b_form_inst(instruction *inst, int *bo, int *bi, int *bd, int *aa, int *lk);
void load_m_form_inst(instruction *inst, int *rs, int *ra, int *rb, int *mb, int *me, int *rc);
void load_md_form_inst(instruction *inst, int *rs, int *ra, int *sh, int *mb, int *xo, int *rc);
word mk_b_op(int opcd, int bo, int bi, int bd, int aa, int lk);
word mk_sc_op(int opcd, int lev, int aa);
word mk_i_op(int opcd, int li, int aa, int lk);
word mk_xl_op(int opcd, int bt, int ba, int bb, int xo, int lk);
word mk_x_op(int opcd, int rt, int ra, int rb, int xo, int rc);
word mk_d_op(int opcd, int rt, int ra, int d);
word mk_ds_op(int opcd, int rt, int ra, int ds, int xo);
word mk_m_op(int opcd, int rs, int ra, int rb, int mb, int me, int rc);
word mk_md_op(int opcd, int rs, int ra, int sh, int mb, int xo, int rc);
void free_inst(instruction *inst);


#endif // __INST_H__
