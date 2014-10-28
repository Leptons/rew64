#include "rew64.h"
#include "utils.h"
#include "reg.h"
#include "inst.h"
#include "mem.h"
#include "op.h"

static instruction *st_b_form_inst(instruction *inst, word val);
static instruction *st_sc_form_inst(instruction *inst, word val);
static instruction *st_i_form_inst(instruction *inst, word val);
static instruction *st_xl_form_inst(instruction *inst, word val);
static instruction *st_x_form_inst(instruction *inst, word val);
static instruction *st_d_form_inst(instruction *inst, word val);
static instruction *st_ds_form_inst(instruction *inst, word val);
static instruction *st_m_form_inst(instruction *inst, word val);
static instruction *st_md_form_inst(instruction *inst, word val);
static instruction *st_bad_inst(instruction *inst, word val);


instruction *decode_inst(word val){
	int opcd = BIN_OPCD(val);
	instruction *inst = zmalloc(sizeof(instruction));
	inst->opcd = opcd;
	inst->encoding = val;

	if(opcd < D_FORM_INST_R1 || (D_FORM_INST_L2 <= opcd && opcd < D_FORM_INST_R2)){ // D_FORM
		return st_d_form_inst(inst, val);
	} else if(opcd == B_FORM_INST){ // B_FORM
		return st_b_form_inst(inst, val);
	} else if(opcd == SC_FORM_INST){ // SC_FORM
		return st_sc_form_inst(inst, val);
	} else if(opcd == I_FORM_INST){ // I_FORM
		return st_i_form_inst(inst, val);
	} else if(opcd == XL_FORM_INST){ // XL_FORM
		return st_xl_form_inst(inst, val);
	} else if(opcd == X_FORM_INST){ // X_FORM
		return st_x_form_inst(inst, val);
	} else if(DS_FORM_INST_L <= opcd && opcd < DS_FORM_INST_R){ // DS_FORM
		return st_ds_form_inst(inst, val);
	} else if(M_FORM_INST_L <= opcd && opcd < M_FORM_INST_R){ // M_FORM
		return st_m_form_inst(inst, val);
	} else if(opcd == MD_FORM_INST){ // MD_FORM
		return st_md_form_inst(inst, val);
	} else {
		inst->opcd = 0;
		return st_bad_inst(inst, val);
	}
}

word mk_b_op(int opcd, int bo, int bi, int bd, int aa, int lk){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_BO(op, bo);
    SET_BIN_BI(op, bi);
    SET_BIN_BD(op, bd);
    SET_BIN_AA(op, aa);
    SET_BIN_LK(op, lk);

	return op;
}

word mk_sc_op(int opcd, int lev, int aa){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_LEV(op, lev);
	SET_BIN_AA(op, aa);

	return op;
}

word mk_i_op(int opcd, int li, int aa, int lk){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_LI(op, li);
	SET_BIN_AA(op, aa);
	SET_BIN_LK(op, lk);

	return op;
}

word mk_xl_op(int opcd, int bt, int ba, int bb, int xo, int lk){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_BT(op, bt);
	SET_BIN_BA(op, ba);
	SET_BIN_BB(op, bb);
	SET_BIN_XL_XO(op, xo);
	SET_BIN_LK(op, lk);

	return op;
}

word mk_x_op(int opcd, int rt, int ra, int rb, int xo, int rc){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_RT(op, rt);
	SET_BIN_RA(op, ra);
	SET_BIN_RB(op, rb);
	SET_BIN_X_XO(op, xo);
	SET_BIN_RC(op, rc);

	return op;
}

word mk_d_op(int opcd, int rt, int ra, int d){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_RT(op, rt);
	SET_BIN_RA(op, ra);
	SET_BIN_D(op, d);

	return op;
}

word mk_ds_op(int opcd, int rt, int ra, int ds, int xo){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_RT(op, rt);
	SET_BIN_RA(op, ra);
	SET_BIN_DS(op, ds);
	SET_BIN_DS_XO(op, xo);

	return op;
}

word mk_m_op(int opcd, int rs, int ra, int rb, int mb, int me, int rc){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_RS(op, rs);
	SET_BIN_RA(op, ra);
	SET_BIN_RB(op, rb);
	SET_BIN_MB(op, mb);
	SET_BIN_ME(op, me);
	SET_BIN_RC(op, rc);

	return op;
}

word mk_md_op(int opcd, int rs, int ra, int sh, int mb, int xo, int rc){
	word op = 0;

	SET_BIN_OPCD(op, opcd);
	SET_BIN_RS(op, rs);
	SET_BIN_RA(op, ra);
	SET_BIN_SH2(op, sh);
	SET_BIN_MD_MB(op, mb);
	SET_BIN_MD_XO(op, xo);
	SET_BIN_RC(op, rc);

	return op;
}

static instruction *st_b_form_inst(instruction *inst, word val){
	inst->inst.b.bo = BIN_BO(val);
	inst->inst.b.bi = BIN_BI(val);
	inst->inst.b.bd = BIN_BD(val);
	inst->inst.b.aa = BIN_AA(val);
	inst->inst.b.lk = BIN_LK(val);

	inst->form = B_FORM;

	return inst;
}

static instruction *st_sc_form_inst(instruction *inst, word val){
	inst->inst.sc.lev = BIN_LEV(val);
	inst->inst.sc.aa = BIN_AA(val);

	inst->form = SC_FORM;
	
	return inst;
}

