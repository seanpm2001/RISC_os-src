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
#ident	"$Header: install.c,v 1.5 90/07/25 16:54:07 jay Exp $"

/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)install.c	5.12 (Berkeley) 7/6/88";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <a.out.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <ctype.h>

#ifndef MIPSEBMAGIC_2
#define MIPSEBMAGIC_2	0x0163
#define MIPSELMAGIC_2	0x0166
#define SMIPSEBMAGIC_2	0x6301
#define SMIPSELMAGIC_2	0x6601
#endif

#define	YES	1			/* yes/true */
#define	NO	0			/* no/false */

#define	PERROR(head, msg) { \
	fputs(head, stderr); \
	perror(msg); \
}

static struct passwd	*pp;
static struct group	*gp;
static int	docopy, dostrip,
		mode = 0755;
static char	*group, *owner,
		pathbuf[MAXPATHLEN];

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	struct stat from_sb, to_sb;
	int ch, no_target;
	char *to_name;

	while ((ch = getopt(argc, argv, "cg:m:o:s")) != EOF)
		switch((char)ch) {
		case 'c':
			docopy = YES;
			break;
		case 'g':
			group = optarg;
			break;
		case 'm':
			mode = atoo(optarg);
			break;
		case 'o':
			owner = optarg;
			break;
		case 's':
			dostrip = YES;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc < 2)
		usage();

	/* get group and owner id's */
	if (group && !(gp = getgrnam(group))) {
		fprintf(stderr, "install: unknown group %s.\n", group);
		exit(1);
	}
	if (owner && !(pp = getpwnam(owner))) {
		fprintf(stderr, "install: unknown user %s.\n", owner);
		exit(1);
	}

	no_target = stat(to_name = argv[argc - 1], &to_sb);
	if (!no_target && (to_sb.st_mode & S_IFMT) == S_IFDIR) {
		for (; *argv != to_name; ++argv)
			install(*argv, to_name, YES);
		exit(0);
	}

	/* can't do file1 file2 directory/file */
	if (argc != 2)
		usage();

	if (!no_target) {
		if (stat(*argv, &from_sb)) {
			fprintf(stderr, "install: can't find %s.\n", *argv);
			exit(1);
		}
		if ((to_sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "install: %s isn't a regular file.\n", to_name);
			exit(1);
		}
		if (to_sb.st_dev == from_sb.st_dev && to_sb.st_ino == from_sb.st_ino) {
			fprintf(stderr, "install: %s and %s are the same file.\n", *argv, to_name);
			exit(1);
		}
		/* unlink now... avoid ETXTBSY errors later */
		(void)unlink(to_name);
	}
	install(*argv, to_name, NO);
	exit(0);
}

/*
 * install --
 *	build a path name and install the file
 */
static
install(from_name, to_name, isdir)
	char *from_name, *to_name;
	int isdir;
{
	struct stat from_sb;
	int devnull, from_fd, to_fd;
	char *C, *rindex();

	/* if try to install "/dev/null" to a directory, fails */
	if (isdir || strcmp(from_name, "/dev/null")) {
		if (stat(from_name, &from_sb)) {
			fprintf(stderr, "install: can't find %s.\n", from_name);
			exit(1);
		}
		if ((from_sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "install: %s isn't a regular file.\n", from_name);
			exit(1);
		}
		/* build the target path */
		if (isdir) {
			(void)sprintf(pathbuf, "%s/%s", to_name, (C = rindex(from_name, '/')) ? ++C : from_name);
			to_name = pathbuf;
		}
		devnull = NO;
	}
	else
		devnull = YES;

	/* unlink now... avoid ETXTBSY errors later */
	(void)unlink(to_name);

	/* create target */
	if ((to_fd = open(to_name, O_CREAT|O_WRONLY|O_TRUNC, 0)) < 0) {
		PERROR("install: ", to_name);
		exit(1);
	}
	if (!devnull) {
		if ((from_fd = open(from_name, O_RDONLY, 0)) < 0) {
			(void)unlink(to_name);
			PERROR("install: open: ", from_name);
			exit(1);
		}
		if (dostrip)
			strip(from_fd, from_name, to_fd, to_name);
		else
			copy(from_fd, from_name, to_fd, to_name);
		(void)close(from_fd);
		if (!docopy)
			(void)unlink(from_name);
	}
	/* set owner, group, mode for target */
	if (fchmod(to_fd, mode)) {
		PERROR("install: fchmod: ", to_name);
		bad();
	}
	if ((group || owner) && fchown(to_fd, owner ? pp->pw_uid : -1,
	    group ? gp->gr_gid : -1)) {
		PERROR("install: fchown: ", to_name);
		bad();
	}
	(void)close(to_fd);
}

/*
 * strip --
 *	copy file, strip(1)'ing it at the same time
 */
#ifdef RISCOS
/* MIPS Extended Coff is enough different that the stripping on the fly
 * must be changed completely.  I tried just to call strip after the copy,
 * but that won't work, e.g. what if you install something in 555 mode,
 * you can't strip it 'cause it's not writable anymore.  You can't do it
 * before the fchmod (above) either 'cause the open modes are wrong.
 * Sooo, the code in this routine is lifted from the cc2.0 version of
 * strip. (usr/src/cmplrs/cc2.0/ldutils/strip.c)  Jay 1/9/90
 */
