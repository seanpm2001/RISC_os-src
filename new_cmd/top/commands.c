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
#ident	"$Header: commands.c,v 1.3.1.2 90/05/10 03:40:39 wje Exp $"

/*
 *  Top users display for Berkeley Unix
 *
 *  This file contains the routines that implement some of the interactive
 *  mode commands.  Note that some of the commands are implemented in-line
 *  in "main".  This is necessary because they change the global state of
 *  "top" (i.e.:  changing the number of processes to display).
 */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "sigdesc.h"		/* generated automatically */
#include "boolean.h"
#ifdef RISCOS
#include <bsd43/sys/syscall.h>
#define setpriority(x,y,z) syscall(BSD43_SYS_setpriority,(x),(y),(z))
#endif RISCOS

extern int  errno;
extern int  sys_nerr;
extern char *sys_errlist[];

extern char *copyright;

int err_compar();
char *err_string();
char *index();

/*
 *  show_help() - display the help screen; invoked in response to
 *		either 'h' or '?'.
 */

show_help()

{
    fputs(copyright, stdout);
    fputs("\n\n\
A top users display for Unix\n\
\n\
These single-character commands are available:\n\
\n\
^L      - redraw screen\n\
q       - quit\n\
h or ?  - help; show this text\n\
d       - change number of displays to show\n\
e       - list errors generated by last \"kill\" or \"renice\" command\n\
k       - kill processes; send a signal to a list of processes\n\
n or #  - change number of processes to display\n\
r       - renice a process\n\
s       - change number of seconds to delay between updates\n\
\n\
\n", stdout);
}

/*
 *  Utility routines that help with some of the commands.
 */

char *next_field(str)

register char *str;

{
    register char *temp;

    if ((str = index(str, ' ')) == NULL)
    {
	return(NULL);
    }
    *str = '\0';
    while (*++str == ' ') /* loop */;
    return(str);
}

scanint(str, intp)

char *str;
int  *intp;

{
    register int val = 0;
    register char ch;

    while ((ch = *str++) != '\0')
    {
	if (isdigit(ch))
	{
	    val = val * 10 + (ch - '0');
	}
	else if (isspace(ch))
	{
	    break;
	}
	else
	{
	    return(-1);
	}
    }
    *intp = val;
    return(0);
}

/*
 *  Some of the commands make system calls that could generate errors.
 *  These errors are collected up in an array of structures for later
 *  contemplation and display.  Such routines return a string containing an
 *  error message, or NULL if no errors occurred.  The next few routines are
 *  for manipulating and displaying these errors.  We need an upper limit on
 *  the number of errors, so we arbitrarily choose 20.
 */

#define ERRMAX 20

struct errs		/* structure for a system-call error */
{
    int  errno;		/* value of errno (that is, the actual error) */
    char *arg;		/* argument that caused the error */
};

static struct errs errs[ERRMAX];
static int errcnt;
static char *err_toomany = " too many errors occurred";
static char *err_listem = 
	" Many errors occurred.  Press `e' to display the list of errors.";

/* These macros get used to reset and log the errors */
#define ERR_RESET   errcnt = 0
#define ERROR(p, e) if (errcnt >= ERRMAX) \
		    { \
			return(err_toomany); \
		    } \
		    else \
		    { \
			errs[errcnt].arg = (p); \
			errs[errcnt++].errno = (e); \
		    }

/*
 *  err_string() - return an appropriate error string.  This is what the
 *	command will return for displaying.  If no errors were logged, then
 *	return NULL.  The maximum length of the error string is defined by
 *	"STRMAX".
 */

#define STRMAX 80

char *err_string()

{
    register char *ptr;
    register struct errs *errp;
    register int  cnt = 0;
    register int  first = Yes;
    register int  currerr = -1;
    int stringlen;		/* characters still available in "string" */
    char string[STRMAX];

    /* if there are no errors, return NULL */
    if (errcnt == 0)
    {
	return(NULL);
    }

    /* sort the errors */
    qsort(errs, errcnt, sizeof(struct errs), err_compar);

    /* need a space at the from of the error string */
    string[0] = ' ';
    string[1] = '\0';
    stringlen = STRMAX - 2;

    /* loop thru the sorted list, building an error string */
    ptr = string;
    while (cnt < errcnt)
    {
	errp = &(errs[cnt++]);
	if (errp->errno != currerr)
	{
	    if (currerr != -1)
	    {
		if ((stringlen = str_adderr(string, stringlen, currerr)) < 2)
		{
		    return(err_listem);
		}
		strcat(string, "; ");		/* we know there's more */
	    }
	    currerr = errp->errno;
	    first = Yes;
	}
	if ((stringlen = str_addarg(string, stringlen, errp->arg, first)) ==0)
	{
	    return(err_listem);
	}
	first = No;
    }

    /* add final message */
    stringlen = str_adderr(string, stringlen, currerr);

    /* return the error string */
    return(stringlen == 0 ? err_listem : string);
}

