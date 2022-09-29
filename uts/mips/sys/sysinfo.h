/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1990 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: sysinfo.h,v 1.8.5.2.1.4 90/08/03 11:27:11 hawkes Exp $ */

#ifndef	_SYS_SYSINFO_
#define	_SYS_SYSINFO_	1


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 *	System Information.
 */

struct sysinfo {
	time_t	cpu[5];
#define	CPU_IDLE	0
#define	CPU_USER	1
#define	CPU_KERNEL	2
#define	CPU_WAIT	3
#define CPU_SXBRK	4
	time_t	wait[3];
#define	W_IO	0
#define	W_SWAP	1
#define	W_PIO	2
	long	bread;
	long	bwrite;
	long	lread;
	long	lwrite;
	long	phread;
	long	phwrite;
	long	swapin;
	long	swapout;
	long	bswapin;
	long	bswapout;
	long	pswitch;
	long	syscall;
	long 	remcall;	/* REMOTE: remote call */
	long    fileop;		/* REMOTE: remote fileop */
	long    serve;		/* REMOTE: remote server */
	long	sysread;
	long	syswrite;
	long	sysfork;
	long	sysexec;
	long	runque;
	long	runocc;
	long	swpque;
	long	swpocc;
	long	iget;
	long	namei;
	long	dirblk;
	long	readch;
	long	writech;
	long	rcvint;
	long	xmtint;
	long	mdmint;
	long	rawch;
	long	canch;
	long	outch;
	long	msg;
	long	sema;
	long	pnpfault;
	long	wrtfault;
};

extern struct sysinfo sysinfo;

struct syswait {
	short	iowait;
	short	swap;
	short	physio;
};

struct dinfo {
			/* ELEMENT FOR sar -Du */
	time_t	serve;		/* ticks in serve() since boot */
			/* ELEMENTS FOR sar -Dc */
	long	isyscall;	/* # of incoming syscalls since boot */
	long	isysread;	/* # of incoming read's since boot */
	long	ireadch;	/* char's read by incoming read's */
	long	isyswrite;	/* # of incoming write's since boot */
	long	iwritech;	/* char's written by incoming writes */
	long	isysexec;	/* # of incoming exec's since boot */
	long	osyscall;	/* # of outgoing syscalls since boot */
	long	osysread;	/* # of outgoing read's since boot */
	long	oreadch;	/* char's read by outgoing read's */
	long	osyswrite;	/* # of outgoing write's since boot */
	long	owritech;	/* char's written by outgoing writes */
	long	osysexec;	/* # of outgoing exec's since boot */
			/* ELEMENTS FOR sar -S*/
	long	nservers;	/* sum of all servers since boot */
	long	srv_que;	/* sum of server Q lgth since boot */
	long	srv_occ;	/* ticks server Q found occupied */
	long	rcv_que;	/* sum of receive Q lgth since boot */
	long	rcv_occ;	/* clock ticks receive Q occupied */
};
extern struct dinfo dinfo;
extern	int minserve;		/* DU tunable: sever low water mark */
extern	int maxserve;		/* DU tunable: sever high water mark */
extern	int nservers;		/* total servers in system */
extern	int idleserver;		/* idle servers in system */
extern	int msglistcnt;		/* receive descriptors in msg queue */


struct minfo {
	unsigned long 	freemem[2]; 	/* freemem in pages */
					/* "double" long format	*/
					/* freemem[0] least significant */
	long	freeswap;	/* free swap space */
	long    vfault;  	/* translation fault */
	long    demand;		/*  demand zero and demand fill pages */
	long    swap;		/*  pages on swap */
	long    cache;		/*  pages in cache */
	long    file;		/*  pages on file */
	long    pfault;		/* protection fault */
	long    cw;		/*  copy on write */
	long    steal;		/*  steal the page */
	long    freedpgs;	/* pages are freed */
	long    unmodsw;	/* getpages finds unmodified pages on swap */
	long	unmodfl;	/* getpages finds unmodified pages in file */ 
	long	tlbpid;		/* new tlbpid used */
	long	tfault;		/* double tlb miss */
	long	rfault;		/* reference bit vfault */
};

extern struct minfo minfo;
extern struct syswait syswait;

struct syserr {
	long	inodeovf;
	long	fileovf;
	long	textovf;
	long	procovf;
};

extern struct syserr syserr;

struct shlbinfo {
	long	shlbs;		/* Max # of libs a process can link in	*/
				/*   at one time.			*/
	long	shlblnks;	/* # of times processes that have used	*/
				/*   shared libraries.			*/
	long	shlbovf;	/* # of processes needed more shlibs	*/
				/*   than the system imposed limit.	*/
	long	shlbatts;	/* # of times processes have attached	*/
				/*   run time libraries.		*/
	unsigned int shlbaddr;	/* starting address where shared	*/
				/*   libraries will be attached.	*/
};

extern struct shlbinfo shlbinfo;

#endif	_SYS_SYSINFO_
