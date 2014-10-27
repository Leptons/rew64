#include "rew64.h"
#include "utils.h"
#include "reg.h"
#include "inst.h"
#include "mem.h"
#include "op.h"
#include "run.h"

static void init(mem_addr initial_NIP);
static bool exec(void);
static void update_CR(int r, int64 a, int64 b);
static void update_CR_u(int r, uint64 a, uint64 b);
static void update_SO_OV(int64 sum, int64 a, int64 b);
static void update3_SO_OV(int64 sum, int64 a, int64 b, int64 c);
static void update_CA(int64 sum, int64 a, int64 b);
static void update3_CA(int64 sum, int64 a, int64 b, int64 c);
static void use_trap(int to, int64 a, int64 b);

bool run_rew64(mem_addr initial_NIP, int steps, bool display){
	init(initial_NIP);

	for(; steps >= 0; steps--){
		if(!exec()){
			return false;
		}
	}

	return true;
}

static void init(mem_addr initial_NIP){
	NIP = initial_NIP;
	// TODO: initialize registers and flags here
	XER = 0;
}


#define XO_OE 0x0200 // 1<<9
#define LT 0x8
#define GT 0x4
#define EQ 0x2
#define IS_SO ((XER>>31)&1)
#define IS_OV ((XER>>30)&1)
#define IS_CA ((XER>>29)&1)
#define SET_SO(V) (XER^=(!!(V)^IS_SO)<<31)
#define SET_OV(V) (XER^=(!!(V)^IS_OV)<<30)
#define SET_CA(V) (XER^=(!!(V)^IS_CA)<<29)
#define MASK_8  0x00000000000000ff
#define MASK_16 0x000000000000ffff
#define MASK_32 0x00000000ffffffff
#define MASK_64 0xffffffffffffffff
#define MASK (mode_64bit?MASK_64:MASK_32)
#define BT(I) (1LL<<(63-(I)))
#define IEA(V) ((V)&MASK)
#define EXTS(V, S) ((int64)(V) << (64-(S)) >> (64-(S))) // TODO: fix (currently depending on implementation)

