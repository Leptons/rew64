#include "rew64.h"
#include "utils.h"
#include "reg.h"
#include "inst.h"
#include "mem.h"
#include "op.h"
#include "run.h"

static void init(mem_addr initial_NIP);
static bool exec(void);

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
}

#define XO_OE 0x0200 // 1<<9
#define LO_MASK 0x0000ffff
#define HI_MASK 0xffff0000
#define IEA(V) (mode_64bit?(V):((V)&LO_MASK))
#define EXTS(V, S) ((int64)(V) << (64-(S)) >> (64-(S)))

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
				// Branch Conditional (TODO)

				break;
			}

		case XL_FORM:
			{
				int bt, ba, bb, xo, lk;
				load_xl_form_inst(inst, &bt, &ba, &bb, &xo, &lk);
				switch(xo){
					case 16:
						// Branch Conditional to Link Register (TODO)
						break;
					case 528:
						// Branch Conditional to Count Register (TODO)
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


		case D_FORM:
			{
				int rt, ra, d;
				load_d_form_inst(inst, &rt, &ra, &d);
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
						// Store Byte (TODO)
						break;
					case 39:
						// Store Byte (TODO)
						break;
					case 44:
						// Store Halfword (TODO)
						break;
					case 45:
						// Store Halfword with Update (TODO)
						break;
					case 36:
						// Store Word (TODO)
						break;
					case 37:
						// Store Word with Update (TODO)
						break;

						// Load/Store Multiple Word
					case 46:
						// Load Multiple Word (TODO)
						break;
					case 47:
						// Store Multiple Word (TODO)
						break;

					// Arithmetric Operations
					case 14:
						// Add Immediate (TODO)
					case 15:
						// Add Immediate Shifted (TODO)
					case 12:
						// Add Immediate Carrying (TODO)
						break;
					case 13:
						// Add Immediate Carrying and Record (TODO)
						break;
					case 8:
						// Subtract From Immediate carrying (TODO)
						break;
					case 7:
						// Multiply Low Immediate (TODO)
						break;

					// Compare
					case 11:
						// Compare Immediate (TODO)
						break;
					case 10:
						// Compare Logical Immediate (TODO)
						break;

					// Trap
					case 3:
						// Trap Word Immediate (TODO)
						break;
					case 2:
						// Trap Doubleword Immediate (TODO)
						break;

					// Logical Operations
					case 28:
						// AND Immediate (TODO)
						break;
					case 29:
						// And Immediate Shifted (TODO)
						break;
					case 24:
						// OR Immediate (TODO)
						break;
					case 25:
						// OR Immediate Shifted (TODO)
						break;
					case 26:
						// XOR Immediate (TODO)
						break;
					case 27:
						// XOR Immediate Shifted (TODO)
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


		case DS_FORM:
			{
				int rt, ra, ds, xo;
				load_ds_form_inst(inst, &rt, &ra, &ds, &xo);
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
								// Store Doubleword (TODO)
								break;
							case 1:
								// Store Doubleword with Update (TODO)
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
						// Store Byte Indexed (TODO)
						break;
					case 247:
						// Store Byte with Update Indexed (TODO)
						break;
					case 407:
						// Store Halfword Indexed (TODO)
						break;
					case 439:
						// Store Halfword with Update Indexed (TODO)
						break;
					case 151:
						// Store Word Indexed (TODO)
						break;
					case 183:
						// Store Word with Update Indexed (TODO)
						break;
					case 149:
						// Store Doubleword Indexed (TODO)
						break;
					case 181:
						// Store Doubleword with Update Indexed (TODO)
						break;

					// Load/Store Byte-Reverse Indexed
					case 790:
						// Load Halfword Byte-Reverse Indexed (TODO)
						break;
					case 918:
						// Store Halfword Byte-Reverse Indexed (TODO)
						break;
					case 534:
						// Load Word Byte-Reverse Indexed (TODO)
						break;
					case 662:
						// Store Word Byte-Reverse Indexed (TODO)
						break;

						// Load/Store String
					case 597:
						// Load String Word Immediate (TODO)
						break;
					case 533:
						// Load String Word Indexed (TODO)
						break;
					case 725:
						// Store String Word Immediate (TODO)
					case 661:
						// Store String Word Indexed (TODO)
						break;
						
					// Compare
					case 0:
						// Compare (TODO)
						break;
					case 32:
						// Compare Logical (TODO)
						break;

					// Trap
					case 4:
						// Trap Word (TODO)
						break;
					case 68:
						// Trap Doubleword (TODO)

					// Logical Operations
					case 28:
						// AND (TODO)
						break;
					case 444:
						// OR (TODO)
						break;
					case 316:
						// XOR (TODO)
						break;
					case 476:
						// NAND (TODO)
						break;
					case 124:
						// NOR (TODO)
						break;
					case 284:
						// Equivalent (TODO)
						break;
					case 60:
						// AND with Complement (TODO)
						break;
					case 412:
						// OR with Complement (TODO)
						break;
					case 954:
						// Extend Sign Byte (TODO)
						break;
					case 922:
						// Extend Sign Halfword (TODO)
						break;
					case 26:
						// Count Leading Zeros Word (TODO)
						break;
					case 508:
						// Compare Bytes (TODO)
						break;
					case 186:
						// Parity Doubleword (TODO)
						break;
					case 154:
						// Parity Word (TODO)
						break;
					case 986:
						// Extend Sign Word (TODO)
						break;
					case 122:
						// Population Count Bytes (TODO)
						break;
					case 58:
						// Count Leading Zeros Doubleword (TODO)
						break;

					// XO_FORM
					default:
						{
							int oe = xo & XO_OE;
							xo &= ~XO_OE;
							switch(xo){
								// Arithmetric Operations
								case 266:
								case 266+XO_OE: // TODO: remove these lines after tested
									// Add (TODO)
									break;
								case 40:
								case 40+XO_OE:
									// Subtract From (TODO)
									break;
								case 10:
								case 10+XO_OE:
									// Add Carrying (TODO)
									break;
								case 8:
								case 8+XO_OE:
									// Subtract From Carrying (TODO)
									break;
								case 138:
								case 138+XO_OE:
									// Add Extended (TODO)
									break;
								case 136:
								case 136+XO_OE:
									// Subtract From Extended (TODO)
									break;
								case 234:
								case 234+XO_OE:
									// Add to Minus One Extended (TODO)
									break;
								case 232:
								case 232+XO_OE:
									// Subtract From Minus One Extended (TODO)
									break;
								case 202:
								case 202+XO_OE:
									// Add to Zero Extended (TODO)
									break;
								case 200:
								case 200+XO_OE:
									// Subtract From Zero Extended (TODO)
									break;
								case 104:
								case 104+XO_OE:
									// Negate (TODO)
									break;
								case 235:
									// Multiply Low Word (TODO)
									break;
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
									// A_FORM (TODO)
									/* Interger Select
									 */
									invalid = true;
							}
						}
				}
				break;
			}

		// Rotate
		case M_FORM:
			{
				switch(OPCD(inst)){
					case 21:
						// Rotate Left Word Immediate then AND (TODO)
						break;
					case 23:
						// Rotate Left Word then AND with Mask (TODO)
						break;
					case 20:
						// Rotate Left Word Immediate then AND with Mask (TODO)
						break;
				}
				break;
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
