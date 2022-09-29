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
#ident	"$Header: eaccess.c,v 1.4.2.2 90/05/09 16:25:45 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* eaccess(name, mode) -- determine accessibility of named file with respect
	to mode.  This routine performs the same function as access(2),
	but with respect to the effective user and group ids.
*/

#include	"lp.h"


int
eaccess(name, mode)
char *name;
int mode;
{
	struct stat buf;
	int perm, euid;

	if(stat(name, &buf) == -1)
		return(-1);

	if((buf.st_mode & S_IFMT) == S_IFDIR && !(mode & ACC_DIR))
		return(-1);

	if((euid = geteuid()) == 0)	/* ROOT */
		return(0);

	if(euid == buf.st_uid)
		perm = buf.st_mode;
	else if(getegid() == buf.st_gid)
		perm = (buf.st_mode & 070) << 3;
	else
		perm = (buf.st_mode & 07) << 6;

	if( ((mode & ACC_R) && !(perm & S_IREAD)) ||
	    ((mode & ACC_W) && !(perm & S_IWRITE)) ||
	    ((mode & ACC_X) && !(perm & S_IEXEC)) )
		return(-1);
	else
		return(0);
}
