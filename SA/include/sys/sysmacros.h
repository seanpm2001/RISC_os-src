/* $Header: sysmacros.h,v 1.2 90/01/23 13:47:19 huang Exp $ */
/* $Copyright$ */

#ifndef	_SYS_SYSMACROS_
#define	_SYS_SYSMACROS_	1


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Some macros for units conversion
 */

/* a click is a page.  A segment is area mapped by a click of ptes (2Meg)*/
/* Core clicks to segments and vice versa */

#define ctos(x)		(((x) + (NCPS-1)) / NCPS)
#define	ctost(x)	((x) / NCPS)
#define	stoc(x)		((x) * NCPS)

/* XXX - what does this stand for? (byte to table physical?)*/
#define btotp(x) ((x)>>BPTSHFT)

#define stob(x)	(((unsigned)(x) & 0x7ff) << 21)
#define btos(x)	((unsigned)(x)>>21 & 0x7ff)

/* Core clicks to disk blocks */
#define	ctod(x) ((x)*NDPC)

/* inumber to disk address */
#ifdef INOSHIFT
#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))>>INOSHIFT)
#else
#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))/INOPB)
#endif

/* inumber to disk offset */
#ifdef INOSHIFT
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))&(INOPB-1))
#else
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))%INOPB)
#endif

/* clicks to bytes */
#ifdef BPCSHIFT
#define	ctob(x)	((x)<<BPCSHIFT)
#else
#define	ctob(x)	((x)*NBPC)
#endif

/* bytes to clicks */
#ifdef BPCSHIFT
#define	btoc(x)	(((unsigned)(x)+(NBPC-1))>>BPCSHIFT)
#define	btoct(x)	((unsigned)(x)>>BPCSHIFT)
#else
#define	btoc(x)	(((unsigned)(x)+(NBPC-1))/NBPC)
#define	btoct(x)	((unsigned)(x)/NBPC)
#endif

#ifdef INKERNEL

/* major part of a device internal to the kernel */

extern char MAJOR[128];
#undef major
#define	major(x)	(int)(MAJOR[(unsigned)((x)>>8)&0x7F])
#define	bmajor(x)	(int)(MAJOR[(unsigned)((x)>>8)&0x7F])

#else

/* major part of a device external from the kernel */
#undef major
#define	major(x)	(int)(((unsigned)x>>8)&0x7F)

#endif	/* INKERNEL */

/* minor part of a device */
#undef minor
#define	minor(x)	(int)(x&0xFF)

/* make a device number */
#undef makedev
#define	makedev(x,y)	(dev_t)(((x)<<8) | (y))

/*
 *   emajor() allows kernel/driver code to print external major numbers
 *   eminor() allows kernel/driver code to print external minor numbers
 */

#define emajor(x)	(int)(((unsigned)(x)>>8)&0x7F)
#define eminor(x)	(int)((x)&0xFF)

/*
 *  Evaluate to true if the process is a server - Distributed UNIX
 */
#define	server()	(u.u_procp->p_sysid != 0)

/*
 * bcd macros -- these macros only operate on 2 bcd digits (8 bits)
 */

#define bcd_to_dec(x)	( ((((x) >> 4) & 0xf) * 10) + ((x) & 0xf) )
#define dec_to_bcd(x)	( (((x) / 10) << 4) | ((x) % 10) )

#endif	_SYS_SYSMACROS_
