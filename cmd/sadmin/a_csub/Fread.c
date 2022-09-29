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
#ident	"$Header: Fread.c,v 1.5.2.2 90/05/09 18:29:30 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	
	fread(3S) with error checking
	Incomplete reads are considered errors.
	Reading End-Of-File or no items (nitem == 0) or zero-length items
	(size == 0) are considered successful reads.
*/

#include	<stdio.h>
#include	<cmderr.h>

Fread( ptr, size, nitems, stream )
char	*ptr;
int	size, nitems;
FILE	*stream;	/* file pointer */
{
	register int	countin;
	extern int	errno;

	if( feof( stream) )
		return 0;
	if( (countin = fread( ptr, size, nitems, stream )) != nitems  &&
		countin  &&  size ) {

		cmderr( CERROR, "Fread() asked for %d %d-byte item(s), got %d",
			nitems, size, countin );
	}
	return  countin;
}
