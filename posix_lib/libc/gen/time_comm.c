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
#ident	"$Header: time_comm.c,v 1.1.1.5 90/05/10 20:28:02 wje Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Functions that are common to ctime(3C) and cftime(3C)
 */
#include	<ctype.h>
#include  	<stdio.h>
#include  	<sys/types.h>
#include 	<time.h>

#define	SEC_IN_MIN	(60)
#define	SEC_IN_HOUR	(60 * SEC_IN_MIN)
#define	SEC_IN_DAY	(24 * SEC_IN_HOUR)
#define MAXTZNAME	3
#define	year_size(A)	(((A) % 4) ? 365 : 366)

time_t	timezone = 0;
time_t	altzone = 0;
int 	daylight = 0;
extern char 	*getenv();

static time_t 	start_dst ;    /* Start date of alternate time zone */
static time_t  	end_dst;      /* End date of alternate time zone */
char 		*tzname[] = {"GMT","   "};
static short	month_size[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

struct tm *
localtime(tim)
time_t	*tim;
{
	register struct tm *ct;
	static void getusa();
	long daybegin, dayend;
	time_t	copyt, curr;

	tzset();
	copyt = *tim - timezone;
	ct = gmtime(&copyt);
	if (!daylight)
		return(ct);

	curr = ct->tm_yday*SEC_IN_DAY + ct->tm_hour*60*60 + ct->tm_min*60 + ct->tm_sec;

	if ( start_dst == 0 && end_dst == 0)
		getusa(&daybegin, &dayend, ct);
	else
	{
		daybegin = start_dst;
		dayend  = end_dst;
	}
	if (curr >= daybegin && curr < dayend)
	{
		copyt = *tim - altzone;
		ct = gmtime(&copyt);
		ct->tm_isdst = 1;
	}
	return(ct);
}

struct tm *
gmtime(tim)
time_t	*tim;
{
	register int		d0, d1;
	long			hms, day;
	static struct tm	xtime;

	/* Break initial number into days */
	hms = *tim % SEC_IN_DAY;
	day = *tim / SEC_IN_DAY;
	if(hms < 0) {
		hms += SEC_IN_DAY;
		day -= 1;
	}

	/* Generate hours:minutes:seconds */
	xtime.tm_sec = hms % 60;
	d1 = hms / 60;
	xtime.tm_min = d1 % 60;
	d1 /= 60;
	xtime.tm_hour = d1;

	/* Generate day of the week. The addend is 4 mod 7 (1/1/1970 was Thursday) */

	xtime.tm_wday = (day + 7340036L) % 7;

	/* Generate year number */
	if(day >= 0)
		for(d1=70; day >= year_size(d1); d1++)
			day -= year_size(d1);
	else
		for(d1=70; day < 0; d1--)
			day += year_size(d1-1);
	xtime.tm_year = d1;
	xtime.tm_yday = d0 = day;

	/* Generate month */
	if(year_size(d1) == 366)
		month_size[1] = 29;
	for(d1 = 0; d0 >= month_size[d1]; d1++)
		d0 -= month_size[d1];
	month_size[1] = 28;
	xtime.tm_mday = d0 + 1;
	xtime.tm_mon = d1;
	xtime.tm_isdst = 0;
	return(&xtime);
}
void
tzset()
{
  char *getzname(), *gettime(), *getdigit();
  int getdst();
  char *ptr;

  
	if ( (ptr = getenv("TZ")) == NULL)
		return;
	
	/* Get main time zone name and difference from GMT */
	if ( ((ptr = getzname(ptr,tzname[0])) == 0) || ((ptr = gettime(ptr,&timezone,1)) == 0))
		return;

	altzone = timezone - 60 * 60;
	start_dst = end_dst = 0;
	daylight = 0;

	 
	/* Get alternate time zone name */
	if ( (ptr = getzname(ptr,tzname[1])) == 0)
		return;
	daylight = 1;

	/* If the difference between alternate time zone and
	 * GMT is not given, use one hour as default.
	 */
	if (*ptr == '\0')
		return;
	if (*ptr != ';')
		if ( (ptr = gettime(ptr,&altzone,1)) == 0 || *ptr != ';')
			return;
	getdst(ptr + 1,&start_dst, &end_dst);
}

static char *
getzname(p,tz)
char *p, *tz;
{
  int n = MAXTZNAME;

  if (!isalpha(*p))
	return(0);
  do
  {
	*tz++ = *p ;
  } while (--n > 0 && isalpha(*++p) );
  while(isalpha(*p))
	p++;
  while(--n >= 0)
	*tz++ = ' ';		/* Pad with blanks */
  return(p);	
}

static char *
gettime(p,timez,f)
char *p;
time_t *timez;
int f;
{
	register time_t t = 0;
	int d, sign = 0;

	d = 0;
	if (f)
		if ( (sign = (*p == '-')) || (*p == '+'))
			p++;
	if ( (p = getdigit(p,&d)) != 0)
	{
		t = d * 60 * 60;
		if (*p == ':')
		{
			if( (p = getdigit(p+1,&d)) != 0)
			{
				t += d * 60;
				if (*p == ':')
				{
					if( (p = getdigit(p+1,&d)) != 0)
						t += d;
				}
			}
		}
	}
	if(sign)
		*timez = -t;
	else
		*timez = t;
	return(p);
}
static char *
getdigit(ptr,d)
char *ptr;
int * d;
{


	if (!isdigit(*ptr))
		return(0);
 	*d = 0;
	do
	{
		*d *= 10;
		*d += *ptr - '0';
	}while( (isdigit(*++ptr)));
	return(ptr);
}
static int
getdst(p,s,e)
char *p;
time_t *s, *e;
{
	int lsd,led;
	time_t st,et;
	st = et = 0; 		/* Default for start and end time is 00:00:00 */
	if ( (p = getdigit(p,&lsd)) == 0 )
		return(0);
	lsd -= 1; 	/* keep julian count in sync with date  1-366 */
	if ( (*p == '/') &&  ((p = gettime(p+1,&st,0)) == 0) )
			return(0);
	if (*p == ',')
	{
		if ( (p = getdigit(p+1,&led)) == 0 )
			return(0);
		led -= 1; 	/* keep julian count in sync with date  1-366 */
		if ((*p == '/') &&  ((p = gettime(p+1,&et,0)) == 0) )
				return(0);
	}
	/* Convert the time into seconds */
	*s = (long)(lsd * SEC_IN_DAY + st);
	*e = (long)(led * SEC_IN_DAY + et - (timezone - altzone));
	return(1);
}

static void
getusa(s,e,t)
int *s,*e;
struct tm *t;
{
	static struct {
		int	yrbgn;
		int	daylb;
		int	dayle;
	} daytab[] = {
		87,	96,	303,	/* new legislation - 1st Sun in April	*/
		76,	119,	303,
		75,	58,	303,	/* 1975: Last Sun in Feb - last Sun in Oct */
		74,	5,	333,	/* 1974: Jan 6 - last Sun. in Nov */
		70,	119,	303,	/* start GMT	*/
	};
	int i = 0;

	while (t->tm_year < daytab[i].yrbgn) /* can't be less than 70	*/
		i++;
	*s = daytab[i].daylb; /* fall through the loop when in correct interval	*/
	*e = daytab[i].dayle;

	*s = sunday(t, *s);
	*e = sunday(t, *e);
	*s = (long)(*s * SEC_IN_DAY +2*60*60);
	*e = (long)(*e * SEC_IN_DAY +60*60);
	return;
}
static int
sunday(t, d)
register struct tm *t;
register int d;
{
	if(d >= 58)
		d += year_size(t->tm_year) - 365;
	return(d - (d - t->tm_yday + t->tm_wday + 700) % 7);
}

/*
 * return time_t from a tm structure whose values may be out of range
 */
time_t 
mktime(timeptr)
struct tm *timeptr;
{
	register i;
	int     sum,tmp;
	time_t	tv, secs, daybegin, dayend;
	struct	tm *tm;
	int	isdst = 0;


	tv = timeptr->tm_sec + (timeptr->tm_min * SEC_IN_MIN) +
	     (timeptr->tm_hour * SEC_IN_HOUR) + 
	     ((timeptr->tm_mday - 1) * SEC_IN_DAY);

	/* deal with months being out of range */
	if (timeptr->tm_mon >= 12) {
		timeptr->tm_year += timeptr->tm_mon/12;
		timeptr->tm_mon %= 12;
	} else if (timeptr->tm_mon < 0) {
		tmp =  timeptr->tm_mon/12 - 1;
		sum =  timeptr->tm_year + tmp;
		/* check for overflow */
		if (((timeptr->tm_year < 0) && (tmp < 0) && (sum >= 0)) ||
		 ((timeptr->tm_year > 0) && (tmp > 0) && (sum < 0))) 
		   	return((time_t) -1);
		else {
			timeptr->tm_year += timeptr->tm_mon/12 - 1;
		    	timeptr->tm_mon = 12 + (timeptr->tm_mon%12);
		}
	}

	/* If the year is before 1970, indicate that
	 * we cannot compute the time.
	 */
	if (timeptr->tm_year < 70)
		return((time_t) -1);

	/* Add in the days of the month for this year */
	for (i = 0; i < timeptr->tm_mon; ++i)
		tv += month_size[i] * SEC_IN_DAY;

	/* calculate seconds since the Epoch (1/1/70) 
	 */
	for (i = 1970; i < timeptr->tm_year+1900; i++)
		tv += year_size(i) * SEC_IN_DAY;

	/* add one day if the year we end up in is a leap year 
	 * and we are in a month past February
 	 */
	if (year_size(timeptr->tm_year) == 366 && timeptr->tm_mon >= 2)
		tv += SEC_IN_DAY;

	tzset();
        if (timeptr->tm_isdst > 0) {
                tv += altzone;
        } else {
                if (timeptr->tm_isdst < 0)
                        isdst = -1;
                tv += timezone;
        }

	tm = localtime(&tv);
        if (isdst == -1) {
                if (start_dst == 0 && end_dst == 0)
                        getusa(&daybegin, &dayend, tm);
                else {
                        daybegin = start_dst;
                        dayend = end_dst;
                }
		secs = timeptr->tm_sec + (timeptr->tm_min * SEC_IN_MIN) +
		       (timeptr->tm_hour * SEC_IN_HOUR) + 
		       (timeptr->tm_yday * SEC_IN_DAY);
                if (secs >= daybegin && secs < dayend) {
                        tv -= (timezone - altzone);
                        tm = localtime((time_t *)&tv);
                }
	}
        *timeptr = *tm;
	return(tv);
}