static
strip(from_fd, from_name, to_fd, to_name)
	register int from_fd, to_fd;
	char *from_name, *to_name;
{
#include "filehdr.h"
#include "sex.h"
	FILHDR hdr;
	int nread;
	int swapit;
	swapit = 0;
	lseek(from_fd,0L,L_SET);
	if ((nread = read (from_fd, &hdr, sizeof(hdr))) != sizeof (hdr)) {
	    if (nread < 0) {
		    PERROR("install: ",from_name);
	    } else {
		    fprintf(stderr, 
"install: Error: %s is not an object file-- can't read header\n", from_name);
	    }
	    bad();
	} /* if */

	switch (hdr.f_magic) {

	case SMIPSEBMAGIC:
	case SMIPSELMAGIC:
	case SMIPSEBMAGIC_2:
	case SMIPSELMAGIC_2:
	    swapit=1;
	    swap_filehdr (&hdr, gethostsex());
	case MIPSEBMAGIC:
	case MIPSELMAGIC:
	case MIPSEBMAGIC_2:
	case MIPSELMAGIC_2:
	case MIPSEBUMAGIC:
	case MIPSELUMAGIC:
	    break;
	default:
	    (void) fprintf (stderr, "install:  %s not in a.out format.\n", from_name);
	    bad();
	} 

	if (hdr.f_symptr == 0 || hdr.f_nsyms == 0) {
	    (void) fprintf(stderr, "install: Info: %s already stripped\n", from_name);
	}
	lseek(from_fd,0L,L_SET);
	if (forward(from_fd, to_fd, hdr.f_symptr) != hdr.f_symptr) {
	    fprintf(stderr, "install: Error: cannot strip %s\n", from_name);
	    bad();
	} /* if */


	/* seek to rewrite header */
	if (lseek(to_fd, 0L, L_SET) < 0) {
	    PERROR("install: ",to_name);
	    bad();
	} /* if */

	hdr.f_symptr = hdr.f_nsyms = 0;
	if (swapit != 0) {
	    swap_filehdr (&hdr, gethostsex() == BIGENDIAN ? LITTLEENDIAN :
		BIGENDIAN);
	}
	if (write (to_fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
	    PERROR("install: ",to_name);
	    bad();
	} /* if */
}
forward (fin, fout, bytes)

int	fin;
int	fout;
int	bytes;

{
    char	buf[BUFSIZ];
    int		total;
    int		nread;
    int		nwrite;

    total = 0;
    while (bytes > 0) {

	nread = (bytes > BUFSIZ) ? BUFSIZ : bytes;
	nread = read (fin, buf, nread);
	if (nread < 0)
	    return (nread);
	else if (nread == 0)
	    return (total);

	nwrite = write (fout, buf, nread);
	if (nwrite != nread)
	    return (nwrite);

	total += nwrite;
	bytes -= nwrite;
    } /* while */
    return (total);
} /* forward */
#else
static
strip(from_fd, from_name, to_fd, to_name)
	register int from_fd, to_fd;
	char *from_name, *to_name;
{
	typedef struct exec EXEC;
	register long size;
	register int n;
	EXEC head;
	char buf[MAXBSIZE];
	off_t lseek();

	if (read(from_fd, (char *)&head, sizeof(head)) < 0 || N_BADMAG(head)) {
		fprintf(stderr, "install: %s not in a.out format.\n", from_name);
		bad();
	}
	if (head.a_syms || head.a_trsize || head.a_drsize) {
		size = (long)head.a_text + head.a_data;
		head.a_syms = head.a_trsize = head.a_drsize = 0;
		if (head.a_magic == ZMAGIC)
			size += getpagesize() - sizeof(EXEC);
		if (write(to_fd, (char *)&head, sizeof(EXEC)) != sizeof(EXEC)) {
			PERROR("install: write: ", to_name);
			bad();
		}
		for (; size; size -= n)
			/* sizeof(buf) guaranteed to fit in an int */
			if ((n = read(from_fd, buf, (int)MIN(size, sizeof(buf)))) <= 0)
				break;
			else if (write(to_fd, buf, n) != n) {
				PERROR("install: write: ", to_name);
				bad();
			}
		if (size) {
			fprintf(stderr, "install: read: %s: premature EOF.\n", from_name);
			bad();
		}
		if (n == -1) {
			PERROR("install: read: ", from_name);
			bad();
		}
	}
	else {
		(void)lseek(from_fd, 0L, L_SET);
		copy(from_fd, from_name, to_fd, to_name);
	}
}
#endif

/*
 * copy --
 *	copy from one file to another
 */
static
copy(from_fd, from_name, to_fd, to_name)
	register int from_fd, to_fd;
	char *from_name, *to_name;
{
	register int n;
	char buf[MAXBSIZE];

	while ((n = read(from_fd, buf, sizeof(buf))) > 0)
		if (write(to_fd, buf, n) != n) {
			PERROR("install: write: ", to_name);
			bad();
		}
	if (n == -1) {
		PERROR("install: read: ", from_name);
		bad();
	}
}

/*
 * atoo --
 *	octal string to int
 */
static
atoo(str)
	register char *str;
{
	register int val;

	for (val = 0; isdigit(*str); ++str)
		val = val * 8 + *str - '0';
	return(val);
}

/*
 * bad --
 *	remove created target and die
 */
static
bad()
{
	(void)unlink(pathbuf);
	exit(1);
}

/*
 * usage --
 *	print a usage message and die
 */
static
usage()
{
	fputs("usage: install [-cs] [-g group] [-m mode] [-o owner] file1 file2;\n\tor file1 ... fileN directory\n", stderr);
	exit(1);
}
