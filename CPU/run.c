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

static bool exec(){
	instruction *inst = read_mem_inst(NIP);
	bool invalid = false;
	switch(FORM(inst)){
		case I_FORM:
			{
				// Branch (TODO)
				break;
			}

		case D_FORM:
			{
				switch(OPCD(inst)){
					case 18:
						// Branch (TODO)
						break;
					case 16:
						// Branch Conditional (TODO)
						break;

						// Load
					case 34:
						// Load Byte and Zero (TODO)
						break;
					case 35:
						// Load Byte and Zero Update (TODO)
						break;
					case 40:
						// Load Halfword and Zero (TODO)
						break;
					case 41:
						// Load Halfword and Zero with Update (TODO)
						break;
					case 42:
						// Load Halfword Algebraic (TODO)
						break;
					case 43:
						// Load Halfword Algebraic with Update (TODO)
						break;
					case 32:
						// Load Word and Zero (TODO)
						break;
					case 33:
						// Load Word and Zero with Update(TODO)
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

						// Add
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

					default:
						invalid = true;
						break;
				}
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

		// System Call
		case SC_FORM:
			{
				if(inst->inst.sc.aa == 1){
					// System Call (TODO)
					// syscall(inst->inst.sc.lev);
				} else {
					invalid = true;
				}
				break;
			}


		case DS_FORM:
			{
				switch(OPCD(inst)){
					case 58:
						switch(inst->inst.ds.xo){
							case 2:
								// Load Word Algebraic (TODO)
								break;
							case 0:
								// Load Doubleword (TODO)
								break;
							case 1:
								// Load Doubleword with Update (TODO)
								break;

							default:
								invalid = true;
								break;
						}
						break;
					case 62:
						switch(inst->inst.ds.xo){
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
						// Load Byte and Zero Indexed (TODO)
						break;
					case 119:
						// Load Byte and Zero with Update Indexed (TODO)
						break;
					case 279:
						// Load Halfword and Zero Indexed (TODO)
						break;
					case 311:
						// Load Halfword and Zero with Update Indexed (TODO)
						break;
					case 343:
						// Load Halfword Algebraic Indexed (TODO)
						break;
					case 375:
						// Load Halfword Algebraic with Update Indexed (TODO)
						break;
					case 23:
						// Load Word and Zero Indexed (TODO)
						break;
					case 55:
						// Load Word and Zero with Update Incexed (TODO)
						break;
					case 341:
						// Load Word Algebraic Indexed (TODO)
						break;
					case 373:
						// Load Word Algebraic with Update Indexed (TODO)
						break;
					case 21:
						// Load Doubleword Indexed (TODO)
						break;
					case 53:
						// Load Doubleword with Update Indexed (TODO)
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

					// XO_FORM
					default:
						{
							int oe = xo & XO_OE;
							xo &= ~XO_OE;
							switch(xo){
								// Add
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

								default:
									invalid = true;
							}
						}
				}
				break;
			}

		case B_FORM:
			{
				break;
			}

		default:
			{
				invalid = true;
				break;
			}
	}

	return !invalid;
}
