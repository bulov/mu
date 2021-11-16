#include "ncurses.h"
struct s_dpd { int xdim,ydim,xold,yold; } *dpd();
#define ctrl(x) ('x'&037)

#define _K(n)  (KEY_F0+(n))
#define _K0    KEY_F0
#define _K1    KEY_F0+1
#define _K2    KEY_F0+2
#define _F6    KEY_F0+6
#define _F7    KEY_F0+7
#define _F8    KEY_F0+8
#define _F9    KEY_F0+9
#define _F10   KEY_F0+10
#define _F11   KEY_F0+11
#define _F12   KEY_F0+12
#define _F13   KEY_F0+13
#define _F14   KEY_F0+14
#define _F15   KEY_F0+15
#define _F16   KEY_F0+16
#define _F17   KEY_F0+17
#define _F18   KEY_F0+18
#define _F19   KEY_F0+19
#define _F20   KEY_F0+20
#define _K01   KEY_F0+21

/*#define _KD    KEY_DOWN        /* 0402 Sent by terminal down arrow key */
/*#define _KU    KEY_UP          /* 0403 Sent by terminal up arrow key */
/*#define _KL    KEY_LEFT        /* 0404 Sent by terminal left arrow key */
/*#define _KR    KEY_RIGHT       /* 0405 Sent by terminal right arrow key */
/*#define _KB    KEY_BACKSPACE   /* 0407 Sent by backspace key */

/* в#define _GE    _GS      /* включить псевдографику */
/* в#define _GS    _GS      /* включить псевдографику */

/*  #define _SE    _SE      /*  выкл. режима выделения строки        */
/*  #define _SO    _SO      /*  вкл. режим выделения строки          */

/*  #define _SF    _SF      /*  прокрутка текста вперед              */


#define BLACK 0 /**/
#define BLUE  1 /**/
#define RED   4 /**/

#define _nl    014       /* 014          /**/
#define _cr    012       /**/
#define _nul   ERR            /**/

/*
**      Флаги настройки редактора полей red   ( bvg 18.08.90 )
*/
#define R_TAIL  0x001   /* Tереть хвост при вставке */
#define R_NEXT  0x002   /* Выход по концу поля */
#define R_RET   0x004   /* Выход из редактора после R_ACT */
#define R_REW   0x008   /* Не перерисовывать строку после R_ACT */
#define R_WRK   0x010   /* Продолжить обработку символа после R_ACT */
#define R_REG   0x020   /* Вызов R_ACT по не совпадению */
#define R_IorS  0x040   /* Вставка или замена(+) */
#define R_STR   0x080   /* Редактор строк или полей(+) */
#define R_CLEAR 0x100   /* Очистить строку по выходу */
#define R_SO    0x200   /* Работать в инверсном режиме */
#define R_US    0x400   /* Строка текста подчеркнута */

#define Attroff(x) attroff(x);dpo('\b')

#define _ht    011       /* таб-ция */
#define _stx   0442       /* обр-ная таб-ция */
#define _del   010
#define _K6    0177
#define _DC    KEY_DC
#define _K5    _K(5)
#define _IC    KEY_IC
#define _bel   ctrl(G)
#define _K8    _K(8)
#define _K7    _K(7)
#define _CD  KEY_EOS     /* 0516 очистка экрана от курсора до конца   */
#define _CE  KEY_EOL     /* 0517 очистка от курсора до конца строки   */
#define _CL  KEY_CLEAR   /* 0515 очистка экрана                       */


#define UTF8_D0(a) ( ( (0xd1 == ((int)(a)&0377) ) || ((int)(a)&0377) == 0xd0 ))
