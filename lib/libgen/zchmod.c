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
#ident	"$Header: zchmod.c,v 1.5.2.2 90/05/10 02:42:22 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	chmod(2) with error checking
*/

#include	"errmsg.h"

int
zchmod( severity, path, mode )
int	 severity;
char	*path;
int	 mode;
{

	int	err_ind;

	if( (err_ind = chmod(path, mode )) == -1 )
	    _errmsg ( "UXzchmod1", severity,
		  "Cannot change the mode of file \"%s\" to %d.",
		   path, mode);

	return err_ind;
}
