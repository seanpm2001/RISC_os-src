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
#ident	"$Header: doprnt.c,v 1.2.1.2 90/05/07 21:05:01 wje Exp $"

/*
 *	_doprnt: common code for printf, fprintf, sprintf
 */

#include <stdio.h>
#include <ctype.h>

#include <varargs.h>
#include <values.h>
#if defined(mips) && !defined(MOXIE)
#include <fp_class.h>
#endif
#include "print.h"

/*
 * _NFILE is no longer defined (4.3 allocates FILE structures dynamically).
 * We define it as 64 here, but that may not be correct.
 */

#ifndef _NFILE
#define _NFILE 64
#endif

#define	NDIG 82
static char buffer[NDIG+2];

static char *
cvt (argsp, ndigits, decpt, sign, f_flag, fp_classp, is_zerop)
	va_list *argsp;
	int ndigits, *sign, f_flag;
	register int *decpt;
	int *fp_classp, *is_zerop;
{
	double value;
#if defined(mips) && !defined(MOXIE)
	int fp_class;
#endif

	value = va_arg(*argsp, double);

#if defined(mips) && !defined(MOXIE)
	fp_class = fp_class_d(value);
	*fp_classp = fp_class;
	*is_zerop = (value == 0.0);
	if (fp_class == FP_QNAN || fp_class == FP_SNAN) {
		*sign = 0;
		*decpt = strlen("NaN");
		strcpy(buffer, "NaN");
		return(buffer);
	}
	if (fp_class == FP_POS_INF || fp_class == FP_NEG_INF) {
		*decpt = strlen("Infinity");
		strcpy(buffer, "Infinity");
		return(buffer);
	}
#endif

	if (!f_flag) {
		if (ndigits > 17) {
			register char *p, *e;
			*decpt = _dtoa (buffer, 17, value, 0) + 1;
			for (p = buffer+18,
			     e = buffer + 1 + (ndigits > NDIG ? NDIG : ndigits);
			     p != e; ) *p++ = '0';
			*p++ = '\0';
		}
		else if (ndigits <= 0) {
			*decpt = _dtoa (buffer, 1, value, 0) + 1;
			buffer[1] = '\0';
		}
		else {
			*decpt = _dtoa (buffer, ndigits, value, 0) + 1;
		}
		*sign = buffer[0] == '-';
		return buffer+1;
	}
	else {
		*decpt = _dtoa (buffer, ndigits, value, 1) + 1;
		*sign = buffer[0] == '-';
		return buffer+1;
	}
}


#ifdef notdef
#define PUT(p, n)     { register char *newbufptr; \
			if ((newbufptr = bufptr + n) > bufferend) { \
				_dowrite(p, n, iop, &bufptr); \
			} else { \
				(void) memcpy((char *) bufptr, p, n); \
				bufptr = newbufptr; \
			} \
		      }

#define PAD(s, n)     { register int nn; \
			for (nn = n; nn > 20; nn -= 20) \
				_dowrite(s, 20, iop, &bufptr); \
			PUT(s, nn); \
		      }
#endif
#define PUT(p, n)     { register int nn, cc; \
			for (nn = 0; nn < n; nn++) { \
				cc = *(p+nn); \
				putc(cc, iop); \
			} \
		      }

#define PAD(s, n)     { register int nn, cc; \
			cc = *s; \
			for (nn = n; nn > 0; nn--) { \
				putc (cc, iop); \
			} \
		      }


/* bit positions for flags used in doprnt */
#define LENGTH	1	/* l */
#define FPLUS	2	/* + */
#define FMINUS	4	/* - */
#define FBLANK	8	/* blank */
#define FSHARP	16	/* # */
#define PADZERO 32	/* padding zeroes requested via '0' */
#define DOTSEEN 64	/* dot appeared in format specification */
#define SUFFIX	128	/* a suffix is to appear in the output */
#define RZERO	256	/* there will be trailing zeros in output */
#define LZERO	512	/* there will be leading zeroes in output */

/*
 *	C-Library routines for floating conversion
 */

extern int fwrite();
static char *_bufendtab[_NFILE+1];


