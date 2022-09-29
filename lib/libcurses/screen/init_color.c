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
#ident	"$Header: init_color.c,v 1.2.1.2 90/05/10 02:13:19 wje Exp $"
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#include "curses_inc.h"

int init_color(color, r, g, b)
register short color, r, g, b;
{
    register _Color *ctp = SP->_color_tbl;   /* color table pointer */
    extern   int    _outch();
    extern   void   _init_HP_pair();

    /* check if terminal can change color and validity of the	    */
    /* first argument						    */

    if (!can_change || color >= COLORS || color < 0)
        return (ERR); 

    /* if any of the last 3 arguments is out of 0 - 1000 range,     */
    /* adjust them accordingly					    */

    if (r > 1000)	r = 1000;
    if (g > 1000)	g = 1000;
    if (b > 1000)	b = 1000;
    if (r < 0)		r = 0;
    if (g < 0)		g = 0;
    if (b < 0)		b = 0;

    /* if the call came from scr_reset, the color_table already      */
    /* contains desired values, but we should still send escape seq. */

    /* if new color is exactly the same as the old one, return */

    if (ctp[color].r == r && ctp[color].g == g && ctp[color].b == b)
        return (OK);

    /* update color table                  */

    ctp[color].r = r;   ctp[color].g = g;    ctp[color].b = b;

    /* all the occurrences of color on the screen must be changed  */
    /* to the new definition                                       */

    /* for terminals that can define individual colors (Tek model)  */
    /* send an escape sequence to define that color                 */

    if (initialize_color)
    {
	if (hue_lightness_saturation)
	{
            int      h, s, l;
	    _rgb_to_hls ((float)r, (float)g, (float)b, &h, &l, &s);
            tputs (tparm (initialize_color, color, h, l, s), 1, _outch);
	}
	else
            tputs (tparm (initialize_color, color, r, g, b), 1, _outch);
    }

    /* for terminals that can only define color pairs, go through   */
    /* pairs table, and re-initialize all pairs that use given color*/

    else
    {
        register int i;
        register _Color_pair *ptp = SP->_pairs_tbl;      /* pairs table pointer */
        for (i=0; i < COLOR_PAIRS; i++)
        {
             if (ptp[i].foreground == color || ptp[i].background == color)
                 _init_HP_pair (i, ptp[i].foreground, ptp[i].background);
        }
    }
    return (OK);
}




_rgb_to_hls(r, g, b, hh, ll, ss)
register float r, g, b;
int  *hh, *ll, *ss;
{
    register float rc, gc, bc, h, l, s;
    double   max, min;
    float MAX(), MIN();

    r /= 1000;  g /= 1000;  b /= 1000;

    max = MAX (r, g, b);
    min = MIN (r, g, b);

    /* calculate lightness  */

    l = (max + min) / 2;

    /* calculate saturation */

    if (max == min)
    {
        s = 0;
        h = 0;
    }
    else
    {
        if (l < 0.5)
            s = (max - min) / (max + min);
        else
            s = (max - min) / (2 - max - min);

        /* calculate hue   */

        rc = (max - r) / (max - min);
        gc = (max - g) / (max - min);
        bc = (max - b) / (max - min);

        if (r == max)
                h = bc - gc;
        else if (g == max)
                h = 2 + rc - bc;
        else /* if (b == max) */
                h = 4 + gc - rc;

        h = h * 60;
        if (h < 0.0)
            h = h + 360;

        /* until here we have converted into HSL.  Now, to convert into */
        /* Tektronix HLS, add 120 to h					*/

	h = ((int)(h+120))%360;
    }
    *hh = (int) h;
    *ss = (int) (s * 100);
    *ll = (int) (l * 100);
}


float MAX (a, b, c)
register float a, b, c;
{
	if ( a>= b)
	     if (a >= c)
		 return (a);
	     else return (c);
        else if (c >=b)
		return (c);
	     else return (b);
}

float MIN (a, b, c)
register float a, b, c;
{
	if ( a> b)
	     if (b > c)
		 return (c);
	     else return (b);
        else if (a <c)
		return (a);
	     else return (c);
}
