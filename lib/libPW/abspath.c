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
#ident	"$Header: abspath.c,v 1.5.2.3 90/05/10 01:07:17 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *abspath(p)

char *p;

{
int state;
int slashes;
char *stktop;
char *slash="/";
char *inptr;
char pop();
char c;

	state = 0;
	stktop = inptr = p;
	while (c = *inptr)
		{
		 switch (state)
			{
			 case 0: if (c=='/') state = 1;
				 push(&inptr,&stktop);
				 break;
			 case 1: if (c=='.') state = 2;
					else state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 2:      if (c=='.') state = 3;
				 else if (c=='/') state = 5;
				 else             state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 3: if (c=='/') state = 4;
					else state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 4: for (slashes = 0; slashes < 3; )
					{
					 if(pop(&stktop)=='/') ++slashes;
					 if (stktop < p) return((char *)-1);
					}
				 push(&slash,&stktop);
				 slash--;
				 state = 1;
				 break;
			 case 5: pop(&stktop);
				 if (stktop < p) return((char *)-1);
				 pop(&stktop);
				 if (stktop < p) return((char *)-1);
				 state = 1;
				 break;
			}
		}
	*stktop='\0';
	return(p);
}

push(chrptr,stktop)

char **chrptr;
char **stktop;

{
	**stktop = **chrptr;
	(*stktop)++;
	(*chrptr)++;
}

char pop(stktop)

char **stktop;

{
char chr;
	(*stktop)--;
	chr = **stktop;
	return(chr);
}	