static instruction *st_i_form_inst(instruction *inst, word val){
	inst->inst.i.li = BIN_LI(val);
	inst->inst.i.aa = BIN_AA(val);
	inst->inst.i.lk = BIN_LK(val);

	inst->form = I_FORM;
	
	return inst;
}

static instruction *st_xl_form_inst(instruction *inst, word val){
	inst->inst.xl.bt = BIN_BT(val);
	inst->inst.xl.ba = BIN_BA(val);
	inst->inst.xl.bb = BIN_BB(val);
	inst->inst.xl.xo = BIN_XL_XO(val);
	inst->inst.xl.lk = BIN_LK(val);

	inst->form = XL_FORM;

	return inst;
}

static instruction *st_x_form_inst(instruction *inst, word val){
	inst->inst.x.rt = BIN_RT(val);
	inst->inst.x.ra = BIN_RA(val);
	inst->inst.x.rb = BIN_RB(val);
	inst->inst.x.xo = BIN_X_XO(val);
	inst->inst.x.rc = BIN_RC(val);

	inst->form = X_FORM;

	return inst;
}

static instruction *st_d_form_inst(instruction *inst, word val){
	inst->inst.d.rt = BIN_RT(val);
	inst->inst.d.ra = BIN_RA(val);
	inst->inst.d.d = BIN_D(val);

	inst->form = D_FORM;

	return inst;
}

static instruction *st_ds_form_inst(instruction *inst, word val){
	inst->inst.ds.rt = BIN_RT(val);
	inst->inst.ds.ra = BIN_RA(val);
	inst->inst.ds.ds = BIN_DS(val);
	inst->inst.ds.xo = BIN_DS_XO(val);

	inst->form = DS_FORM;
	return inst;
}

static instruction *st_m_form_inst(instruction *inst, word val){
	inst->inst.m.rs = BIN_RS(val);
	inst->inst.m.ra = BIN_RA(val);
	inst->inst.m.rb = BIN_RB(val);
	inst->inst.m.mb = BIN_MB(val);
	inst->inst.m.me = BIN_ME(val);
	inst->inst.m.rc = BIN_RC(val);

	inst->form = M_FORM;
	return inst;
}

static instruction *st_md_form_inst(instruction *inst, word val){
	inst->inst.md.rs = BIN_RS(val);
	inst->inst.md.ra = BIN_RA(val);
	inst->inst.md.sh = BIN_SH2(val);
	inst->inst.md.mb = BIN_MD_MB(val);
	inst->inst.md.xo = BIN_MD_XO(val);
	inst->inst.md.rc = BIN_RC(val);

	inst->form = MD_FORM;
	return inst;
}

static instruction *st_bad_inst(instruction *inst, word val){
	fprintf(stderr, "Cannot create instruction\n");
	return inst;
}

void load_i_form_inst(instruction *inst, int *li, int *aa, int *lk){
	*li = inst->inst.i.li;
	*aa = inst->inst.i.aa;
	*lk = inst->inst.i.lk;
}

void load_d_form_inst(instruction *inst, int *rt, int *ra, int *d){
	*rt = inst->inst.d.rt;
	*ra = inst->inst.d.ra;
	*d = inst->inst.d.d;
}

void load_xl_form_inst(instruction *inst, int *bt, int *ba, int *bb, int *xo, int *lk){
	*bt = inst->inst.xl.bt;
	*ba = inst->inst.xl.ba;
	*bb = inst->inst.xl.bb;
	*xo = inst->inst.xl.xo;
	*lk = inst->inst.xl.lk;
}

void load_sc_form_inst(instruction *inst, int *lev, int *aa){
	*lev = inst->inst.sc.lev;
	*aa = inst->inst.sc.aa;
}

void load_ds_form_inst(instruction *inst, int *rt, int *ra, int *ds, int *xo){
	*rt = inst->inst.ds.rt;
	*ra = inst->inst.ds.ra;
	*ds = inst->inst.ds.ds;
	*xo = inst->inst.ds.xo;
}

void load_x_form_inst(instruction *inst, int *rt, int *ra, int *rb, int *xo, int *rc){
	*rt = inst->inst.x.rt;
	*ra = inst->inst.x.ra;
	*rb = inst->inst.x.rb;
	*xo = inst->inst.x.xo;
	*rc = inst->inst.x.rc;
}

void load_b_form_inst(instruction *inst, int *bo, int *bi, int *bd, int *aa, int *lk){
	*bo = inst->inst.b.bo;
	*bi = inst->inst.b.bi;
	*bd = inst->inst.b.bd;
	*aa = inst->inst.b.aa;
	*lk = inst->inst.b.lk;
}
void load_m_form_inst(instruction *inst, int *rs, int *ra, int *rb, int *mb, int *me, int *rc){
	*rs = inst->inst.m.rs;
	*ra = inst->inst.m.ra;
	*rb = inst->inst.m.rb;
	*mb = inst->inst.m.mb;
	*me = inst->inst.m.me;
	*rc = inst->inst.m.rc;
}

void load_md_form_inst(instruction *inst, int *rs, int *ra, int *sh, int *mb, int *xo, int *rc){
	*rs = inst->inst.md.rs;
	*ra = inst->inst.md.ra;
	*sh = inst->inst.md.sh;
	*mb = inst->inst.md.mb;
	*xo = inst->inst.md.xo;
	*rc = inst->inst.md.rc;
}
void free_inst(instruction *inst){
	free(inst);
}
