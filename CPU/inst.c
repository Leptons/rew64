#include "rew64.h"
#include "utils.h"
#include "reg.h"
#include "inst.h"
#include "mem.h"
#include "op.h"

// static int load_opcd(word val);
#define load_opcd(VAL) ((VAL >> 26) & 0x3f)

static instruction *st_b_form_inst(instruction *inst, word val);
static instruction *st_sc_form_inst(instruction *inst, word val);
static instruction *st_i_form_inst(instruction *inst, word val);
static instruction *st_xl_form_inst(instruction *inst, word val);
static instruction *st_x_form_inst(instruction *inst, word val);
static instruction *st_d_form_inst(instruction *inst, word val);
static instruction *st_ds_form_inst(instruction *inst, word val);
static instruction *st_bad_inst(instruction *inst, word val);


instruction *decode_inst(word val){
	int opcd = BIN_OPCD(val);
	instruction *inst = zmalloc(sizeof(instruction));
	inst->opcd = opcd;
	inst->encoding = val;

	/*
	switch(opcd){
		case 16:
			return st_b_form_inst(instruction *inst, val);
		case 17:
			return st_sc_form_inst(instruction *inst, val);
		case 18:
			return st_i_form_inst(instruction *inst, val);
		case 19:
			return st_xl_form_inst(instruction *inst, val);
		case 31:
			return st_x_form_inst(instruction *inst, val);
		case 14:
		case 15:
		case 32:
		case 33:
		case 34:
		case 35:
		case 40:
		case 41:
		case 42:
		case 43:
		case 46:
		case 47:
			return st_d_form_inst(instruction *inst, val);
		case 58:
		case 62:
			return st_ds_form_inst(instruction *inst, val);
		default:
			return st_bad_inst(val);
	}
	*/
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
	} else {
		inst->opcd = 0;
		return st_bad_inst(inst, val);
	}
}

static instruction *st_b_form_inst(instruction *inst, word val){
	inst->inst.b.bo = BIN_BO(val);
	inst->inst.b.bi = BIN_BI(val);
	inst->inst.b.bd = BIN_BD(val);
	inst->inst.b.aa = BIN_AA(val);
	inst->inst.b.lk = BIN_LK(val);

	inst->form = B_FORM; // for test

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

	inst->form = DS_FORM; // for test
	return inst;
}
static instruction *st_bad_inst(instruction *inst, word val){
	fprintf(stderr, "Cannot create instruction\n");
	return inst;
}


void free_inst(instruction *inst){
	free(inst);
}

/*
static int load_opcd(word val){
	return (val >> 26) & 0x3f;
}
*/
