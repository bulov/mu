/*
**      Редактор полей / строк.
*/
#include "mu.h"
#include "tty.h"
#include <ctype.h>
static int isin ();
static int      nulltrap[] = {0};
static int traps(int x,int y, char s[], int mx, int *pos, int c){}
/*
** Могут быть изменены извне  ( настроены )
*/
int     Red;                    /* Флаг управления */
int    *R_trap = nulltrap;      /* Должен кончаться нулем */
int     (*r_act) () = traps;    /* Заглушка */
/* #define R_TAIL  0x001   /* Tереть хвост при вставке */
/* #define R_NEXT  0x002   /* Выход по концу поля */
/* #define R_RET   0x004   /* Выход из редактора после R_ACT */
/* #define R_REW   0x008   /* Не перерисовывать строку после R_ACT */
/* #define R_WRK   0x010   /* Продолжить обработку символа после R_ACT */
/* #define R_REG   0x020   /* Вызов R_ACT по не совпадению */
/* #define R_IorS  0x040   /* Вставка или замена(+) */
/* #define R_STR   0x080   /* Редактор строк или полей(+) */
/* #define R_CLEAR 0x100   /* Очистить строку по выходу */
/* #define R_SO    0x200   /* Работать в инверсном режиме */
/* #define R_US    0x400   /* Строка текста подчеркнута */

int fUTF8( char *a ){  // Проверка на Russian UTF8 символ
   return  ( (0xd0 == (*(a)&0xd0))  && (0x80 == (*(a+1)&0xC0)) ) ? 1:0;
}
int tUTF8(char *cline ,int thiscol){    // Сколько Russian UTF8 символов в строке до позиции thiscol  на экране
   int n,i,j;
   for (n=i=j=0; i < thiscol && 0 != cline[i] ; i++)
       if ( fUTF8(&cline[i]) ) i++ ,n++ , j++;
   return (n);
}
int nUTF8(char *cline ,int thiscol){    // Сколько Russian UTF8 символов в строке до позиции thiscol+UTF8  на экране
   int n,i,j;
   for (n=i=j=0; i < thiscol+n && 0 != cline[i] ; i++)
       if ( fUTF8(&cline[i]) ) i++ ,n++ , j++;
   return (n);
}
int wUTF8(char *cline ,int thiscol,int ulhccno){     // Сколько Russian UTF8 символов в строке до позиции thiscol  в окне  экрана
   int n=0,i,j;
   ulhccno += tUTF8(cline,ulhccno);
   j=ulhccno+thiscol;
   for (i=ulhccno ; i < j; i++)
       if ( fUTF8(&cline[i]) ) i++, n++ , j++;
   return (n);
}