static bool exec(){
	instruction *inst = read_mem_inst(NIP);
	bool invalid = false;

	mem_addr CIA = NIP;
	mem_addr NIA = CIA+4;

	switch(FORM(inst)){
		case I_FORM:
			{
				int li, aa, lk;
				load_i_form_inst(inst, &li, &aa, &lk);
				// Branch
				if(aa){
					NIA = IEA(EXTS(li<<4, 26)); // TODO: verify working correctly
				} else {
					NIA = IEA(CIA + EXTS(li<<4, 26));
				}
				if(lk){
					LR = CIA+4;
				}
				break;
			}

		case B_FORM:
			{
				int bo, bi, bd, aa, lk;
				load_b_form_inst(inst, &bo, &bi, &bd, &aa, &lk);
				// Branch Conditional
				if(~((bo>>2)&1)){
					CTR--;
				}
				bool ctr_ok = (bo>>2)&1 || ((IEA(CTR)!=0) ^ ((bo>>1)&1));
				bool cond_ok = (bo>>4)&1 || (get_CR(bi+32)==bo>>3);
				if(ctr_ok && cond_ok){
					if(aa){
						NIA = IEA(EXTS(bd<<2, 16));
					} else {
						NIA = IEA(CIA + EXTS(bd<<2, 16));
					}
				}
				if(lk){
					LR = CIA+4;
				}

				break;
			}

		case XL_FORM:
			{
				int bt, ba, bb, xo, lk;
				load_xl_form_inst(inst, &bt, &ba, &bb, &xo, &lk);
				int bo = bt;
				int bi = ba;
				// int bh = bb & 0x3;
				switch(xo){
					case 16:
						// Branch Conditional to Link Register
						{
							if(~((bo>>2)&1)){
								CTR--;
							}
							bool ctr_ok = (bo>>2)&1 || ((IEA(CTR)!=0) ^ ((bo>>1)&1));
							bool cond_ok = (bo>>4)&1 || (get_CR(bi+32)==bo>>3);
							if(ctr_ok && cond_ok){
								NIA = IEA(EXTS(LR&~0x3, 64));
							}
							if(lk){
								LR = CIA+4;
							}
							break;
						}
					case 528:
						// Branch Conditional to Count Register
						{
							bool cond_ok = (bo>>4)&1 || (get_CR(bi+32)==bo>>3);
							if(cond_ok){
								NIA = IEA(EXTS(LR&~0x3, 64));
							}
							if(lk){
								LR = CIA+4;
							}
							break;
						}
						break;
					case 257:
						// Condition Register AND (TODO)
						break;
					case 225:
						// Condition Register NAND (TODO)
						break;
					case 449:
						// Condition Register OR (TODO)
						break;
					case 193:
						// Condition Register XOR (TODO)
						break;
					case 33:
						// Condition Register NOR (TODO)
						break;
					case 289:
						// Condition Register Equivalent (TODO)
						break;
					case 129:
						// Condition Register AND with Complement (TODO)
						break;
					case 417:
						// Condition Register OR with Complement (TODO)
						break;
					case 0:
						// Move Condition Register Field (TODO)
						break;
					default:
						invalid = true;
						break;
				}
				break;
			}

			// System Call
		case SC_FORM:
			{
				int lev, aa;
				load_sc_form_inst(inst, &lev, &aa);
				if(aa == 1){
					// System Call (TODO)
					// syscall(lev);
				} else {
					invalid = true;
				}
				break;
			}

		case D_FORM:
			{
				int rt, ra, d;
				load_d_form_inst(inst, &rt, &ra, &d);
				int rs = rt;
				int bf = rt>>2;
				int to = rt;
				int l = rt&1;
				int si = d;
				int ui = d;
				switch(OPCD(inst)){
					// Load
					case 34:
						// Load Byte and Zero
						R[rt] = read_mem_8((ra==0?0:R[ra])+EXTS(d, 16));
						break;
					case 35:
						// Load Byte and Zero Update
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += EXTS(d, 16);
						R[rt] = read_mem_8(R[ra]);
						break;
					case 40:
						// Load Halfword and Zero
						R[rt] = read_mem_16((ra==0?0:R[ra])+EXTS(d, 16));
						break;
					case 41:
						// Load Halfword and Zero with Update
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += EXTS(d, 16);
						R[rt] = read_mem_16(R[ra]);
						break;
					case 42:
						// Load Halfword Algebraic
						R[rt] = EXTS(read_mem_16((ra==0?0:R[ra])+EXTS(d, 16)), 16);
						break;
					case 43:
						// Load Halfword Algebraic with Update
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += EXTS(d, 16);
						R[rt] = EXTS(read_mem_16(R[ra]), 16);
						break;
					case 32:
						// Load Word and Zero
						R[rt] = read_mem_32((ra==0?0:R[ra])+EXTS(d, 16));
						break;
					case 33:
						// Load Word and Zero with Update
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += EXTS(d, 16);
						R[rt] = read_mem_32(R[ra]);
						break;

						// Store
					case 38:
						// Store Byte
						set_mem_8((ra==0?0:R[ra])+EXTS(d, 16), rs);
						break;
					case 39:
						// Store Byte
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += EXTS(d, 16);
						set_mem_8(R[ra], rs);
						break;
					case 44:
						// Store Halfword
						set_mem_16((ra==0?0:R[ra])+EXTS(d, 16), rs);
						break;
					case 45:
						// Store Halfword with Update
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += EXTS(d, 16);
						set_mem_16(R[ra], rs);
						break;
					case 36:
						// Store Word
						set_mem_32((ra==0?0:R[ra])+EXTS(d, 16), rs);
						break;
					case 37:
						// Store Word with Update
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += EXTS(d, 16);
						set_mem_32(R[ra], rs);
						break;

						// Load/Store Multiple Word
					case 46:
						// Load Multiple Word
						{
							mem_addr EA = (ra==0?0:R[ra])+EXTS(d, 16);
							int r = rt;
							for(; r<32; r++, EA+=4){
								R[r] = read_mem_32(EA);
							}
							break;
						}
					case 47:
						// Store Multiple Word
						{
							mem_addr EA = (ra==0?0:R[ra])+EXTS(d, 16);
							int r = rs;
							for(; r<32; r++, EA+=4){
								set_mem_32(EA, R[r]);
							}
							break;
						}

						// Arithmetric Operations
					case 14:
						// Add Immediate
						R[rt] = (ra==0?0:R[ra])+EXTS(si, 16);
					case 15:
						// Add Immediate Shifted
						R[rt] = (ra==0?0:R[ra])+EXTS(si<<16, 32);
					case 12:
						// Add Immediate Carrying
						{
							int64 a = (ra==0?0:R[ra]), b = EXTS(si, 16);
							int64 t = a + b;
							update_CA(t, a, b);
							R[rt] = t;
							break;
						}
					case 13:
						// Add Immediate Carrying and Record
						{
							int64 a = (ra==0?0:R[ra]), b = EXTS(si, 16);
							int64 t = a + b;
							update_CA(t, a, b);
							update_CR(0, t, 0);
							R[rt] = t;
							break;
						}
					case 8:
						// Subtract From Immediate carrying (TODO: fix (-R[ra] may be bug))
						{
							int64 a = (ra==0?0:-R[ra]), b = EXTS(si, 16);
							int64 t = a + b;
							update_CA(t, a, b);
							R[rt] = t;
							break;
						}
						break;
					case 7:
						// Multiply Low Immediate (TODO)
						break;

						// Compare
					case 11:
						// Compare Immediate
						{
							int a;
							if(l==0){
								a = EXTS(R[ra]&MASK_32, 32);
							} else {
								a = R[ra]&MASK_32;
							}
							update_CR(bf, a, EXTS(si, 16));
							break;
						}
					case 10:
						// Compare Logical Immediate
						{
							int a;
							if(l){
								a = R[ra]&MASK_32;
							} else {
								a = R[ra];
							}
							update_CR_u(bf, a, ui);
							break;
						}

						// Trap
					case 3:
						// Trap Word Immediate
						use_trap(to, EXTS(R[ra]&MASK_32, 32), EXTS(si, 16));
						break;
					case 2:
						// Trap Doubleword Immediate
						use_trap(to, R[ra], EXTS(si, 16));
						break;

						// Logical Operations
					case 28:
						// AND Immediate
						R[ra] = R[rs] & ui;
						update_CR(0, R[ra], 0);
						break;
					case 29:
						// And Immediate Shifted
						R[ra] = R[rs] & (ui<<16);
						update_CR(0, R[ra], 0);
						break;
					case 24:
						// OR Immediate
						R[ra] = R[rs] | ui;
						break;
					case 25:
						// OR Immediate Shifted
						R[ra] = R[rs] | (ui<<16);
						break;
					case 26:
						// XOR Immediate
						R[ra] = R[rs] ^ ui;
						break;
					case 27:
						// XOR Immediate Shifted
						R[ra] = R[rs] ^ (ui<<16);
						break;

					default:
						invalid = true;
						break;
				}
				break;
			}

		case DS_FORM:
			{
				int rt, ra, ds, xo;
				load_ds_form_inst(inst, &rt, &ra, &ds, &xo);
				int rs = rt;
				switch(OPCD(inst)){
					case 58:
						switch(xo){
							case 2:
								// Load Word Algebraic
								R[rt] = EXTS(read_mem_32((ra==0?0:R[ra])+EXTS(ds<<2, 16)), 32);
								break;
							case 0:
								// Load Doubleword
								R[rt] = read_mem_64((ra==0?0:R[ra])+EXTS(ds<<2, 16));
								break;
							case 1:
								// Load Doubleword with Update
								if(ra==0 || ra==rt){
									invalid = true;
									break;
								}
								R[ra] += EXTS(ds<<2, 16);
								R[rt] = read_mem_64(R[ra]);
								break;

							default:
								invalid = true;
								break;
						}
						break;
					case 62:
						switch(xo){
							case 0:
								// Store Doubleword
								set_mem_64((ra==0?0:R[ra])+EXTS(ds<<2, 16), rs);
								break;
							case 1:
								// Store Doubleword with Update
								if(ra==0){
									invalid = true;
									break;
								}
								R[ra] += EXTS(ds<<2, 16);
								set_mem_64(R[ra], rs);
								break;

							default:
								invalid = true;
								break;
						}
						break;

				}
				break;
			}

		case X_FORM:
			{
				int rt, ra, rb, xo, rc;
				load_x_form_inst(inst, &rt, &ra, &rb, &xo, &rc);
				int rs = rt;
				int bf = rt>>2;
				int to = rt;
				int l = rt&1;
				int nb = rb;
				int sh = rb;
				switch(xo){
					// Load
					case 87:
						// Load Byte and Zero Indexed
						R[rt] = read_mem_8((ra==0?0:R[ra])+R[rb]);
						break;
					case 119:
						// Load Byte and Zero with Update Indexed
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						R[rt] = read_mem_8(R[ra]);
						break;
					case 279:
						// Load Halfword and Zero Indexed
						R[rt] = read_mem_16((ra==0?0:R[ra])+R[rb]);
						break;
					case 311:
						// Load Halfword and Zero with Update Indexed
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						R[rt] = read_mem_16(R[ra]);
						break;
					case 343:
						// Load Halfword Algebraic Indexed
						R[rt] = EXTS(read_mem_16((ra==0?0:R[ra])+R[rb]), 16);
						break;
					case 375:
						// Load Halfword Algebraic with Update Indexed
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						R[rt] = EXTS(read_mem_16(R[ra]), 16);
						break;
					case 23:
						// Load Word and Zero Indexed
						R[rt] = read_mem_32((ra==0?0:R[ra])+R[rb]);
						break;
					case 55:
						// Load Word and Zero with Update Incexed
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						R[rt] = read_mem_32(R[ra]);
						break;
					case 341:
						// Load Word Algebraic Indexed
						R[rt] = EXTS(read_mem_32((ra==0?0:R[ra])+R[rb]), 32);
						break;
					case 373:
						// Load Word Algebraic with Update Indexed
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						R[rt] = EXTS(read_mem_32(R[ra]), 32);
						break;
					case 21:
						// Load Doubleword Indexed
						R[rt] = read_mem_64((ra==0?0:R[ra])+R[rb]);
						break;
					case 53:
						// Load Doubleword with Update Indexed
						if(ra==0 || ra==rt){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						R[rt] = read_mem_64(R[ra]);
						break;

						// Store
					case 215:
						// Store Byte Indexed
						set_mem_8((ra==0?0:R[ra])+R[rb], rs);
						break;
					case 247:
						// Store Byte with Update Indexed
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						set_mem_8(R[ra], rs);
						break;
					case 407:
						// Store Halfword Indexed
						set_mem_16((ra==0?0:R[ra])+R[rb], rs);
						break;
					case 439:
						// Store Halfword with Update Indexed
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						set_mem_16(R[ra], rs);
						break;
					case 151:
						// Store Word Indexed
						set_mem_32((ra==0?0:R[ra])+R[rb], rs);
						break;
					case 183:
						// Store Word with Update Indexed
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						set_mem_32(R[ra], rs);
						break;
					case 149:
						// Store Doubleword Indexed
						set_mem_64((ra==0?0:R[ra])+R[rb], rs);
						break;
					case 181:
						// Store Doubleword with Update Indexed
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						set_mem_64(R[ra], rs);
						break;

						// Load/Store Byte-Reverse Indexed
					case 790:
						// Load Halfword Byte-Reverse Indexed
						R[rt] = rev(read_mem_16((ra==0?0:R[ra])+R[rb]), 2, 8);
						break;
					case 918:
						// Store Halfword Byte-Reverse Indexed
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						set_mem_16(R[ra], rev(rs, 2, 8));
						break;
					case 534:
						// Load Word Byte-Reverse Indexed
						R[rt] = rev(read_mem_32((ra==0?0:R[ra])+R[rb]), 4, 8);
						break;
					case 662:
						// Store Word Byte-Reverse Indexed 
						if(ra==0){
							invalid = true;
							break;
						}
						R[ra] += R[rb];
						set_mem_32(R[ra], rev(rs, 4, 8));
						break;

						// Load/Store String
					case 597:
						// Load String Word Immediate
						{
							if(ra==0){
								invalid = true;
							}
							mem_addr EA = (ra==0?0:R[ra]);
							int n = (nb==0?32:nb);
							int r = rt-1;
							int i = 32;
							for(; n>0; n--, EA++){
								if(i==32){
									r++;
									if(r==32) r = 0;
									R[r] = 0;
								}
								R[r] &= ~(BT(i-1)-BT(i+7)); // TODO: test
								R[r] = read_mem_8(EA) << (63-(i+7));
								i += 8;
								if(i==64) i = 32;
							}
							break;
						}
					case 533:
						// Load String Word Indexed
						{
							if(rt==ra || rt==rb){
								invalid = true;
							}
							mem_addr EA = (ra==0?0:R[ra])+R[rb];
							int n = XER & 0x7f;
							int r = rt-1;
							int i = 32;
							R[rt] = -1; // undifined
							for(; n>0; n--, EA++){
								if(i==32){
									r++;
									if(r==32) r = 0;
									R[r] = 0;
								}
								R[r] &= ~(BT(i-1)-BT(i+7)); // TODO: test
								R[r] = read_mem_8(EA) << (63-(i+7));
								i += 8;
								if(i==64) i = 32;
							}
							break;
						}
						break;
					case 725:
						// Store String Word Immediate
						{
							mem_addr EA = (ra==0?0:R[ra]);
							int n = (nb==0?32:nb);
							int r = rs-1;
							int i = 32;
							for(; n>0; n--, EA++){
								if(i==32){
									r++;
									if(r==32) r = 0;
								}
								set_mem_8(EA, R[r] >> (63-(i+7)));
								i += 8;
								if(i==64) i = 32;
							}
							break;
						}

					case 661:
						// Store String Word Indexed
						{
							mem_addr EA = (ra==0?0:R[ra])+R[rb];
							int n = XER & 0x7f;
							int r = rs-1;
							int i = 32;
							for(; n>0; n--, EA++){
								if(i==32){
									r++;
									if(r==32) r = 0;
								}
								set_mem_8(EA, R[r] >> (63-(i+7)));
								i += 8;
								if(i==64) i = 32;
							}
							break;
						}
						break;

						// Compare
					case 0:
						// Compare (TODO)
						{
							int a, b;
							if(l){
								a = EXTS(R[ra]&MASK_32, 32);
								b = EXTS(R[rb]&MASK_32, 32);
							} else {
								a = R[ra];
								b = R[rb];
							}
							update_CR(bf, a, b);
							break;
						}
					case 32:
						// Compare Logical
						{
							int a, b;
							if(l){
								a = R[ra]&MASK_32;
								b = R[rb]&MASK_32;
							} else {
								a = R[ra];
								b = R[rb];
							}
							update_CR_u(bf, a, b);
							break;
						}

						// Trap
					case 4:
						// Trap Word
						use_trap(to, EXTS(R[ra]&MASK_32, 32), EXTS(R[rb]&MASK_32, 32));
						break;
					case 68:
						// Trap Doubleword
						use_trap(to, R[ra], R[rb]);
						break;

						// Logical Operations
					case 28:
						// AND
						R[ra] = R[rs]&R[rb];
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 444:
						// OR
						R[ra] = R[rs]|R[rb];
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 316:
						// XOR 
						R[ra] = R[rs]^R[rb];
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 476:
						// NAND 
						R[ra] = ~(R[rs]&R[rb]);
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 124:
						// NOR 
						R[ra] = ~(R[rs]|R[rb]);
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 284:
						// Equivalent 
						R[ra] = ~(R[rs]^R[rb]);
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 60:
						// AND with Complement
						R[ra] = (R[rs]&~R[rb]);
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 412:
						// OR with Complement
						R[ra] = (R[rs]|~R[rb]);
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 954:
						// Extend Sign Byte
						R[ra] = EXTS(R[rs]&MASK_8, 8);
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 922:
						// Extend Sign Halfword
						R[ra] = EXTS(R[rs]&MASK_16, 16);
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 26:
						// Count Leading Zeros Word
						{
							int n = 31;
							while(n >= 0 && !(R[rs]&(1<<(n--))));
							R[ra] = 31-n;
							if(rc) update_CR(0, R[ra], 0);
							break;
						}
					case 508:
						// Compare Bytes
						{
							int n;
							for(n = 0; n < 8; n++){
								int64 mask = MASK_8<<(8*n);
								if((R[rs]&mask) == (R[rb]&mask)) R[ra] |= mask;
								else R[ra] &= ~mask;
							}
							break;
						}
					case 186:
						// Parity Doubleword
						{
							int s = 0;
							int i;
							for(i = 0; i < 8; i++){
								s ^= (rs>>(8*i))&1;
							}
							R[ra] = s;
							break;
						}
					case 154:
						// Parity Word
						{
							int64 s = 0;
							int t = 0;
							int i;
							for(i = 0; i < 4; i++){
								t ^= (rs>>(8*i))&1;
							}
							for(i = 4; i < 8; i++){
								s ^= (rs>>(8*i))&1;
							}
							R[ra] = t;
							R[ra] |= s<<32;
							break;
						}
					case 986:
						// Extend Sign Word
						R[ra] = EXTS(R[rs], 32);
						if(rc) update_CR(0, R[ra], 0);
						break;
					case 122:
						// Population Count Bytes 
						{
							int i, j;
							R[ra] = 0;
							for(i = 0; i < 8; i++){
								int64 n = 0;
								for(j = 0; j < 8; j++){
									if(R[rs]&(1LL<<(8*i+j))) n++;
								}
								R[ra] |= n<<(8*i);
							}
							break;
						}
					case 58:
						// Count Leading Zeros Doubleword
						{
							int n = 64;
							while(n >= 0 && !(R[rs]&(1LL<<(n--))));
							R[ra] = 63-n;
							if(rc) update_CR(0, R[ra], 0);
							break;
						}

						// Shift
					case 24:
						// Shift Left Word
						{
							int64 n = R[rb]&0x1f;
							int64 r = rot(R[rs]&MASK_32, n, 32);
							int64 m = (R[rb]&(1<<5))?0:mk_mask(32, 63-n);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
						}
					case 536:
						// Shift Right Word
						{
							int64 n = R[rb]&0x1f;
							int64 r = rot(R[rs]&MASK_32, 64-n, 32);
							int64 m = (R[rb]&(1<<5))?0:mk_mask(n+32, 63);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
						}
					case 824:
						// Shift Right Algebraic Word Immediate
						{
							int64 n = sh;
							int64 r = rot(R[rs]&MASK_32, 64-n, 32);
							int64 m = mk_mask(n+32, 63);
							int64 s = (R[rs]>>31)&1;
							R[ra] = (r&m) | ((-s)&~m);
							SET_CA(s & (((r&~m)&MASK_32)!=0));
							if(rc) update_CR(0, R[ra], 0);
						}
					case 792:
						// Shift Right Algebraic Word
						{
							int64 n = R[rb]&0x1f;
							int64 r = rot(R[rs]&MASK_32, 64-n, 32);
							int64 m = (R[rb]&(1<<5))?0:mk_mask(n+32, 63);
							int64 s = (R[rs]>>31)&1;
							R[ra] = (r&m) | ((-s)&~m);
							SET_CA(s & (((r&~m)&MASK_32)!=0));
							if(rc) update_CR(0, R[ra], 0);
						}
					case 27:
						// Shift Left Doubleword
						{
							int64 n = R[rb]&0x3f;
							int64 r = rot(R[rs], n, 64);
							int64 m = (R[rb]&(1<<6))?0:mk_mask(0, 63-n);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
						}
					case 539:
						// Shift Right Doubleword
						{
							int64 n = R[rb]&0x3f;
							int64 r = rot(R[rs], 64-n, 64);
							int64 m = (R[rb]&(1<<6))?0:mk_mask(n, 63);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
						}
					case 794:
						// Shift Right Algebraic Doubleword
						{
							int64 n = R[rb]&0x3f;
							int64 r = rot(R[rs], 64-n, 64);
							int64 m = (R[rb]&(1<<6))?0:mk_mask(n, 63);
							int64 s = R[rs]&1;
							R[ra] = (r&m) | ((-s)&~m);
							SET_CA(s & ((r&~m)!=0));
							if(rc) update_CR(0, R[ra], 0);
							break;
						}

					default:
						{
						}
				}
				// XO_FORM
				int oe = xo & XO_OE;
				switch(xo&~XO_OE){
					// Arithmetric Operations
					case 266:
					case 266+XO_OE: // TODO: remove these lines after tested
						// Add
						{
							int64 a = R[ra], b = R[rb];
							int64 t = a + b;
							R[rt] = t;
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 40:
					case 40+XO_OE:
						// Subtract From
						{
							int64 a = -R[ra], b = R[rb];
							int64 t = a + b;
							R[rt] = t;
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 10:
					case 10+XO_OE:
						// Add Carrying
						{
							int64 a = R[ra], b = R[rb];
							int64 t = a + b;
							R[rt] = t;
							update_CA(t, a, b);
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 8:
					case 8+XO_OE:
						// Subtract From Carrying
						{
							int64 a = -R[ra], b = R[rb];
							int64 t = a + b;
							R[rt] = t;
							update_CA(t, a, b);
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 138:
					case 138+XO_OE:
						// Add Extended (TODO)
						{
							int64 a = R[ra], b = R[rb], c = IS_CA;
							int64 t = a + b + c;
							R[rt] = t;
							update3_CA(t, a, b, c);
							if(oe){
								update3_SO_OV(t, a, b, c);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 136:
					case 136+XO_OE:
						// Subtract From Extended (TODO)
						{
							int64 a = -R[ra], b = R[rb], c = IS_CA;
							int64 t = a + b + c;
							R[rt] = t;
							update3_CA(t, a, b, c);
							if(oe){
								update3_SO_OV(t, a, b, c);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 234:
					case 234+XO_OE:
						// Add to Minus One Extended
						{
							int64 a = R[ra], b = IS_CA - 1;
							int64 t = a + b;
							R[rt] = t;
							update_CA(t, a, b);
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 232:
					case 232+XO_OE:
						// Subtract From Minus One Extended
						{
							int64 a = -R[ra], b = IS_CA - 1;
							int64 t = a + b;
							R[rt] = t;
							update_CA(t, a, b);
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 202:
					case 202+XO_OE:
						// Add to Zero Extended
						{
							int64 a = R[ra], b = IS_CA;
							int64 t = a + b;
							R[rt] = t;
							update_CA(t, a, b);
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 200:
					case 200+XO_OE:
						// Subtract From Zero Extended
						{
							int64 a = -R[ra], b = IS_CA;
							int64 t = a + b;
							R[rt] = t;
							update_CA(t, a, b);
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 104:
					case 104+XO_OE:
						// Negate
						{
							int64 a = ~R[ra], b = 1;
							int64 t = a + b;
							R[rt] = t;
							if(oe){
								update_SO_OV(t, a, b);
							}
							if(rc){
								update_CR(0, t, 0);
							}
							break;
						}
					case 235:
						// Multiply Low Word (TODO
						{
							R[rt] = (R[ra]&MASK_32) * (R[rb]&MASK_32);
							break;
						}
					case 75:
						// Multiply High Word (TODO)
						break;
					case 11:
						// Multiply High Word Unsigned (TODO)
						break;
					case 491:
						// Divide Word (TODO)
						break;
					case 259:
						// Divide Word Unsigned (TODO)
						break;
					case 233:
						// Divide Low Doubleword (TODO)
						break;
					case 73:
						// Divide High Doubleword (TODO)
						break;
					case 9:
						// Divide High Doubleword Unsigned (TODO)
						break;
					case 489:
						// Divide Doubleword (TODO)
						break;
					case 457:
						// Divide Doubleword Unsigned (TODO)
						break;

					default:
						// A_FORM (TODO: move before XO_FORM)
						/* Interger Select
						*/
						invalid = true;
				}
				// XS_FORM
				int sh2 = (rb<<1) | (xo&1);
				switch(xo>>1){
					case 413:
						// Shift Right Algebraic Doubleword Immediate
						{
							int n = rot(sh2, 5, 6);
							int64 r = rot(R[rs], 64-n, 64);
							int64 m = mk_mask(n, 63);
							int64 s = R[rs]&1;
							R[ra] = (r&m) | ((-s)&~m);
							SET_CA(s & ((r&~m)!=0));
							if(rc) update_CR(0, R[ra], 0);
							break;
						}
				}
				break;
			}

			// Rotate
		case M_FORM:
			{	
				int rs, ra, rb, mb, me, rc;
				load_m_form_inst(inst, &rs, &ra, &rb, &mb, &me, &rc);
				int sh = rb;
				switch(OPCD(inst)){
					case 21:
						// Rotate Left Word Immediate then AND
						{
							int n = sh;
							int64 r = rot(R[rs]&MASK_32, n, 32);
							int64 m = mk_mask(mb+32, me+32);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
							break;
						}
					case 23:
						// Rotate Left Word then AND with Mask
						{
							int n = R[rb] & 0x1f;
							int64 r = rot(R[rs]&MASK_32, n, 32);
							int64 m = mk_mask(mb+32, me+32);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
							break;
						}

					case 20:
						// Rotate Left Word Immediate then AND with Mask (TODO)
						{
							int n = sh;
							int64 r = rot(R[rs]&MASK_32, n, 32);
							int64 m = mk_mask(mb+32, me+32);
							R[ra] = (r&m) | (R[ra]&~m);
							if(rc) update_CR(0, R[ra], 0);
							break;
						}

					default:
						invalid = true;
						break;

				}
				break;
			}

		case MD_FORM:
			{
				int rs, ra, sh, mb, xo, rc;
				load_md_form_inst(inst, &rs, &ra, &sh, &mb, &xo, &rc);
				int me = mb;
				switch(xo){
					case 0:
						// Rotate Left Doubleword Immediate then Clear Left
						{
							int n = rot(sh, 5, 6);
							int64 r = rot(R[rs], n, 64);
							int b = rot(mb, 5, 6);
							int64 m = mk_mask(b, 63);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
							break;
						}
					case 1:
						// Rotate Left Doubleword Immediate then Clear Right
						{
							int n = rot(sh, 5, 6);
							int64 r = rot(R[rs], n, 64);
							int e = rot(me, 5, 6);
							int64 m = mk_mask(0, e);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
							break;
						}
					case 2:
						// Rotate Left Doubleword then Clear
						{
							int n = rot(sh, 5, 6);
							int64 r = rot(R[rs], n, 64);
							int b = rot(mb, 5, 6);
							int64 m = mk_mask(b, ~n);
							R[ra] = r&m;
							if(rc) update_CR(0, R[ra], 0);
							break;
						}	
					case 3:
						// Rotate Left Doubleword then Clear
						{
							int n = rot(sh, 5, 6);
							int64 r = rot(R[rs], n, 64);
							int b = rot(mb, 5, 6);
							int64 m = mk_mask(b, ~n);
							R[ra] = (r&m) | (R[ra]&~m);
							if(rc) update_CR(0, R[ra], 0);
							break;
						}


					default:
						xo = (xo<<1) | (sh&1);
						int rb = sh>>1;
						switch(xo){
							case 8:
								// Rotate Left Doubleword then Clear Left
								{
									int n = R[rb] & 0x3f;
									int64 r = rot(R[rs], n, 64);
									int b = rot(mb, 5, 6);
									int64 m = mk_mask(b, 63);
									R[ra] = r&m;
									if(rc) update_CR(0, R[ra], 0);
									break;
								}
							case 9:
								// Rotate Left Doubleword then Clear Right
								{
									int n = R[rb] & 0x3f;
									int64 r = rot(R[rs], n, 64);
									int e = rot(me, 5, 6);
									int64 m = mk_mask(0, e);
									R[ra] = r&m;
									if(rc) update_CR(0, R[ra], 0);
									break;
								}

							default:
								invalid = true;
						}

				}
			}

		default:
			{
				invalid = true;
				break;
			}
	}

	NIP = NIA;

	return !invalid;
}

static void update_CR(int r, int64 a, int64 b){
	if(a < b) CR[r] = LT;
	else if(a > b) CR[r] = GT;
	else CR[r] = EQ;
	CR[r] |= IS_SO;
}

static void update_CR_u(int r, uint64 a, uint64 b){
	if(a < b) CR[r] = LT;
	else if(a > b) CR[r] = GT;
	else CR[r] = EQ;
	CR[r] |= IS_SO;
}

static void update_SO_OV(int64 sum, int64 a, int64 b){
	// TODO: implement
}

static void update3_SO_OV(int64 sum, int64 a, int64 b, int64 c){
	// TODO: implement
}

static void update_CA(int64 sum, int64 a, int64 b){
	// TODO: implement
}

static void update3_CA(int64 sum, int64 a, int64 b, int64 c){
	// TODO: implement
}

static void use_trap(int to, int64 a, int64 b){
	// TODO: invoke trap
	if(a < b && ((to>>4)&1));
	if(a > b && ((to>>3)&1));
	if(a == b && ((to>>2)&1));
	if((uint64)a < (uint64)b && ((to>>1)&1));
	if((uint64)a > (uint64)b && ((to>>0)&1));
}