static int
_lowdigit(valptr)
long *valptr;
{	/* This function computes the decimal low-order digit of the number */
	/* pointed to by valptr, and returns this digit after dividing   */
	/* *valptr by ten.  This function is called ONLY to compute the */
	/* low-order digit of a long whose high-order bit is set. */

	int lowbit = *valptr & 1;
	long value = (*valptr >> 1) & ~HIBITL;

	*valptr = value / 5;
	return(value % 5 * 2 + lowbit + '0');
}

#ifdef notdef
/* The function _dowrite carries out buffer pointer bookkeeping surrounding */
/* a call to fwrite.  It is called only when the end of the file output */
/* buffer is approached or in other unusual situations. */
static
_dowrite(p, n, iop, ptrptr)
register char *p;
register int	n;
register FILE	*iop;
register char **ptrptr;
{
	if (iop->_file != _NFILE) {
		iop->_cnt -= (*ptrptr - iop->_ptr);
		iop->_ptr = *ptrptr;
		_bufsync(iop);
		(void) fwrite(p, 1, n, iop);
		*ptrptr = iop->_ptr;
	} else
		*ptrptr =  memcpy((char *) *ptrptr, p, n) + n;
}
#endif

int
_doprnt(format, args, iop)
register char	*format;
va_list	args;
register FILE	*iop;
{
	static char _blanks[] = "                    ";
	static char _zeroes[] = "00000000000000000000";

	/* bufptr is used inside of doprnt instead of iop->_ptr; */
	/* bufferend is a copy of _bufend(iop), if it exists.  For */
	/* dummy file descriptors (iop->_file = _NFILE), bufferend */
	/* may be meaningless.				*/

	char *bufptr, *bufferend;

	/* This variable counts output characters. */
	int	count = 0;

	/* Starting and ending points for value to be printed */
	register char	*bp;
	char *p;

	/* Field width and precision */
	int	width, prec;

	/* Format code */
	register int	fcode;

	/* Number of padding zeroes required on the left and right */
	int	lzero, rzero;

	/* Flags - bit positions defined by LENGTH, FPLUS, FMINUS, FBLANK, */
	/* and FSHARP are set if corresponding character is in format */
	/* Bit position defined by PADZERO means extra space in the field */
	/* should be padded with leading zeroes rather than with blanks */
	register int	flagword;

	/* Values are developed in this buffer */
	char	buf[max(MAXDIGS, 1+max(MAXFCVT+MAXEXP, MAXECVT))];

	/* Pointer to sign, "0x", "0X", or empty */
	char	*prefix;

	/* Exponent or empty */
	char	*suffix;

	/* Buffer to create exponent */
	char	expbuf[MAXESIZ + 1];

	/* Length of prefix and of suffix */
	int	prefixlength, suffixlength;

	/* combined length of leading zeroes, trailing zeroes, and suffix */
	int 	otherlength;

	/* The value being converted, if integer */
	long	val;

	/* Output values from fcvt and ecvt */
	int	decpt, sign, is_zero;

	/* Pointer to a translate table for digits of whatever radix */
	char	*tab;

	/* Work variables */
	int	k, lradix, mradix;

	/* class of floating point value */
	int	fp_class;

	/* initialize buffer pointer and buffer end pointer */
	{ register int fno = iop->_file;
	bufptr = iop->_ptr;
	bufferend = (fno == _NFILE) ? 
			(char *)((long) bufptr | (-1L & ~HIBITL))
				 : _bufendtab[fno];
	}

	/*
	 *	The main loop -- this loop goes through one iteration
	 *	for each string of ordinary characters or format specification.
	 */
	for ( ; ; ) {
		register int n;
	mainloop:
		while ((fcode = *format) != '\0' && fcode != '%') {
			putc(fcode, iop);
			format++;
			count++;
		}
#ifdef notdef
		if ((fcode = *format) != '\0' && fcode != '%') {
			bp = format;
			do {
				format++;
			} while ((fcode = *format) != '\0' && fcode != '%');
		
			count += (n = format - bp); /* n = no. of non-% chars */
			PUT(bp, n);
		}
#endif
		if (fcode == '\0') {  /* end of format; return */
#ifdef notdef
			register int nn = bufptr - iop->_ptr;
			iop->_cnt -= nn;
			iop->_ptr = bufptr;
			if (bufptr + iop->_cnt > bufferend && /* in case of */
					iop->_file != _NFILE) /* interrupt */
				_bufsync(iop);	/* during last several lines */
			if (iop->_flag & (_IONBF | _IOLBF) &&
				    (iop->_flag & _IONBF ||
				     memchr((char *)(bufptr-count), '\n', count) != NULL))
				(void) _xflsbuf(iop);
#endif
			return(ferror(iop) ? EOF : count);
		}

		/*
		 *	% has been found.
		 *	The following switch is used to parse the format
		 *	specification and to perform the operation specified
		 *	by the format letter.  The program repeatedly goes
		 *	back to this switch until the format letter is
		 *	encountered.
		 */
		width = prefixlength = otherlength = flagword = 0;
		format++;

	charswitch:

		switch (fcode = *format++) {

		case '+':
			flagword |= FPLUS;
			goto charswitch;
		case '-':
			flagword |= FMINUS;
			goto charswitch;
		case ' ':
			flagword |= FBLANK;
			goto charswitch;
		case '#':
			flagword |= FSHARP;
			goto charswitch;

		/* Scan the field width and precision */
		case '.':
			flagword |= DOTSEEN;
			prec = 0;
			goto charswitch;

		case '*':
			if (!(flagword & DOTSEEN)) {
				width = va_arg(args, int);
				if (width < 0) {
					width = -width;
					flagword ^= FMINUS;
				}
			} else {
				prec = va_arg(args, int);
				if (prec < 0)
					prec = 0;
			}
			goto charswitch;

		case '0':	/* obsolescent spec:  leading zero in width */
				/* means pad with leading zeros */
			if (!(flagword & (DOTSEEN | FMINUS)))
				flagword |= PADZERO;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		      { register num = fcode - '0';
			while (isdigit(fcode = *format)) {
				num = num * 10 + fcode - '0';
				format++;
			}
			if (flagword & DOTSEEN)
				prec = num;
			else
				width = num;
			goto charswitch;
		      }

		/* Scan the length modifier */
		case 'l':
			flagword |= LENGTH;
			/* No break */
		case 'h':
			goto charswitch;

		/*
		 *	The character addressed by format must be
		 *	the format letter -- there is nothing
		 *	left for it to be.
		 *
		 *	The status of the +, -, #, and blank
		 *	flags are reflected in the variable
		 *	"flagword".  "width" and "prec" contain
		 *	numbers corresponding to the digit
		 *	strings before and after the decimal
		 *	point, respectively. If there was no
		 *	decimal point, then flagword & DOTSEEN
		 *	is false and the value of prec is meaningless.
		 *
		 *	The following switch cases set things up
		 *	for printing.  What ultimately gets
		 *	printed will be padding blanks, a
		 *	prefix, left padding zeroes, a value,
		 *	right padding zeroes, a suffix, and
		 *	more padding blanks.  Padding blanks
		 *	will not appear simultaneously on both
		 *	the left and the right.  Each case in
		 *	this switch will compute the value, and
		 *	leave in several variables the informa-
		 *	tion necessary to construct what is to
		 *	be printed.  
		 *
		 *	The prefix is a sign, a blank, "0x",
		 *	"0X", or null, and is addressed by
		 *	"prefix".
		 *
		 *	The suffix is either null or an
		 *	exponent, and is addressed by "suffix".
		 *	If there is a suffix, the flagword bit
		 *	SUFFIX will be set.
		 *
		 *	The value to be printed starts at "bp"
		 *	and continues up to and not including
		 *	"p".
		 *
		 *	"lzero" and "rzero" will contain the
		 *	number of padding zeroes required on
		 *	the left and right, respectively.
		 *	The flagword bits LZERO and RZERO tell
		 *	whether padding zeros are required.
		 *
		 *	The number of padding blanks, and
		 *	whether they go on the left or the
		 *	right, will be computed on exit from
		 *	the switch.
		 */



		
		/*
		 *	decimal fixed point representations
		 *
		 *	HIBITL is 100...000
		 *	binary, and is equal to	the maximum
		 *	negative number.
		 *	We assume a 2's complement machine
		 */

		case 'D':
			flagword |= LENGTH;
			/* fall through */
		case 'd':
		case 'i':
			/* Fetch the argument to be printed */
			if (flagword & LENGTH)
				val = va_arg(args, long);
			else
				val = va_arg(args, int);

			/* Set buffer pointer to last digit */
			p = bp = buf + MAXDIGS;

			/* If signed conversion, make sign */
			if (val < 0) {
				prefix = "-";
				prefixlength = 1;
				/*
				 * Negate, checking in
				 * advance for possible
				 * overflow.
				 */
				if (val != HIBITL)
					val = -val;
				else     /* number is -HIBITL; convert last */
					 /* digit now and get positive number */
					*--bp = _lowdigit(&val);
			} else if (flagword & FPLUS) {
				prefix = "+";
				prefixlength = 1;
			} else if (flagword & FBLANK) {
				prefix = " ";
				prefixlength = 1;
			}

		decimal:
			{ register long qval = val;
				if (qval <= 9) {
					if (qval != 0 || !(flagword & DOTSEEN))
						*--bp = qval + '0';
				} else {
					do {
						n = qval;
						qval /= 10;
						*--bp = n - qval * 10 + '0';
					} while (qval > 9);
					*--bp = qval + '0';
				}
			}

			/* Calculate minimum padding zero requirement */
			if (flagword & DOTSEEN) {
				register leadzeroes = prec - (p - bp);
				if (leadzeroes > 0) {
					otherlength = lzero = leadzeroes;
					flagword |= LZERO;
				}
			}

			break;

		case 'n':
			if (flagword & LENGTH)
				*va_arg(args, long *) = count;
			else
				*va_arg(args, int *) = count;
			goto mainloop;

		case 'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			/* NOSTRICT */
			val = (long)va_arg(args, void *);
			mradix = 15;
			lradix = 3;
			goto nosign;

		case 'U':
			flagword |= LENGTH;
			/* fall through */
		case 'u':
			/* Fetch the argument to be printed */
			if (flagword & LENGTH)
				val = va_arg(args, long);
			else
				val = va_arg(args, unsigned);

			p = bp = buf + MAXDIGS;

			if (val & HIBITL)
				*--bp = _lowdigit(&val);

			goto decimal;

		/*
		 *	non-decimal fixed point representations
		 *	for radix equal to a power of two
		 *
		 *	"mradix" is one less than the radix for the conversion.
		 *	"lradix" is one less than the base 2 log
		 *	of the radix for the conversion. Conversion is unsigned.
		 *	HIBITL is 100...000
		 *	binary, and is equal to	the maximum
		 *	negative number.
		 *	We assume a 2's complement machine
		 */

		case 'O':
			flagword |= LENGTH;
			/* fall through */
		case 'o':
			mradix = 7;
			lradix = 2;
			goto fixed;

		case 'X':
		case 'x':
			mradix = 15;
			lradix = 3;

		fixed:
			/* Fetch the argument to be printed */
			if (flagword & LENGTH)
				val = va_arg(args, long);
			else
				val = va_arg(args, unsigned);

		nosign:
			/* Set translate table for digits */
			tab = (fcode == 'X') ?
			    "0123456789ABCDEF" : "0123456789abcdef";

			/* Develop the digits of the value */
			p = bp = buf + MAXDIGS;
			{ register long qval = val;
				if (qval == 0) {
					if (!(flagword & DOTSEEN)) {
						otherlength = lzero = 1;
						flagword |= LZERO;
					}
				} else
					do {
						*--bp = tab[qval & mradix];
						qval = ((qval >> 1) & ~HIBITL)
								 >> lradix;
					} while (qval != 0);
			}

			/* Calculate minimum padding zero requirement */
			if (flagword & DOTSEEN) {
				register leadzeroes = prec - (p - bp);
				if (leadzeroes > 0) {
					otherlength = lzero = leadzeroes;
					flagword |= LZERO;
				}
			}

			/* Handle the # flag */
			if (flagword & FSHARP && val != 0)
				switch (fcode) {
				case 'o':
					if (!(flagword & LZERO)) {
						otherlength = lzero = 1;
						flagword |= LZERO;
					}
					break;
				case 'x':
					prefix = "0x";
					prefixlength = 2;
					break;
				case 'X':
					prefix = "0X";
					prefixlength = 2;
					break;
				}

			break;

		case 'E':
		case 'e':
			/*
			 * E-format.  The general strategy
			 * here is fairly easy: we take
			 * what ecvt gives us and re-format it.
			 */

			/* Establish default precision */
			if (!(flagword & DOTSEEN))
				prec = 6;

			/* Develop the mantissa */
			bp = cvt(&args, min(prec + 1, MAXECVT), &decpt, &sign,
			    0, &fp_class, &is_zero);

			/* Determine the prefix */
		e_merge:
#if defined(mips) && !defined(MOXIE)
			if (fp_class == FP_QNAN || fp_class == FP_SNAN) {
				bp = "NaN";
				p = bp + strlen(bp);
				break;
			}
			if (fp_class == FP_POS_INF) {
				if(flagword & FPLUS) 
					bp = "+Infinity";
				else
					bp = "Infinity";
				p = bp + strlen(bp);
				break;
			}
			if (fp_class == FP_NEG_INF) {
				bp = "-Infinity";
				p = bp + strlen(bp);
				break;
			}
#endif
			if (sign) {
				prefix = "-";
				prefixlength = 1;
			} else if (flagword & FPLUS) {
				prefix = "+";
				prefixlength = 1;
			} else if (flagword & FBLANK) {
				prefix = " ";
				prefixlength = 1;
			}

			/* Place the first digit in the buffer*/
			p = &buf[0];
			*p++ = (*bp != '\0') ? *bp++ : '0';

			/* Put in a decimal point if needed */
			if (prec != 0 || (flagword & FSHARP))
				*p++ = '.';

			/* Create the rest of the mantissa */
			{ register rz = prec;
				for ( ; rz > 0 && *bp != '\0'; --rz)
					*p++ = *bp++;
				if (rz > 0) {
					otherlength = rzero = rz;
					flagword |= RZERO;
				}
			}

			bp = &buf[0];

			/* Create the exponent */
			*(suffix = &expbuf[MAXESIZ]) = '\0';
			if (!is_zero) {
				register int nn = decpt - 1;
				if (nn < 0)
				    nn = -nn;
				for ( ; nn > 9; nn /= 10)
					*--suffix = todigit(nn % 10);
				*--suffix = todigit(nn);
			}

			/* Prepend leading zeroes to the exponent */
			while (suffix > &expbuf[MAXESIZ - 2])
				*--suffix = '0';

			/* Put in the exponent sign */
			*--suffix = (decpt > 0 || is_zero) ? '+' : '-';

			/* Put in the e */
			*--suffix = isupper(fcode) ? 'E'  : 'e';

			/* compute size of suffix */
			otherlength += (suffixlength = &expbuf[MAXESIZ]
								 - suffix);
			flagword |= SUFFIX;

			break;

		case 'f':
			/*
			 * F-format floating point.  This is a
			 * good deal less simple than E-format.
			 * The overall strategy will be to call
			 * fcvt, reformat its result into buf,
			 * and calculate how many trailing
			 * zeroes will be required.  There will
			 * never be any leading zeroes needed.
			 */

			/* Establish default precision */
			if (!(flagword & DOTSEEN))
				prec = 6;

			/* Do the conversion */
			bp = cvt(&args, min(prec, MAXFCVT), &decpt, &sign, 1,
			    &fp_class, &is_zero);

			/* Determine the prefix */
		f_merge:
#if defined(mips) && !defined(MOXIE)
			if (fp_class == FP_QNAN || fp_class == FP_SNAN) {
				bp = "NaN";
				p = bp + strlen(bp);
				break;
			}
			if (fp_class == FP_POS_INF) {
				if(flagword & FPLUS) 
					bp = "+Infinity";
				else
					bp = "Infinity";
				p = bp + strlen(bp);
				break;
			}
			if (fp_class == FP_NEG_INF) {
				bp = "-Infinity";
				p = bp + strlen(bp);
				break;
			}
#endif
			if (sign && decpt > -prec && *bp != '0') {
				prefix = "-";
				prefixlength = 1;
			} else if (flagword & FPLUS) {
				prefix = "+";
				prefixlength = 1;
			} else if (flagword & FBLANK) {
				prefix = " ";
				prefixlength = 1;
			}

			/* Initialize buffer pointer */
			p = &buf[0];

			{ register int nn = decpt;

				/* Emit the digits before the decimal point */
				k = 0;
				do {
					*p++ = (nn <= 0 || *bp == '\0' 
						|| k >= MAXFSIG) ?
				    		'0' : (k++, *bp++);
				} while (--nn > 0);

				/* Decide whether we need a decimal point */
				if ((flagword & FSHARP) || prec > 0)
					*p++ = '.';

				/* Digits (if any) after the decimal point */
				nn = min(prec, MAXFCVT);
				if (prec > nn) {
					flagword |= RZERO;
					otherlength = rzero = prec - nn;
				}
				while (--nn >= 0)
					*p++ = (++decpt <= 0 || *bp == '\0' ||
				   	    k >= MAXFSIG) ? '0' : (k++, *bp++);
			}

			bp = &buf[0];

			break;

		case 'G':
		case 'g':
			/*
			 * g-format.  We play around a bit
			 * and then jump into e or f, as needed.
			 */
		
			/* Establish default precision */
			if (!(flagword & DOTSEEN))
				prec = 6;
			else if (prec == 0)
				prec = 1;

			/* Do the conversion */
			bp = cvt(&args, min(prec, MAXECVT), &decpt, &sign, 0,
			    &fp_class, &is_zero);
			if (is_zero)
				decpt = 1;

			{ register int kk = prec;
				if (!(flagword & FSHARP)) {
					n = strlen(bp);
					if (n < kk)
						kk = n;
					while (kk >= 1 && bp[kk-1] == '0')
						--kk;
				}
				
				if (decpt < -3 || decpt > prec) {
					prec = kk - 1;
					goto e_merge;
				}
				prec = kk - decpt;
				goto f_merge;
			}

		case '%':
			buf[0] = fcode;
			goto c_merge;

		case 'c':
			buf[0] = va_arg(args, int);
		c_merge:
			p = (bp = &buf[0]) + 1;
			break;

		case 's':
			bp = va_arg(args, char *);
			if (bp == NULL) {	/* DAG -- robustness addition */
				bp = "(null)";
				flagword &= ~DOTSEEN;
			}
			if (!(flagword & DOTSEEN))
				p = bp + strlen(bp);
			else { /* a strnlen function would  be useful here! */
				register char *qp = bp;
				while (*qp++ != '\0' && --prec >= 0)
					;
				p = qp - 1;
			}
			break;

		default: /* this is technically an error; what we do is to */
			/* back up the format pointer to the offending char */
			/* and continue with the format scan */
			format--;
			continue;

		}

		/* Calculate number of padding blanks */
		k = (n = p - bp) + prefixlength + otherlength;
		if (width <= k)
			count += k;
		else {
			count += width;

			/* Set up for padding zeroes if requested */
			/* Otherwise emit padding blanks unless output is */
			/* to be left-justified.  */

			if (flagword & PADZERO) {
				if (!(flagword & LZERO)) {
					flagword |= LZERO;
					lzero = width - k;
				}
				else
					lzero += width - k;
				k = width; /* cancel padding blanks */
			} else
				/* Blanks on left if required */
				if (!(flagword & FMINUS))
					PAD(_blanks, width - k);
		}

		/* Prefix, if any */
		if (prefixlength != 0)
			PUT(prefix, prefixlength);

		/* Zeroes on the left */
		if (flagword & LZERO)
			PAD(_zeroes, lzero);
		
		/* The value itself */
		if (n > 0)
			PUT(bp, n);

		if (flagword & (RZERO | SUFFIX | FMINUS)) {
			/* Zeroes on the right */
			if (flagword & RZERO)
				PAD(_zeroes, rzero);

			/* The suffix */
			if (flagword & SUFFIX)
				PUT(suffix, suffixlength);

			/* Blanks on the right if required */
			if (flagword & FMINUS && width > k)
				PAD(_blanks, width - k);
		}
	}
}
