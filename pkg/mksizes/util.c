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
#ident	"$Header: util.c,v 1.2.2.2 90/05/10 03:58:24 wje Exp $"
static char rcs[]="$Header: util.c,v 1.2.2.2 90/05/10 03:58:24 wje Exp $";

#include "mksizes.h"

error(etype, s)
int etype;
char *s;
{
  static char *typstr[] = { "warning", "error", "fatal"};
  char errstr[1024];

  sprintf(errstr, "%s: %s", typstr[etype], s);
  errmsg(errstr);
  if (etype == ERRTYPE_FATAL)
    exit(1);
}

errmsg(s)
char *s;
{
  fprintf(stderr, "mksizes: %s\n", s);
}

usage()
{
  errmsg(
  "usage: mksizes bomfile ...");

  exit(1);
}

strcpyfld(to, from, len)
char **to, **from;
int len;
{
  while((**from) && (! isspace(**from)) && len--)
    *(*to)++ = *(*from)++;
  if (len)
    {
      **to = '\0';
      return (1);
    }
  else
    return(0);
}

