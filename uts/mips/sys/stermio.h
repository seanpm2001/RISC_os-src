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
/* $Header: stermio.h,v 1.6.4.2 90/05/10 06:38:50 wje Exp $ */

#ifndef	_SYS_STERMIO_
#define	_SYS_STERMIO_	1


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * ioctl commands for control channels
 */
#define STSTART		1	/* start protocol */
#define STHALT		2	/* cease protocol */
#define STPRINT		3	/* assign device to printer */
#define STENABLE	4	/* enable polling */
#define STDISABLE	5	/* disable polling */
#define STPOLL		6	/* set polling rate */
#define STCNTRS		7	/* poke for status reports */
#define STTCHAN		8	/* set trace channel number */

/*
 * ioctl commands for terminal and printer channels
 */
#define STGET	(('X'<<8)|0)	/* get line options */
#define STSET	(('X'<<8)|1)	/* set line options */
#define	STTHROW	(('X'<<8)|2)	/* throw away queued input */
#define	STWLINE	(('X'<<8)|3)	/* get synchronous line # */
#define STTSV	(('X'<<8)|4)	/* get all line information */

struct stio {
	unsigned short	ttyid;
	char		row;
	char		col;
	char		orow;
	char		ocol;
	char		tab;
	char		aid;
	char		ss1;
	char		ss2;
	unsigned short	imode;
	unsigned short	lmode;
	unsigned short	omode;
};

/*
**	Mode Definitions.
*/
#define	STFLUSH	00400	/* FLUSH mode; lmode */
#define	STWRAP	01000	/* WRAP mode; lmode */
#define	STAPPL	02000	/* APPLICATION mode; lmode */

struct sttsv {
	char	st_major;
	short	st_pcdnum;
	char	st_devaddr;
	int	st_csidev;
};

struct stcntrs {
	char	st_lrc;
	char	st_xnaks;
	char	st_rnaks;
	char	st_xwaks;
	char	st_rwaks;
	char	st_scc;
};

/* trace message definitions */

#define LOC	113	/* loss of carrier */


#endif	_SYS_STERMIO_