/*
 *  str_adderr(str, len, err) - add an explanation of error "err" to
 *	the string "str".
 */

str_adderr(str, len, err)

char *str;
int len;
int err;

{
    register char *msg;
    register int  msglen;

    msg = err == 0 ? "Not a number" : sys_errlist[err];
    msglen = strlen(msg) + 2;
    if (len <= msglen)
    {
	return(0);
    }
    strcat(str, ": ");
    strcat(str, msg);
    return(len - msglen);
}

/*
 *  str_addarg(str, len, arg, first) - add the string argument "arg" to
 *	the string "str".  This is the first in the group when "first"
 *	is set (indicating that a comma should NOT be added to the front).
 */

str_addarg(str, len, arg, first)

char *str;
int  len;
char *arg;
int  first;

{
    register int arglen;

    arglen = strlen(arg);
    if (!first)
    {
	arglen += 2;
    }
    if (len <= arglen)
    {
	return(0);
    }
    if (!first)
    {
	strcat(str, ", ");
    }
    strcat(str, arg);
    return(len - arglen);
}

/*
 *  err_compar(p1, p2) - comparison routine used by "qsort"
 *	for sorting errors.
 */

err_compar(p1, p2)

register struct errs *p1, *p2;

{
    register int result;

    if ((result = p1->errno - p2->errno) == 0)
    {
	return(strcmp(p1->arg, p2->arg));
    }
    return(result);
}

/*
 *  error_count() - return the number of errors currently logged.
 */

error_count()

{
    return(errcnt);
}

/*
 *  show_errors() - display on stdout the current log of errors.
 */

show_errors()

{
    register int cnt = 0;
    register struct errs *errp = errs;

    printf("%d error%s:\n\n", errcnt, errcnt == 1 ? "" : "s");
    while (cnt++ < errcnt)
    {
	printf("%5s: %s\n", errp->arg,
	    errp->errno == 0 ? "Not a number" : sys_errlist[errp->errno]);
	errp++;
    }
}

/*
 *  kill_procs(str) - send signals to processes, much like the "kill"
 *		command does; invoked in response to 'k'.
 */

char *kill_procs(str)

char *str;

{
    register char *nptr;
    register char *optr;
    int signum = SIGTERM;	/* default */
    int procnum;
    char badnum = 0;
    struct sigdesc *sigp;

    ERR_RESET;
    if (str[0] == '-')
    {
	/* explicit signal specified */
	if ((optr = nptr = next_field(str)) == NULL)
	{
	    return(" kill: no processes specified");
	}

	if (isdigit(str[1]))
	{
	    scanint(str + 1, &signum);
	    if (signum <= 0 || signum >= NSIG)
	    {
		return(" invalid signal number");
	    }
	}
	else 
	{
	    /* translate the name into a number */
	    for (sigp = sigdesc; sigp->name != NULL; sigp++)
	    {
		if (strcmp(sigp->name, str + 1) == 0)
		{
		    signum = sigp->number;
		    break;
		}
	    }

	    /* was it ever found */
	    if (sigp->name == NULL)
	    {
		return(" bad signal name");
	    }
	}
	/* put the new pointer in place */
	str = nptr;
    }

    /* loop thru the string, killing processes */
    do
    {
	if (scanint(str, &procnum) == -1)
	{
	    ERROR(str, 0);
	}
	else if (kill(procnum, signum) == -1)
	{
	    /* chalk up an error */
	    ERROR(str, errno);
	}
    } while ((str = next_field(str)) != NULL);

    /* return appropriate error string */
    return(err_string());
}

/*
 *  renice_procs(str) - change the "nice" of processes, much like the
 *		"renice" command does; invoked in response to 'r'.
 */

char *renice_procs(str)

char *str;

{
    register char negate;
    int prio;
    int procnum;

    ERR_RESET;

    /* allow for negative priority values */
    if ((negate = *str == '-'))
    {
	/* move past the minus sign */
	str++;
    }

    /* use procnum as a temporary holding place and get the number */
    procnum = scanint(str, &prio);

    /* negate if necessary */
    if (negate)
    {
	prio = -prio;
    }

    /* check for validity */
    if (procnum == -1 || prio <= PRIO_MIN || prio >= PRIO_MAX)
    {
	return(" bad priority value");
    }

    /* move to the first process number */
    if ((str = next_field(str)) == NULL)
    {
	return(" no processes specified");
    }

    /* loop thru the process numbers, renicing each one */
    do
    {
	if (scanint(str, &procnum) == -1)
	{
	    ERROR(str, 0);
	}
	else if (setpriority(PRIO_PROCESS, procnum, prio) == -1)
	{
	    ERROR(str, errno);
	}
    } while ((str = next_field(str)) != NULL);

    /* return appropriate error string */
    return(err_string());
}

