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
#ident	"$Header: cmd4.c,v 1.4.1.2 90/05/09 16:38:35 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#

#include "rcv.h"
#include <errno.h>

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * More commands..
 */


/*
 * pipe messages to cmd.
 */

dopipe(str)
	char str[];
{
	register int *ip, mesg;
	register struct message *mp;
	char *cp, *cmd;
	int f, *msgvec, lc, t, nowait=0;
	long cc;
	register int pid;
	int page, s, pivec[2], (*sigint)();
	char *Shell;
	FILE *pio;

	msgvec = (int *) salloc((msgCount + 2) * sizeof *msgvec);
	if ((cmd = snarf(str, &f, 0)) == NOSTR) {
		if (f == -1) {
			printf("pipe command error\n");
			return(1);
			}
		if ( (cmd = value("cmd")) == NOSTR) {
			printf("\"cmd\" not set, ignored.\n");
			return(1);
			}
		}
	if (!f) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == NULL) {
			printf("No messages to pipe.\n");
			return(1);
		}
		msgvec[1] = NULL;
	}
	if (f && getmsglist(str, msgvec, 0) < 0)
		return(1);
	if (*(cp=cmd+strlen(cmd)-1)=='&'){
		*cp=0;
		nowait++;
		}
	if ((cmd = expand(cmd)) == NOSTR)
		return(1);
	printf("Pipe to: \"%s\"\n", cmd);
	flush();

					/*  setup pipe */
	if (pipe(pivec) < 0) {
		perror("pipe");
		/* signal(SIGINT, sigint) */
		return(0);
	}

	if ((pid = vfork()) == 0) {
		close(pivec[1]);	/* child */
		fclose(stdin);
		dup(pivec[0]);
		close(pivec[0]);
		setuid(getuid());
		setgid(getgid());
		if ((Shell = value("SHELL")) == NOSTR || *Shell=='\0')
			Shell = SHELL;
		execlp(Shell, Shell, "-c", cmd, 0);
		perror(Shell);
		_exit(1);
	}
	if (pid == -1) {		/* error */
		perror("fork");
		close(pivec[0]);
		close(pivec[1]);
		return(0);
	}

	close(pivec[0]);		/* parent */
	pio=fdopen(pivec[1],"w");

					/* send all messages to cmd */
	page = (value("page")!=NOSTR);
	cc = 0L;
	lc = 0;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		if ((t = send(mp, pio)) < 0) {
			perror(cmd);
			return(1);
		}
		lc += t;
		cc += mp->m_size;
		if (page) putc('\f', pio);
	}

	fflush(pio);
	if (ferror(pio))
	      perror(cmd);
	fclose(pio);

					/* wait */
	if (!nowait){
		while (wait(&s) != pid);
		s &= 0377;
		if (s != 0) {
			printf("Pipe to \"%s\" failed\n", cmd);
			goto err;
		}
	}

	printf("\"%s\" %d/%ld\n", cmd, lc, cc);
	return(0);

err:
	/* signal(SIGINT, sigint); */
	return(0);
}
