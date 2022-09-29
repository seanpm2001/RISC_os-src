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
#ident	"$Header: getnetent.c,v 1.4.1.2 90/05/07 20:51:48 wje Exp $"

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/types.h>
#include <netdb.h>

setnetent(f)
	int f;
{
        char *(*vis_func)() = VIS_FIRST_CALL;
	int hp;

        while ((vis_func = vis_nextserv(vis_func, VIS_SETNETENT))
                != VIS_DONE) {
                hp = (int)((*vis_func)(VIS_SETNETENT, f));
		/* XXX
		if ((hp != -1) && (hp != 0))
                        return(hp);
		*/
        }
        return(0);
}

endnetent()
{
        char *(*vis_func)() = VIS_FIRST_CALL;
	int hp;

        while ((vis_func = vis_nextserv(vis_func, VIS_ENDNETENT))
                != VIS_DONE) {
                hp = (int)((*vis_func)(VIS_ENDNETENT, 0));
		/*
                if (hp != -1)
                        return(hp);
		*/
        }
        return(0);
}

struct netent *
getnetent()
{
        char *(*vis_func)() = VIS_FIRST_CALL;
	struct netent *hp;

        while ((vis_func = vis_nextserv(vis_func, VIS_GETNETENT))
                != VIS_DONE) {
                hp = (struct netent *)((*vis_func)(VIS_ENDNETENT, 0));
		if ((hp != (struct netent *)-1) && (hp != (struct netent *)0))
                        return(hp);
        }
        return((struct netent *)0);
}
