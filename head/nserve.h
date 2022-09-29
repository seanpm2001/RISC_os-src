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
 * |         928 Arques Avenue                                 |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: nserve.h,v 1.7.3.2 90/05/10 01:02:38 wje Exp $ */

#ifndef	_NSERVE_
#define	_NSERVE_	1


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <sys/nserve.h>
/*
 *
 *	name server header file.  contains defines necessary for
 *	any program that wants to talk with the name server.
 *
 */
#define NSVERSION	1	/* coincides with load n7 SVR3		*/

/*
 * Pathname defines:
 * NSDIRQ and NSDEV are set up to be combined with other #defines
 * The missing trailing quote (") on these defines is correct, as are the
 * missing leading quotes on the other defines in this group.
 * NOTE: don't put a comment after the NSDIRQ or NSDEV defines.
 */
#define NSDEV	  "/usr/nserve
#define NSDIRQ	  "/usr/nserve
#define NSPID	  NSDEV/nspid"	     /* lock file for ns, also has pid	*/
#define NS_PIPE   NSDEV/nspip"	     /* stream pipe for ns		*/
#define NSDIR	  NSDIRQ"	     /* name server working directory.	*/
#define NETMASTER NSDIRQ/rfmaster"   /* master file for nudnix network	*/
#define DOMMASTER NSDIRQ/dom.master" /* file for outside domains 	*/
#define SAVEDB	  NSDIRQ/save.db"    /* saves database when ns exits	*/
#define PASSFILE  NSDIRQ/loc.passwd" /* location of local passwd	*/
#define VERPASSWD NSDIRQ/verify/%s/passwd"  /* passwd for verification  */
#define DOMPASSWD NSDIRQ/auth.info/%s/passwd"  /* passwd for dom  "%s"  */
				     /* contains the domain name.	*/
#define NSDOM	  NSDIRQ/domain"     /* place to save domain name	*/

/* other defines	*/
#define PRIMENAME "PRIMARY"	/* default name for unknown primary	*/
#define CORRECT	  "correct"	/* message stating password is correct	*/
#define INCORRECT "sorry"	/* message stating password is wrong	*/
#define ADVFILE	  "/etc/advtab"
#define TEMPADV	  "/etc/tmpadv"
#define REC_TIME  10

/* sizes	*/

#define	SZ_RES	14	/* maximum size of resource name 		     */
#define SZ_MACH	9	/* maximum size of machine name.		     */
#define SZ_PATH 64	/* size of "pathname" of resource		     */
#define SZ_DESC	32	/* size of description				     */
#define SZ_DELEMENT 14	/* max size of one element of a domain name	     */

/* types	*/
#define FILE_TREE 1	/* sharable file tree	*/

/* flags	*/
#define READONLY  1
#define READWRITE 2

/* command types	*/
#define NS_SNDBACK	0	/* actually a "null" type	*/
#define	NS_ADV		1	/* advertise a resource		*/
#define NS_UNADV	2	/* unadvertise a resource	*/
#define NS_GET		3	/* unused query			*/
#define NS_QUERY	4	/* query by name and type.	*/
#define NS_INIT		5	/* called to setup name service */
#define NS_SENDPASS	7	/* register passwd w/ primary	*/
#define NS_VERIFY	8	/* verify validity of your pwd	*/
#define NS_MODADV	9	/* modify an advertisement	*/
#define NS_BYMACHINE	10	/* inv query by owner and type	*/
#define NS_IQUERY	11	/* general inverse query	*/
#define NS_IM_P		12	/* I am primary poll		*/
#define NS_IM_NP	13	/* I am secondary poll		*/
#define NS_FINDP	14	/* find which machine is prime	*/
#define NS_REL		15	/* relinquish being primary	*/

/* return codes	*/
#define FAILURE		0
#define SUCCESS		1
#define MORE_DATA 	2	/* or'ed into return when more coming */

/* error codes returned by the name server 			*/

#define R_NOERR  0	/* no error				*/
#define R_FORMAT 1	/* format error				*/
#define R_NSFAIL 2	/* name server failure			*/
#define R_NONAME 3	/* name does not exist			*/
#define R_IMP	 4	/* request type not implemented or bad	*/
#define R_PERM	 5	/* no permission for this operation	*/
#define R_DUP	 6	/* name not unique (for advertise)	*/
#define R_SYS	 7	/* a system call failed in name server  */
#define R_EPASS  8	/* error accessing primary passwd file	*/
#define R_INVPW  9   	/* invalid password			*/
#define R_NOPW   10	/* no passwd in primary passwd file	*/
#define R_SETUP  11	/* error in ns_setup()			*/
#define R_SEND   12	/* error in ns_send()			*/
#define R_RCV    13	/* error in ns_rcv()			*/
#define R_INREC	 14	/* in recovery, try again		*/
#define R_FAIL	 15	/* unknown failure			*/

/* name server request structure	*/

struct nssend {
	short	ns_code;	/* request code (e.g., NS_ADV)	*/
	short	ns_type;	/* type of data, unused for now	*/
	short	ns_flag;	/* read/write flag		*/
	char	*ns_name;	/* name of resource		*/
	char	*ns_desc;	/* description of resource	*/
	char	*ns_path;	/* local pathname of resource	*/
	struct address	*ns_addr; /* address of resource's owner*/
	char	**ns_mach;	/* list of client machines	*/
};

/* function declarations	*/

int	ns_setup();
int	ns_close();
int	ns_send();
struct nssend	*ns_rcv();

/* macros	*/
#define	SET_NODELAY(f)	fcntl(f,F_SETFL,fcntl(f,F_GETFL,0)|O_NDELAY)
#define	CLR_NODELAY(f)	fcntl(f,F_SETFL,fcntl(f,F_GETFL,0)&~O_NDELAY)

#endif	_NSERVE_
