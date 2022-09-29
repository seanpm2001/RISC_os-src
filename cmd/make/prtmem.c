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
#ident	"$Header: prtmem.c,v 1.2.2.2 90/05/09 16:45:10 wje Exp $"
#ifdef GETU
#define udsize uu[0]
#define ussize uu[1]
#endif


prtmem()
{
#ifdef GETU
#include "stdio.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/user.h"
	unsigned uu[2];
	register int i;

	if(getu( &((struct user *)0)->u_dsize, &uu, sizeof uu) > 0)
	{
		udsize *= 64;
		ussize *= 64;
		printf("mem: data = %u(0%o) stack = %u(0%o)\n",
			udsize, udsize, ussize, ussize);
/*
 *	The following works only when `make' is compiled
 *	with I&D space separated (i.e. cc -i ...).
 *	(Notice the hard coded `65' below!)
 */
		udsize /= 1000;
		ussize /= 1000;
		printf("mem:");
		for(i=1; i<=udsize;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf("d");
		}
		for(;i<=65-ussize;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf(".");
		}
		for(;i<=65;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf("s");
		}
		printf("\n");
		fflush(stdout);
	}
#endif
}
