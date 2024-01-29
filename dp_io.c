#include "tty.h"
#include "mu.h"
int    DP_ON;

void tab_date(){   // Заглушка
}


void dpbeg() {
       register int i;
       static int  nfirst = 0;

       if( !( nfirst++ )){
//pipe              system (" stty sane" );
	      Win = initscr();        /* curses bvg */
       }
       noecho();
       cbreak();
       intrflush(Win, FALSE);
       keypad(Win,TRUE);              /* init f-keyboard */
       DP_ON =1;
}
void dpend() {          //          ПЕРЕКЛЮЧЕНИЕ В ОБЫЧНЫЙ РЕЖИМ
       extern char *rmcup;
       if( !DP_ON)
	       return;
     /*  keypad(Win,FALSE);      /* init f-keyboard */
       refresh();
//     endwin();
       doupdate();
       reset_shell_mode();/**/
       if(rmcup != NULL)
	   puts(rmcup);   // rmcup=\E[?1049l // Переключить на терминал для scroll
       DP_ON = 0;
}
/* ВЫВОД ОДНОГО СИМВОЛА С БУФЕРИЗАЦИЕЙ */
int dpo( int cc )
{
       chtype c;

       if( cc == _CL ){
	       clear();
	       return(0);
       }else if( cc == _CE ){
	       clrtoeol();
	       return(0);
       }
       c    = cc & 0x000000ffL;
       return(addch(c));
}
/* ВВОД СИМВОЛА */
int dpi() {

       int c;
       touchwin(Win);
       c=getch();
       if ( c == EOF ){
	   longjmp (Ext, _K0);
       }
       return ( c );
}
/* УСТАНОВ КУРСОРА В ПОЗИЦИЮ (x,y) */
int dpp(int x, int y )
{
       return( move (y,x));
}
/* ВЫВОД СИМВОЛА 'c' 'n' РАЗ */
void dpn(int n, int c )
{
    for(; --n>=0;)  dpo( c );
}
/* СТИРКА ХВОСТА СТРОКИ */
void ceol(int x, int y){
	dpp( x,y);
	clrtoeol();
}
/* СООБЩАЕТ ПАРАМЕТРЫ ТЕРМИНАЛА */
struct s_dpd *dpd() {
	static struct s_dpd sd;

	sd.xdim = Win->_maxx;  /* столбцов в экране */
	sd.ydim = Win->_maxy;  /* строк в экране    */
	sd.xold = Win->_curx;  /* x  из последнего dpp( x,y) */
	sd.yold = Win->_cury;  /* y  из последнего dpp( x,y) */
	return( &sd);
}
void region(int tl,int bl){        //  Открыть окно
       setscrreg(tl,bl);
       dpp(0,tl);
}
void dpc (int c) {      //  /* УСТАНОВИТЬ ЦВЕТ */
       /* Пока не реализовано */
}
void dpmsg (int x,int  y,char   *s){
    ceol( x, y);
    dpc (RED);
    dps (s);
    dpc (BLACK);
}
/* вывод псевдографики */
/* usage :
	dpo( _GS); dpcg('-'); dpo(_GE);
	или
	dpsg("-->");
*/

/* преобразование букв в псевдографику */
/*dpcg( c)
/*{
/*        attron(A_ALTCHARSET );
/*        addch( c & 0x000000ffL );
/*        attroff(A_ALTCHARSET );
/*}
*/
void dps(char *str ){       //     ВЫВОД СТРОКИ СИМВОЛОВ ДО \0
	addstr(str);
}
