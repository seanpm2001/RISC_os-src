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
#ident	"$Header: wwinschar.c,v 1.1.2.2 90/05/07 20:01:10 wje Exp $"

#include "ww.h"
#include "tt.h"

wwinschar(w, row, col, c)
register struct ww *w;
short c;
{
	register i;
	int nvis;

	/*
	 * First, shift the line.
	 */
	{
		register union ww_char *p, *q;

		p = &w->ww_buf[row][w->ww_b.r];
		q = p - 1;
		for (i = w->ww_b.r - col; --i > 0;)
			*--p = *--q;
		q->c_w = c;
	}

	/*
	 * If can't see it, just return.
	 */
	if (row < w->ww_i.t || row >= w->ww_i.b
	    || w->ww_i.r <= 0 || w->ww_i.r <= col)
		return;

	if (col < w->ww_i.l)
		col = w->ww_i.l;

	/*
	 * Now find out how much is actually changed, and fix wwns.
	 */
	{
		register union ww_char *buf;
		register char *win;
		register union ww_char *ns;
		register char *smap;
		char touched;

		nvis = 0;
		smap = &wwsmap[row][col];
		for (i = col; i < w->ww_i.r && *smap++ != w->ww_index; i++)
			;
		if (i >= w->ww_i.r)
			return;
		col = i;
		buf = w->ww_buf[row];
		win = w->ww_win[row];
		ns = wwns[row];
		smap = &wwsmap[row][i];
		touched = wwtouched[row];
		for (; i < w->ww_i.r; i++) {
			if (*smap++ != w->ww_index)
				continue;
			touched |= WWU_TOUCHED;
			if (win[i])
				ns[i].c_w =
					buf[i].c_w ^ win[i] << WWC_MSHIFT;
			else {
				nvis++;
				ns[i] = buf[i];
			}
		}
		wwtouched[row] = touched;
	}

	/*
	 * Can/Should we use delete character?
	 */
	if (tt.tt_hasinsert && nvis > (wwncol - col) / 2) {
		register union ww_char *p, *q;

		tt.tt_ninsert = 1;
		tt.tt_nmodes = c >> WWC_MSHIFT & tt.tt_availmodes;
		(*tt.tt_move)(row, col);
		(*tt.tt_putc)(c & WWC_CMASK);
		tt.tt_ninsert = 0;

		p = &wwos[row][wwncol];
		q = p - 1;
		for (i = wwncol - col; --i > 0;)
			*--p = *--q;
		q->c_w = c;
	}
}