//        int             x, y;           /* место */
//        char           *s;              /* строка- объект */
//        int             mx;             /* max len without \0 */
//        int             pos;            /* начальная позиция курсора */
int red (int x,int y,char *s,int mx,int pos){
	register int    c;
	register char  *ss, *dd, *se;
	int             rewr=0, max;
	int             iUTF8, RcyflagUTF8=0;
	static char    *acts[] = {
				  "cr","Backspace","Del","Ins","K8","^A","K7","KR","KL","ht","stx","nl","KB",0
	};
	static char    *helps[] = {
				"Выход из редактора",
				"Удаление символа слева",
				"Удаление символа справа",
				"Вставка замена",
				"Очистить строку",
				"Очистить экран",
				"Перерисовать строку",
				"Курсор вправо",
				"Курсор влево",
				"Tабуляция",
				"Oбратная табуляциия",
				"В начало строки",
				"Выдать справку ЭТУ",
				0
	};

	iUTF8 = tUTF8(s,mx);
	max = mx-1;
	dpp (x,y);
	dd = s;                         /* Начало строки */
	se = dd + max + iUTF8;                  /* Конец строки */
	if( Red & R_SO){
		if( Red & R_US)
			attroff (A_UNDERLINE); /* Выключить подчеркивание */
		attron (A_REVERSE);            /* Включить выделение */
	}
	while (*dd && dd <= se)    dpo (*dd++);    /* head string */
	if(pos > dd -s){
		pos = dd -s;
	}
	if(pos > max){
		pos = max;
	}
	if ( !(Red & R_STR)  || !dpo(_CE) )
		rewr = 1;
	while (dd <= se) {              /* tail string */
		*dd++ = '\0';
		if(rewr)
			dpo (' ');
	}
	dpp (x + pos -tUTF8(s,pos), y);
	for(;;){
		rewr = -1;
		c = dpi ();
		if ( UTF8_D0(c) ){
		   RcyflagUTF8 = c;
		   c = dpi ();
		}
		if (*R_trap && isin (c, R_trap)) {       /* Cмотрим cимволы прерывания */
			(*r_act) (x, y, s, mx, &pos, c);     /* Oбработчик прерывания */
			pos = pos > max ? max : (pos < 0 ? 0 : pos);
			if (Red & R_RET) {      /* Выйти из функции */
				Red &= ~R_RET;
				goto RET;
			}
			if (Red & R_REW) {      /* Не перерисовывать строку */
				Red &= ~R_REW;
				continue;
			}
			if (Red & R_WRK) {      /* Продолжить обработку символа */
				Red &= ~R_WRK;
				goto CON;
			}
			rewr = 0;               /* Перерисовать строку */
			goto REW;
		}
CON:
		switch (c) {
		    case KEY_RIGHT:           /* Курсор вправо */
			if (pos < max + tUTF8(s,pos) && s[pos]) {
				if ( fUTF8(&s[pos-1]) ){
				   pos--;
				   dpo (s[pos++]);
				}else if ( fUTF8(&s[pos]) ){
				   dpo (s[pos++]);
				}
				dpo (s[pos++]);
				continue;
			} else
				pos = 0;
			break;
		    case KEY_LEFT:           /* Курсор влево */
			if ( fUTF8(&s[pos-1]) ){
				pos--;
			}
			if ( 0 < pos ) {
				dpo ('\b');
				pos--;
				continue;
			} else {
				pos=0;
				while (s[pos]) ++pos;
				iUTF8 = nUTF8(s,max);
				if( pos > max + iUTF8 ){
					pos = max + iUTF8;
				}
			}
			break;
		    case _ht:		/* таб-ция */
			pos += 8;
			iUTF8 = nUTF8(s,max);
			if (pos > max + iUTF8){
				pos = max + iUTF8;
			}
			while (!s[pos] && pos >= 0){
				 --pos;
			}
			if(pos < max + iUTF8){
				pos++;
			}
			break;
		    case _stx:		/* обр-ная таб-ция */
			if ((pos -= 8) >= 0)
				break;
		    case _nl:           /* в начало строки */
			pos = 0;
			break;
		    case KEY_BACKSPACE:
		    case _del:
			if (!pos)
				continue;
			dpo ('\b');
			pos--;		/* left and dc */
		    case KEY_DC:        /* delete char */
		    case _K6:
		    case KEY_F(6):
			if (!s[pos])
				continue;	/* нечего */
			if (!(Red & R_STR) || !dpo (_DC))
				rewr = pos;
			for (ss = dd = &s[pos]; *ss++; *dd++ = *ss);
			break;
//                    case ctrl (H):      /* Helpик */
		    case KEY_F(1):          /* minihelp */
			dpline (0, dpd ()->ydim - 1, acts, helps);
			ceol (0, dpd ()->ydim - 2);
			ceol (0, dpd ()->ydim - 1);
			rewr = 0;
			break;
		    case KEY_IC:
		    case _K5:           /* Вставка замена */
			Red = Red & R_IorS ? Red & ~R_IorS : Red | R_IorS;
			ceol(0,dpd()->ydim-1);
			attron (A_REVERSE);            /* Включить выделение */
			if( Red & R_IorS ){
				dps(" Режим вставки ");
			}else{
				dps(" Режим замены ");
			}
			Attroff (A_REVERSE);            /* Включить выделение */
			break;
		    default:            /* Рисуем символ */
			if (c >= 0400 || iscntrl (c))
				goto RET;	/* ненормальные символы */
			if (Red & R_IorS) {        /* insertion */
				if (Red & R_TAIL)	/* обрезаем хвост */
					*se = '\0';
				if (!*se) {
					if (!(Red & R_STR) || !dpo (_IC))
						rewr = pos;
					for (dd = ss = &s[pos]; *++ss;);
					for (; ss != dd; *ss-- = *(ss - 1));
				}else{
					dpo(_bel);
					continue;
				}
			}		/* substitution */
			if ( RcyflagUTF8 ){
			   dpo (s[pos++] = RcyflagUTF8);
			   RcyflagUTF8 = 0;
			}
			dpo (s[pos++] = c);
			if (pos - tUTF8(s,pos) > max)
				if (Red & R_NEXT) {
					c = (int) KEY_RIGHT ;
					goto RET;
				} else {
					pos = max;
					dpo(_bel);
				}
			break;
RET:                case _cr:           /* Выход из редактора */
			if( Red & R_SO){
				Attroff(A_REVERSE);/* Выключить выделение */
				if( Red & R_US)
				       attron (A_UNDERLINE);/* Включить подчеркивание */
			}
			dpp (x, y);
			if (Red & R_CLEAR){
				Red & R_STR?dpo (_CE):dpn (max+1, ' ');
			}else if (Red & ( R_SO | R_US ) ){
				for( dd = s ; dd <= se; dd++ )
					*dd?dpo(*dd):dpo(' ');
			}
			if( Red & R_US)
				attron(A_STANDOUT);     /* Выключить все */
			return (c);
		    case _K8:           /* Очистить строку */
			dpp(x,y);
			for(dd = s;*dd && dd <= se;*dd++ = '\0')
				dpo(' ');
			pos=0;
			break;
		    case ctrl (A):      /* Очистить экран */
			dpo (_CL);
		    case _K7:           /* Перерисовать строку */
			rewr = 0;
		}
REW:            if (rewr >= 0) {        /* перерисуем хвост строки */
			dpp (x + rewr, y);
			dps (s + rewr);
			if(!*se)
				dpo (' ');
		}
		dpp (x + pos - tUTF8(s,pos), y);
	}
}
static int isin (i, arr)
	register   int     i, *arr;
{
	while (*arr)
		if (*arr++ == i)
			return (!(Red & R_REG));
	return (Red & R_REG);
}
#ifdef DEBUG
#define MX 20
#define R(i)  Red = Red & i ? Red & ~i : Red | i;
static int trap[] = {'t','n','r','w','g','s','c','o','u'};
cretin(x,y,s,m,pos,c)
{
	switch(c){
	   case 't':    R(R_TAIL);  break;
	   case 'n':    R(R_NEXT);  break;
	   case 'r':    R(R_RET);   break;
	   case 'w':    R(R_REW);   break;
	   case 'g':    R(R_REG);   break;
	   case 's':    R(R_STR);   break;
	   case 'c':    R(R_CLEAR); break;
	   case 'o':    R(R_SO);    break;
	   case 'u':    R(R_US);    break;
	}
	R_trap = nulltrap;
	red(0,0,s,m,1);
	R_trap = trap;
}
main()
{
	char s[MX+1];

	dpbeg();
	R_trap = trap;
	r_act = cretin;
	s[0] = 0;
	Red = R_US | R_SO;
	for(;;){
		red( 5,5,s,MX,90);
		red( 5,7,s,MX,0);
		red( 5,8,s,MX,90);
	}
}
#endif
