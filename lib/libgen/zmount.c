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
#ident	"$Header: zmount.c,v 1.5.2.2 90/05/10 02:46:11 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	mount(2) with error checking
*/

#include	"errmsg.h"

int
zmount( severity, spec, dir, rwflag )
int	 severity;
char	*spec;
char	*dir;
int	 rwflag;
{

	int	err_ind;

	if( (err_ind = mount(spec, dir, rwflag )) == -1 )
	    _errmsg ( "UXzmount1", severity,
		  "Cannot mount file system on block \"%s\" to directory \"%s\" , rwflag=%d.",
		   spec, dir, rwflag);

	return err_ind;
}
