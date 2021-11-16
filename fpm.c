#include "mu.h"
#define  ST (p-l)+1+tab
#define S '-'
#define I '|'
#define Ss (ACS_HLINE & 0xff)
#define Ii (ACS_VLINE & 0xff)
char            *
psewdo (u, l, d)
	register char  *u, *l, *d;
{
	while (*l == S || *l == I) {
		if (*l == S) {
			if (*d == I && *(l - 1) == Ss && *(l + 1) == S)
				*l = ACS_TTEE;
			else if (*d == I && *(l - 1) != Ss)
				*l = ACS_ULCORNER;
			else if (*d == I && *(l + 1) != S)
				*l = ACS_URCORNER;
			else if (*u == Ii && *(l - 1) != Ss)
				*l = ACS_LLCORNER;
			else if (*u == Ii && *(l + 1) != S)
				*l = ACS_LRCORNER;
			else if (*u == Ii && *(l - 1) == Ss && *(l + 1) == S)
				*l = ACS_BTEE;
			else
				*l = ACS_HLINE;
		} else {
			if (*(l - 1) == Ss && *(l + 1) == S)
				*l = ACS_PLUS;
			else if (*(l - 1) == Ss)
				*l = ACS_RTEE;
			else if (*(l + 1) == S)
				*l = ACS_LTEE;
			else
				*l = ACS_VLINE;
		}
		u++, l++, d++;
	}
	return (l);
}
int com (register char *p){
	return ((*p++ == *p) ? com (p) + 1 : 1);
}
void fpm (char *u,char *l,char *d){
	register char  *p, *s;
	int             ip = 0, tab = 0, lp = 0, xold = 0, col, j;

	Str++;
	s = l;
	p = l - 1;
	while (*++p != 0) {
		col = com (p);
		switch (*p) {
		    default:
			if (lp && ip > 1) {     /* Были псевдо пробелы */
				s_tab (xold, Str, p - s - ip, s, 1);
				s = p;
				xold = (s - l) + 1 + tab;
			}
			ip = 0;
			p += --col;
			++lp;
			continue;
		    case '-':
		    case '|':
			j = p - l;
			col = psewdo (u + j, p, d + j) - p;
			s_tab (ST, Str, col, p, 0);
			break;
		    case '\t':
			tab = col * 8 - col;
			break;
		    case '_':
			s_pol (ST, Str, col, p);
		    case ' ':
			break;
		}
		ip += col;
		p += --col;
		if (!lp) {
			s = p + 1;	/* Лидирующие пробелы */
			xold = (s - l) + 1 + tab;
		}
	}
	if ((p - s) - ip > 0)
		s_tab (xold, Str, (p - s) - ip, s, 1);
}
