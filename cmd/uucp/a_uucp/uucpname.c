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
#ident	"$Header: uucpname.c,v 1.4.2.2 90/05/10 00:37:48 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#include "uucp.h"

/*
 * get the uucp name
 * return:
 *	none
 */
void
uucpname(name)
register char *name;
{
	char *s;

#ifdef BSD4_2
	int nlen;
	char	NameBuf[MAXBASENAME + 1];

	/* This code is slightly wrong, at least if you believe what the */
	/* 4.1c manual says.  It claims that gethostname's second parameter */
	/* should be a pointer to an int that has the size of the buffer. */
	/* The code in the kernel says otherwise.  The manual also says that */
	/* the string returned is null-terminated; this, too, appears to be */
	/* contrary to fact.  Finally, the variable containing the length */
	/* is supposed to be modified to have the actual length passed back; */
	/* this, too, doesn't happen.  So I'm zeroing the buffer first, and */
	/* passing an int, not a pointer to one.  *sigh*

	/*		--Steve Bellovin	*/
	bzero(NameBuf, sizeof NameBuf);
	nlen = sizeof NameBuf;
	gethostname(NameBuf, nlen);
	s = NameBuf;
	s[nlen] = '\0';
#else /* !BSD4_2 */
#ifdef UNAME
	struct utsname utsn;

	uname(&utsn);
	s = utsn.nodename;
#else /* !UNAME */
	char	NameBuf[MAXBASENAME + 1], *strchr();
	FILE	*NameFile;

	s = MYNAME;
	NameBuf[0] = '\0';

	if ((NameFile = fopen("/etc/whoami", "r")) != NULL) {
		/* etc/whoami wins */
		(void) fgets(NameBuf, MAXBASENAME + 1, NameFile);
		(void) fclose(NameFile);
		NameBuf[MAXBASENAME] = '\0';
		if (NameBuf[0] != '\0') {
			if ((s = strchr(NameBuf, '\n')) != NULL)
				*s = '\0';
			s = NameBuf;
		}
	}
#endif /* UNAME */
#endif /* BSD4_2 */

	(void) strncpy(name, s, MAXBASENAME);
	name[MAXBASENAME] = '\0';
	return;
}
