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
#ident	"$Header: zioctl.c,v 1.5.2.2 90/05/10 02:45:10 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	ioctl(2) with error checking
*/

#include <stdio.h>
#include "errmsg.h"

int
zioctl( severity, fildes, request, arg)
int	severity;
int 	fildes;
int	request;
{
	int	rc;

	if( (rc = ioctl(fildes, request, arg)) == -1 )
		_errmsg("UXzioctl1", severity,
		       "Cannot control device; fildes = %d request = %d arg = %d.",
			fildes, request, arg);

	return  rc;
}
