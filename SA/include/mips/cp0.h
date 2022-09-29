#ident "$Header: cp0.h,v 1.2 90/01/23 14:09:29 huang Exp $"
/* $Copyright$ */

/******************************************************************************
 *
 *		CP0 Definitions
 *
 * %Q% %M% %I% 
 *
 * Author	Bromo
 * Date started Wed July 10 15:37:59 PDT 1985
 * Module	standard.h
 * Purpose	provide a set of standard constants and macros for all avts
 *
 ******************************************************************************/

#define LOCORE
#include "cpu.h"

/******************************************************************************
 *    C O N S T A N T S
 ******************************************************************************/

/* entryhi register */

#define TLBHI_MASK (TLBHI_VPNMASK | TLBHI_PIDMASK)	/* Vaild bits */


/* entry lo register */

#define TLBLO_MASK (TLBLO_PFNMASK | TLBLO_N | TLBLO_D | TLBLO_G | TLBLO_V)

/* TLB array */
#ifdef LANGUAGE_C
struct tlb_entry_struct {
	union tlb_hi hi;
	union tlb_lo lo;
};
typedef struct tlb_entry_struct tlb_entry;
#endif

/* bad virtual address register */


/* tlb index register */
#define TLBINDX_MASK (TLBINX_PROBE | TLBINX_INXMASK)


/* tlb random register */


/* Context register */

#define CTXT_MASK TLBCTXT_BASEMASK


/* Status register */



/* Cause register */

/* Some of the diags use unused bits of the cause register to pass information
 *   these bits are defined below
 */
#define DIAG_RETURN_EPC4	0x10000		/* Return to epc + 4 on exception */
#define DIAG_RETURN_EPC4_BD 0x20000	/* Return via epc + 4 if not in a 
					 *  branch delay slot */
#define DIAG_BUMP_EPC 0x40000	/* Increment expected epc each time thru */


/* Exception Program counter */



/*******************************************************************************
 *   M  A C R O S
 ******************************************************************************/

#define TLB_ENTRY_HI( vpn, pid )\
  (((vpn)<<TLBHI_VPNSHIFT) | ((pid)<<TLBHI_PIDSHIFT)) 
#define TLB_ENTRY_LO( pfn, n, d, v, g )\
   (((pfn)<<TLBLO_PFNSHIFT) | (n) | (d) | (v) | (g) )
#define TLB_ENTRY( vpn, pid, pfn ) \
  TLB_ENTRY_HI( vpn, pid ), TLB_ENTRY_LO( pfn, 0, 0, 0, 0 )
