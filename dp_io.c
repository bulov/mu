#include "tty.h"
#include "mu.h"
int    DP_ON;

tab_date(){
}


dpbeg() {
       register int i;
       static int  nfirst = 0;

       if( !( nfirst++ )){
	      system (" stty sane" );
	      Win = initscr();        /* curses bvg */
       }
       noecho();
       cbreak();
       intrflush(Win, FALSE);
       keypad(Win,TRUE);              /* init f-keyboard */
       DP_ON =1;
}
/* ПЕРЕКЛЮЧЕНИЕ В ОБЫЧНЫЙ РЕЖИМ */
dpend() {
       if( !DP_ON)
	       return;
     /*  keypad(Win,FALSE);      /* init f-keyboard */
       refresh();
       endwin();
       doupdate();
       reset_shell_mode();/**/
       DP_ON = 0;
}
/* ВЫВОД ОДНОГО СИМВОЛА С БУФЕРИЗАЦИЕЙ */
dpo( cc )
{
       chtype c;

       if( cc == _CL ){
	       clear();
	       return;
       }else if( cc == _CE ){
	       clrtoeol();
	       return;
       }
       c    = cc & 0x000000ffL;
       return(addch(c));
}
/* ВВОД СИМВОЛА */
int dpi() {
       touchwin(Win);
       return ( getch() );
}
/* УСТАНОВ КУРСОРА В ПОЗИЦИЮ (x,y) */
dpp( x, y )
{
       return( move (y,x));
}
/* ВЫВОД СИМВОЛА 'c' 'n' РАЗ */
dpn(    n, c )
    int n, c;
{
    for(; --n>=0;)  dpo( c );
}
/* СТИРКА ХВОСТА СТРОКИ */
ceol( x,y){
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
/* Открыть окно */
region(tl,bl)
int tl,bl;
{
       setscrreg(tl,bl);
       dpp(0,tl);
}
/* УСТАНОВИТЬ ЦВЕТ */
dpc (c) {
       /* Пока не реализовано */
}
dpmsg (x, y, s) char   *s;
{
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
/* ВЫВОД СТРОКИ СИМВОЛОВ ДО \0 */
dps(      str )
    char *str;
{
	addstr(str);
}
