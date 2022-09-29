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
#ident	"$Header: mntent.c,v 1.2.1.2 90/05/07 20:39:23 wje Exp $"

#ifndef lint
static	char sccsid[] = "@(#)mntent.c 1.1 86/02/03 SMI";
/* @(#)mntent.c	2.1 86/04/11 NFSSRC */
#endif

#include <stdio.h>
#include <ctype.h>
#ifdef SYSTYPE_BSD43
#include <mntent.h>
#include <sys/file.h>
#endif
#ifdef SYSTYPE_SYSV
#include <sun/mntent.h>
#include <bsd/sys/file.h>
#endif

static	struct mntent mnt;
static	char line[BUFSIZ+1];

static char *
mntstr(p)
	register char **p;
{
	char *cp = *p;
	char *retstr;

	while (*cp && isspace(*cp))
		cp++;
	retstr = cp;
	while (*cp && !isspace(*cp))
		cp++;
	if (*cp) {
		*cp = '\0';
		cp++;
	}
	*p = cp;
	return (retstr);
}

static int
mntdigit(p)
	register char **p;
{
	register int value = 0;
	char *cp = *p;
	char *retstr;

	while (*cp && isspace(*cp))
		cp++;
	for (; *cp && isdigit(*cp); cp++) {
		value *= 10;
		value += *cp - '0';
	}
	while (*cp && !isspace(*cp))
		cp++;
	if (*cp) {
		*cp = '\0';
		cp++;
	}
	*p = cp;
	return (value);
}

static
mnttabscan(mnttabp, mnt)
	FILE *mnttabp;
	struct mntent *mnt;
{
	char *cp;

	do {
		cp = fgets(line, 256, mnttabp);
		if (cp == NULL) {
			return (EOF);
		}
	} while (*cp == '#');
	mnt->mnt_fsname = mntstr(&cp);
	if (*cp == '\0')
		return (1);
	mnt->mnt_dir = mntstr(&cp);
	if (*cp == '\0')
		return (2);
	mnt->mnt_type = mntstr(&cp);
	if (*cp == '\0')
		return (3);
	mnt->mnt_opts = mntstr(&cp);
	if (*cp == '\0')
		return (4);
	mnt->mnt_freq = mntdigit(&cp);
	if (*cp == '\0')
		return (5);
	mnt->mnt_passno = mntdigit(&cp);
	return (6);
}
	
FILE *
setmntent(fname, flag)
	char *fname;
	char *flag;
{
	FILE *mnttabp;
	int lock;

	if ((mnttabp = fopen(fname, flag)) == NULL) {
		return (NULL);
	}
	lock = LOCK_SH;
	while (*flag) {
		if (*flag == 'w' || *flag == 'a' || *flag == '+') {
			lock = LOCK_EX;
		}
		flag++;
	}
	if (flock(fileno(mnttabp), lock) < 0) {
		fclose(mnttabp);
		return (NULL);
	}
	return (mnttabp);
}

int
endmntent(mnttabp)
	FILE *mnttabp;
{

	if (mnttabp) {
		fclose(mnttabp);
	}
	return (1);
}

struct mntent *
getmntent(mnttabp)
	FILE *mnttabp;
{
	int nfields;

	if (mnttabp == 0)
		return ((struct mntent *)0);
	nfields = mnttabscan(mnttabp, &mnt);
	if (nfields == EOF || nfields != 6)
		return ((struct mntent *)0);
	return (&mnt);
}

addmntent(mnttabp, mnt)
	FILE *mnttabp;
	register struct mntent *mnt;
{
	if (fseek(mnttabp, 0, 2) < 0) {
		return (1);
	}
	mntprtent(mnttabp, mnt);
	return (0);
}

static char tmpopts[256];

static char *
mntopt(p)
	char **p;
{
	char *cp = *p;
	char *retstr;

	while (*cp && isspace(*cp))
		cp++;
	retstr = cp;
	while (*cp && *cp != ',')
		cp++;
	if (*cp) {
		*cp = '\0';
		cp++;
	}
	*p = cp;
	return (retstr);
}

char *
hasmntopt(mnt, opt)
	register struct mntent *mnt;
	register char *opt;
{
	char *f, *opts;

	strcpy(tmpopts, mnt->mnt_opts);
	opts = tmpopts;
	f = mntopt(&opts);
	for (; *f; f = mntopt(&opts)) {
		if (strncmp(opt, f, strlen(opt)) == 0)
			return (f - tmpopts + mnt->mnt_opts);
	} 
	return (NULL);
}

static
mntprtent(mnttabp, mnt)
	FILE *mnttabp;
	register struct mntent *mnt;
{
	fprintf(mnttabp, "%s %s %s %s %d %d\n",
	    mnt->mnt_fsname,
	    mnt->mnt_dir,
	    mnt->mnt_type,
	    mnt->mnt_opts,
	    mnt->mnt_freq,
	    mnt->mnt_passno);
	return(0);
}
