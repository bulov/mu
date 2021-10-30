#include "tty.h"
#include <ctype.h>
#include <stdio.h>
#define AMONG "  "
#define MAXATMENU 15
int             dplin_key;		/* нажатая кнопка */

dpline( x,y, list ,list2 ){
       help( x,y, list ,list2, -1 );
}

help (x, y, list, list2, key)		/* if ( key ) - только нарисовать */
	char          **list;		/* строка, по которой происходит выбор */
	char          **list2;		/* строка - подсказка. Выдается в строке y-1 */
{
	register int    chosen, atlist;
	register char **L;
	char          **tryfind;
	int             Position[MAXATMENU];
	register int    xchosen;
	int             c, xt, yt, xtold;

RESTART:
	xtold = 0;
	dpp (x, y);
	L = list;
	atlist = 0;
	xchosen = x;
	while (*L) {
		Position[atlist] = xchosen;
		if (!key)		/* bvg */
			attron (A_REVERSE);      /* bvg */
		dpo (islower (**L) ? toupper (**L) : **L);
		dps (*L + 1);		/* tail */
		if (!key)		/* bvg */
			attroff (A_REVERSE);      /* bvg */
		xchosen += strlen (*L);
		if (*(L + 1)) {
			dps (AMONG);
			xchosen += strlen (AMONG);
		}
		atlist++;
		L++;
	}
	clrtoeol();
	chosen = 0;			/* CHOISE */
	L = list;
	if (!key)			/* bvg */
		return;			/* bvg */
	while (1) {
		if (list2 != NULL) {	/* Выдать подсказку */
			xt = (Position[chosen]) +
				((int)(strlen (*L)) - (yt = strlen (*(list2 + chosen)))) / 2;
			if (xt < 0)
				xt = 0;
			if (xt + yt >= dpd ()->xdim  )
				xt = dpd ()->xdim - yt - 1;
			if (y == 0)
				yt = 1;
			else
				yt = y - 1;
			ceol(xtold, yt);/* Стереть старую подсказку */
			dpp (xtold = xt, yt);/**/
			attron (A_REVERSE);
			dps (*(list2 + chosen));
			attroff (A_REVERSE);      /* bvg */
		}
		dpp (Position[chosen], y);	/* Выделить выбранный item */
		attron (A_REVERSE);
		dpo (islower (**L) ? toupper (**L) : **L);
		dps (*L + 1);		/* tail */
		attroff (A_REVERSE);      /* bvg */
		xchosen = chosen;
		switch (c = dpi ()) {
		    case KEY_LEFT:
			if (chosen == 0)
				chosen = atlist - 1;
			else
				chosen--;
			break;
		    case KEY_RIGHT:
			if (chosen == atlist - 1)
				chosen = 0;
			else
				chosen++;
			break;
		    case _nul:
			dplin_key = (int) _nul;
			if(key >= 0 )
			       ceol (xt, yt);  /* bvg */
			return -1;	/* отказ */
		    case ctrl (A):
			dpbeg ();
			goto RESTART;	/* перерисовка */
		    default:
			tryfind = list;
			while (*tryfind) {
				if (c == **tryfind || c == toupper (**tryfind)) {
					dplin_key = c;
					if(key >= 0 )
					       ceol (xt, yt);  /* bvg */
					return (tryfind - list);
				}
				tryfind++;
			}
			dplin_key = c;
			if(key >= 0 )
			       ceol (xt, yt);  /* bvg */
			return chosen;
		}
		dpp (Position[xchosen], y);
		dpo (islower (**L) ? toupper (**L) : **L);
		dps (*L + 1);		/* tail */
		L = list + chosen;
	}
}
