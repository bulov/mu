
char    head[] = "\n\
********************************************************\n\
*ВАЗ ПАП БРАС   24.10.91  Булов B.Г.(@bvg) version 2.0 *\n\
********************************************************\n\
*  Программа работы с меню в UNIX ( mu )               *\n\
*  Вызов:  mu [-m меню] [-d каталог] [-p file] [-h]    *\n\
*  где:    -m меню    = файл с меню       ( .root.u )  *\n\
*          -d каталог = перейти в каталог ( . )        *\n\
*          -p file    = протол в файл     ( prot.u  )  *\n\
*          -h         = справка эта                    *\n\
********************************************************\n";
/*
-+ Дата 07.03.90   ВАЗ ПАП БРАС       в редакции Булова В.Г. (bvg)
-+ ВАЗ ПАП БРАС   24.10.91  Булов B.Г.(@bvg) version 2.0
-+
-+
-+ Формат строк меню:   ( старое )
-+  #ИНСТРУКЦИЯ                           - помещается в область help-а.
-+  @xx yy                                - координаты привязки меню
-+  !ДЕЙСТВИЕ                             - выполняется при входе в это меню.
-+  ТЕКСТ[       ]:$Environ               - издать setenv и сохранить .save.u
-+  ТЕКСТ:&                               - перенести в строку редактирования.
-+  ТЕКСТ:ДЕЙСТВИЕ                        - выполнить указанное действие.
-+  ТЕКСТ:<ИМЯ_ФАЙЛА_С_МЕНЮ               - перейти (прочитать) в новое меню.
-+  ТЕКСТ:<ИМЯ_ФАЙЛА_С_МЕНЮ:ДЕЙСТВИЕ      - перейти и попутно выполнить действ
-+  ТЕКСТ:<МЕНЮ_УСПЕХА:команда:<МЕНЮ_НЕУД. - По успеху в МЕНЮ_УСПЕХА по неудаче
-+  Управляюцие символы:                  - Смотри по ^H
-+
-+  МЕНЮ может иметь имя SELF - это то же самое меню (текущее).
-+                       PRED - предыдущее в диалоге.
-+       после знака <   -    - не тереть старое меню
-+       после знака <   +    - не перерисовытать старое меню
-+ Формат строк меню:   ( новое )
-+  !&ДЕЙСТВИЕ                          - Запуск в парралель с pipe-ами.
-+  # - коментарий
-+  ! - Имя базы данных : отношение.
-+  { - Имя маски начало описания.
-+  } - Конец описания маски экрана
-+  . - Описание поля
-+  _ - Тело поля ( кол _ столько символов выводится на экран )
-+  : - Разделитель описания полей
-+  | - Резерв для построения рамок
-+  - - Резерв для построения рамок
-+  :3:make=<file maska - Выполнить следущую маску
-+  :run                - Запуск по входу в маску
-+  :task=name          - Запуск задач в маске
-+  :3:task=name        - Запуск задач в поле  (Для последующего развития)
-+  :n_ex               - Работа в парралель ( после выхода из маски задача не завершается )
-+  :chld               - Поискать сынишку
-+  :date               - Подставить время дату в поле
-+  :time               - Подставить время дату в поле
-+  :full               - Дополнять поля ( char->DEL, num->0 )
-+  :char               - Символьное поле
-+  :num                - Числовое поле
-+  :left               - Выравнивать по левому краю
-+  :rigt               - Выравнивать по правому краю
-+  :ndsp               - Не корректировать поле
-+  :div=;              - Символ разделитель полей(def=blank)
-+  :div=ndiv           - Нет разделителя полей
-+  :help=name          - Строка подсказки
-+  =all                - Установить ключ на все следующие поля
-+  :inc=3              - С какой позиции заполнять таблицу
*/
#define MAIN    1
#define USAGE()         {puts (head);exit (1);}
#include "mu.h"
#include "tty.h"
/*#include "dpi.h" */
#include <signal.h>			/* Сигналы обмена */
#include <fcntl.h>
#include <sys/syslog.h>
#include <ctype.h>
#include <locale.h>
#define BACK    1			/* отступ влево для пометки       */
#define MARK    '>'			/* символ пометки */
#define unify( c) ( ((c) & ~0240 ) & 0377 )
/*
** Типы функций
*/
int     chld_int ();
int     pipe_int ();
int     clck_int ();
int     Ks;
extern  int kioutf;
extern  int Red;
int     Stop;
WINDOW  *Win;
Null()
{
}
int (*DPR_CLEAN)();
/*
*+ main()       Головной модуль
*/
main (argc, argv)
	int             argc;		/* Character pointers to and count */
	char          **argv;		/* pk->command line arguments */
{
	static char    *firstmenu = ".root.u";
	struct maska   *ms;
	char           *cp;		/* char pointer */
	char           *file = "prot.u";

	setlocale(LC_ALL, "ru_RU.UTF-8");
	initscr();
	for (argv++, argc--;(cp = *argv) != NULL; argv++, argc--) {
		while (*cp != 0 ) {
			switch (*cp++) {
			    case 'h':
				USAGE();
				break;
			   case 'm':            /* Файл с меню */
				if (--argc)
					firstmenu = *++argv;
				else
					USAGE();
				break;
			   case 'd':            /* Перейти в каталог */
				if (--argc)
					chdir (*++argv);
				else
					USAGE();
				break;
			   case 'p':            /* Вести протокол */
				if (--argc)
					file = *++argv;
				if ((Fp = open(file, O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0){
					printf("Не могу открыть. %s Fp = %d\n", file, Fp);
					exit(1);
				}
				break;
			}               /* switch */
		}                       /* while  */
	}                               /* for    */
	Draw=OFF;          /* Проверять environ */
	dpbeg ();
	mu_set (ON);
	if(getenv("P") != NULL && !strcmp(getenv("P"),"P"))
		Pc++;
	Xdim = dpd ()->xdim;
	Ydim = dpd ()->ydim;
	Rew = ON;
	Del = ON;
	dpo (_CL);

	if (setjmp (Ext) || setjmp (Env)) {
stop:                dpo (_CL);
		dpend ();
		mu_set (OFF);
		closelog(); /* vsi */
		exit (0);
	}
	if ((ms = grep (firstmenu, OFF)) == NULL) {
		err ("Нет такой маски %s", firstmenu);
		longjmp (Ext, _K0);
	}
	signal (SIGCLD, chld_int);
	signal (SIGALRM, clck_int);
	signal (SIGPIPE, pipe_int);
	DPR_CLEAN = Null;
/*        system("stty -a");/**/
       openlog(getlogin(), 0, 0); /* vsi */
	Stop = 0;
	for (;;){
	   choise (ms);
	   if ( Stop++ >= 2 )
	       goto stop;
	}
}
/*
*+ choise ()    Выбор в меню
*/
choise (m)
struct maska *m;
{
	register int    c, cc;          /* команда */
	struct pol     *to, *pol, *save, *pl;
	int             poz;
	static char    *acts[] = {
				  "F0", "F1", "^A", "^H", "F-", "RETURN", NULL
	};
	static char    *helps[] = {
				   " Вход/выход в/из меню ",
				   " Вход во встроенное меню ",
				   " Перерисовать экран ",
				   " Выдать справку ЭТУ ",
				   " Вызвать редактор ",
				   " Выполнить действие ",
				   NULL
	};

	Stop++;
	Maska = m;
	if (m->dir & HLP)
		return(ON);
	if(m->make)
		dosystem(m->make,OFF);
	if(m->dir & RUN )
		send_task(0);   /* Запуск задачи */
	if(m->dir & TAB ){
		if(!m->task && (m->task = Task_Tab)){
			m->task = Task_Tab;
			m->task->count++;
			m->cur = m->pol;
			Task_Tab = 0;
			read_task (OFF,OFF);
		}
	}
	m->dir |= STACK;
	drawmenu (m,ON);
	pol = m->pol;
	for (;;) {
		e_item (pol);
		c = dpi ();
BEGIN:
		switch (c) {
/*                  case _F8:
			if(Ks){
				kioutf = 1;
				dpo(_KS);
			}else{
				kioutf = 0;
				dpo(_KE);
			}
			Ks = ~Ks;
			continue;
*/
		    case _F6 :case _F7 :case _F8 :case _F9 :
			      case _F11:case _F12:case _F13:
		    case _F14:case _F16:case _F17:case _F18:
		    case _F19:case _F20:
			if (pl = (struct pol*)setjmp (Exec)) {
				c = execute(pl);
				FREE(pl->d);
				FREE(pl);
				goto execut;
			}
			if (!in_esc (c))      /* Встроенное меню */
				break;
			continue;
		    case KEY_LEFT:
			l_item ();/* стерли пометку */
			for (to = pol->back; to != pol; to = to->back){
				if( to->key & DSP ){
					pol = to;
					break;
				}
			}
			continue;
		    case KEY_UP:
			l_item ();/* стерли пометку */
			for (to = pol->back; to != pol; to = to->back)
				if(pol->y != to->y && to->key & DSP ){
					save = pol = to;
					for (to = to->back; to != pol; to = to->back){
						if(pol->y != to->y )
							break;
						if(pol->y == to->y && to->key & DSP )
							save = to;
					}
					pol = save;
					break;
				}
			continue;
		    case _nl:          /* Вверх по стеку */
		    case _K(2):          /* Вверх по стеку */
			dpo (' ');	/* стирание метки MARK */
			c = (int) m;
			goto execut;
		    case _cr:          /* выполнить операцию */
			dpo (' ');	/* стирание метки MARK */
			if (Maska->dir & MSK
			     && ( pol->d == NULL || pol->key & ENV )){
				poz = 0;
				goto ED;
			}
			c = execute (pol);
execut:                 if (c > 0){
				m->dir &= ~STACK;
				l_item ();/* стерли пометку */
				if( Rew == OFF )  /* bvg */
				   Del = OFF;     /* bvg */
				delmenu(m,ON);
				if(m->dir & TAB )  /* если не все прочитали */
					read_task(ON,ON);
				if((m->dir & EXIT ) &&  m->task )
					clear_tab(&m->task,OFF);
				return (c);
			}else if (c){   /* Перерисовать экран */
				m->dir |= DISPLAY;
				Maska = m;
				break;
			}
			Maska = m;
		    case KEY_RIGHT:           /* Следующая строка */
			l_item ();/* стерли пометку */
			for (to = pol->next; to != pol; to = to->next)
				if(to->key & DSP ){
					pol = to;
					break;
				}
			continue;
		    case KEY_DOWN:           /* Следующая строка */
			l_item ();     /* стерли пометку */
			for (to = pol->next; to != pol; to = to->next)
				if(pol->y != to->y && to->key & DSP ){
					pol = to;
					break;
				}
			continue;
		    case ctrl (A):
			dpbeg();
			dpo (_CL);
			break;
		    case KEY_F(10):
			dpo (' ');	/* стирание метки MARK */
			longjmp (Ext, _K0);	/* Закончить задачу */
		    case KEY_F(1):          /* minihelp */
			dpline (0, Ydim - 1, acts, helps);
			ceol (0, Ydim - 2);
			if (m->help) {
				register struct maska *o = m->help;

				help (o->x, o->y, o->pol->t, o->pol->d, OFF);
			}else{
				ceol (0, Ydim - 1);
			}
			continue;
		    default:		/* Искать команду с такой буквы */
			if(m->dir & MSK){
				if(!(*pol->t))
					*(pol->t +1) = '\0';
				*pol->t = c;
				poz = 1;
		 ED:            Red = R_SO | R_US | R_TAIL | R_NEXT ;
				c = red (m->x + pol->x, m->y + pol->y, pol->t, pol->l, poz);
				if( c == (int) _cr )
					c = (int) KEY_RIGHT;
				if(pol->key & ENV)
					setenv (pol->d + 1, pol->t);
				goto BEGIN;
			}
			if ( isprint (c) ) {
				cc = toupper (c);
				for (to = pol->next; to != pol; to = to->next){
					if ((toupper (*to->t & 0xff) == cc
						  ||  *to->t & 0xff  == c )
						  && to->key & DSP ) {
						l_item ();
						pol = to;
						break;
					}
			       }
			}
			continue;
		}                       /* endswitch */
		display (OFF);
	}				/* endfor(;;) */
}
/*
*+ execute ()   Выполнение строк меню ( старое )
*/
execute (pol)
	register struct pol *pol;
{
	register char  *s, *end;
	char           *oblom, buf[L_SIZ];
	struct maska   *new, *ret, *m;

	if ((s = pol->d) == NULL){
		err("Пустая команда");
		return ( OFF );
	}
	if (*s == '&')			/* подстановка в строку */
		longjmp (Env, (int) pol->t);  /* Тащим строку вверх */
	m = Maska;
	if ( *s == '$' && pol->key & ENV) {                /* подстановка в строку */
		char           *ss, *se;
		int             x, y, num;

		if( (ss = index (pol->t, '[')) != 0 ){
		   for (num = 0, se = ++ss; *se != ']';)
			   buf[num++] = *se++;
		   buf[num] = '\0';
		   x = m->x + pol->x + (ss - pol->t) ;
		   y = m->y + pol->y;
		   Red = R_SO | R_TAIL | R_NEXT ;
		   red (x+1, y, buf, num, 0);
		   strncpy (ss, buf, num);
		   for (--se; ss <= se && *se == '\0'; *se-- = ' ');
		   setenv (s + 1, buf);
		}
		return (OFF);
	}
	if (*s == '<') {		/* ТЕКСТ:<МЕНЮ_УСПЕХА:команда:МЕНЮ_НЕУДАЧИ. */
		s = strcpy (buf, ++s);
		if (*s == '-') {	/* Не затирать текущее меню */
			Del = OFF;
			s++;
		}
		if (*s == '+') {	/* Не перерисовывать меню */
			Rew = OFF;
			s++;
		}
		GREP(end,s,':');        /* <МЕНЮ_УСПЕХА */
		if (*end == ':') {	/* :команда  */
			*end++ = '\0';
			GREP(oblom,end,':');
			if (*oblom == ':'){      /* :<МЕНЮ_НЕУДАЧИ. */
				*oblom++ = '\0';
				*oblom++ = '\0';
			}else
				oblom = NULL;
			if (dosystem (end, *end == ' ' ? OFF : ON) && oblom != NULL)
				s = oblom;
		}
		if (!strcmp (s, "PRED"))        /* Предыдущее меню */
			return ((int)m);
		else if (!strcmp (s, "SELF"))   /* Это же меню */
			return (ON);
		if ((new = grep (s, OFF)) == NULL) {
			err ("Нет такой маски %s", s);
			return (OFF);
		}
		if (new->dir & STACK)           /* уже вызывалась */
			return ((int)new);
		delmenu (m,ON);
		ret = (struct maska *) choise (new);
		if (ret == new || ret == m)
				return (ON);
		return ((int)ret);
	}
	if( pol->key & SYS1 )          /* vsi */
	   syslog_v(LOG_LOCAL0, "START %s", pol->t);     /* vsi */
	dosystem (s, *s == ' ' ? OFF : ON);
	if( pol->key & SYS1 )          /* vsi */
	   syslog_v(LOG_LOCAL0, "STOP  %s", pol->t);     /* vsi */
	return (ON);
}
/*
*+ dosystem ()  Выполнить команду системы.
*/
dosystem (s, key)
	char           *s;
{
       extern WINDOW  *Win;
	int             i;
	char           *ssetenv();

	/* Смотрим setenv XXXX yyyy */
	if(!(s=ssetenv(s)))
	   return(0);
	dpo (_CL);
	DPIS ("Минуточку...\n");
	dpend ();    /**/
	signal (SIGCLD, SIG_DFL);
	i = system (s) >> 8;
	signal (SIGCLD, chld_int);
	dpbeg ();    /**/
	if (key) {
		DPIS ("---нажми кнопку---");
		dpi ();
	}
/*        dpo ('\017');   /* SI */
	dpo (_CL);
	Rew = ON;
	Del = ON;
	return (i);
}
/*
*+ in_esc()     Выбрать и выполнить команду
*/
in_esc (c)
{

	if(c == F_CLEAR){
		if(Maska->dir & MSK){
			clear_pol(Maska);
			write_task(c);
			draw_pol (Maska, OFF);
		}
		return (ON);
	}
	if(c == (int)_K1 || (c == F_DO && Maska->dir & OLD)){
		return(in_menu ());/* Встроенное меню */
	}
	if(c == F_DO){
		if (Maska->help) {
			register struct maska *o = Maska->help;
			c = help (o->x, o->y, o->pol->t, o->pol->d, ON);
			help (o->x, o->y, o->pol->t, o->pol->d, OFF);
			if(c == -1)
				return (ON);            /* Нет операций */
			c += (int)_F6;
		}
		return (ON);            /* Нет операций */
	}
	if(read_task(ON,OFF)){
		return (ON);
	}
	if(send_task (ON, c))
		read_task(Maska->task->dfr?OFF:ON,OFF);
	return (ON);
}
/*
*+ in_menu ()   Встроенное меню
*/
in_menu ()
{
	static char    *acts[] = {
				  "exit", "shell", "files", "cd", "nothing", 0
	};
	static char    *helps[] = {
				   "выход из системы",
				   "выход в СиШелл",
				   "список файлов текущего каталога",
				   "изменить текущий каталог",
				   "выход из командного меню",
				   NULL
	};
	int             i;
	char            buf[L_SIZ];

	i = dpline (0, dpd ()->ydim - 1, acts, helps);
	ceol (0, dpd ()->ydim - 2);
	ceol (0, dpd ()->ydim - 1);
	switch (i) {
	    case -1:
	    case 4:
		return (ON);           /* Нет операций */
	    case 0:
		longjmp (Ext, ON);
	    case 1:
		dosystem ("csh -f", OFF);
		break;
	    case 2:
		dosystem ("ls -sa", ON);
		break;
	    case 3:
		buf[0] = 0;
		DPIM ("Введите имя нового каталога :");
		Red = R_SO | R_TAIL | R_NEXT ;
		red (30, dpd ()->ydim - 1, buf, 14, 0);
		if (chdir (buf) < 0)
			err ("\t\tОблом.\t\t");
		return (ON);
	}
	return (OFF);
}
/*
*+ e_item ()    Выделение строк
*/
e_item (k)
	register struct pol *k;
{
	register int x = Maska->x + k->x;
	register int y = Maska->y + k->y;

	Maska->cur = k;
	dpp (x,y);
	attron (A_REVERSE);
	drawline (Maska);
	attroff (A_REVERSE);
	dpp (x - BACK, y);
	dpo (MARK);
	dpo ('\b');
}
/*
*+ l_item () Гашение строки
*/
l_item ()
{
	register int x = Maska->x + Maska->cur->x;
	register int y = Maska->y + Maska->cur->y;

	dpp ( x -BACK, y);
	dpo (' ');			/* стирание метки MARK */
	dpp ( x, y);
	if(Maska->dir & MSK )
		attron ( A_UNDERLINE );
	drawline (Maska);
	if(Maska->dir & MSK )
		attroff ( A_UNDERLINE );
}
/*
*+ drawline ()  Рисовать строку
*/
drawline (ms)
struct maska *ms;
{
	register char  *s = ms->cur->t;
	register int    x = ms->x + ms->cur->x;
	register int    l = ms->cur->l;

       if( ms->cur->key & MENU){
	   dpo('>');
       }else{
	   dpo(' ');
       }
	while (*s && x++ < Xdim){
		dpo (*s++);
		l--;
	}
	if( l > 0 )
		if(Pc)
			dpn(l,'_');
		else
			dpn(l,' ');
}
/*
*+ err ()       Выдать ошибку
*/
//err (fmt, args)
#include <stdarg.h>
void err (char *fmt,...){
	va_list         args;
	FILE            _strbuf;
	char            buf[200];
	int             yd=Ydim - 2, i;

/*        _strbuf._flag = _IOWRT /* +  _IOSTRG /* bvg for SCO*/;
/*        _strbuf._ptr = buf;
/*        _strbuf._cnt = 200;
/*        _doprnt (fmt, &args, &_strbuf);/**/
/*        putc ('\0', &_strbuf);           /**/
	va_start (args, buf);
	vsprintf (buf, fmt, &args);
	va_end   (args);
	dpp (0,yd);
	attron (A_REVERSE);
	dpc (RED);
	if(Pc)
		attron ( A_UNDERLINE );
	dps (buf);
	if(Pc)
		attroff ( A_UNDERLINE );
	dpc (BLACK);
	attroff (A_REVERSE);
	i = dpi ();
	dpp (0,yd);
	clrtoeol();
       /*  region(0,Ydim - 1);   **/
	if (Maska && Maska->help && Maska->help->y == yd ) {
		register struct maska *o = Maska->help;
		help (o->x, o->y, o->pol->t, o->pol->d, OFF);
	}
	if ( i == F_CLEAR && Maska->dir & MSK ){
		clear_pol(Maska);
		draw_pol(Maska,1);
	}
	return;
}
syslog_v(int l1, char *s , char *m){
char   alt7[]={
/*6*/  0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F
/*7*/ ,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x21,0x5D,0x5E,0x7F
/*8*/ ,0x61,0x62,0x77,0x67,0x64,0x65,0x76,0x7A,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70
/*9*/ ,0x72,0x73,0x74,0x75,0x66,0x68,0x63,0x7E,0x7B,0x7D,0x78,0x79,0x78,0x7C,0x60,0x71
/*A*/ ,0x61,0x62,0x77,0x67,0x64,0x65,0x76,0x7A,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70
/*B*/ ,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A
/*C*/ ,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A
/*D*/ ,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A
/*E*/ ,0x72,0x73,0x74,0x75,0x66,0x68,0x63,0x7E,0x7B,0x7D,0x78,0x79,0x78,0x7C,0x60,0x71
/*F*/ ,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x2A,0x7F
};
   char    mes[1024];
   char   *ss, *f7;

   for(ss = mes ; *m != 0; m++ ){
      if( (0xff & *m) >= 0x60 ){
	  *ss++ = alt7[ (0xff & *m) - 0x60];
      }else
	   *ss++ = *m;
   }
   *ss = 0;
   syslog(LOG_INFO|LOG_LOCAL0, s, mes);     /* vsi */

}
