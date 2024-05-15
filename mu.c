
char    head[] = "\n\
****************************************************************\n\r\
*АО РТИ им. Академика А.Л. Минца                               *\n\r\
	       2022/09/09  Булов B.Г.(V.Bulov@gmail.com) ver3.0*\n\r\
*ВАЗ ПАП БРАС  24.10.1991  Булов B.Г.(@bvg) version 2.0        *\n\r\
*ВАЗ ПАП БРАС  07.03.1990  в редакции Булова В.Г. (bvg)        *\n\r\
****************************************************************\n\r\
*  Программа работы с меню в UNIX ( mu )                       *\n\r\
*  Вызов:  mu [-m меню] [-d каталог] [-p file] [-h]            *\n\r\
*  где:    -m меню    = файл с меню       ( .root.u )          *\n\r\
*          -d каталог = перейти в каталог ( . )                *\n\r\
*          -p file    = протол в файл     ( prot.u  )          *\n\r\
*          -h         = справка эта                            *\n\r\
*  git clone  https://github.com/bulov/mu                      *\n\r\
****************************************************************\n\r";
/*
-+ Дата 07.03.90   ВАЗ ПАП БРАС       в редакции Булова В.Г. (bvg)
-+ ВАЗ ПАП БРАС   24.10.91  Булов B.Г.(@bvg) version 2.0
-+
-+
-+ Формат строк меню:   ( старое )
-+  #ИНСТРУКЦИЯ                           - помещается в область help-а.
-+  @xx yy                                - координаты привязки меню
-+  !ДЕЙСТВИЕ                             - выполняется при входе в это меню.
-+  ТЕКСТ[       ]:$Environ               - издать setenv и сохранить .setenv.u
-+  ТЕКСТ[       ]:\$Environ              - \$ если вызываем из bash  mu <<EOL
-+  ТЕКСТ:&                               - перенести в строку редактирования
-+  ТЕКСТ:ДЕЙСТВИЕ                        - выполнить указанное действие.
-+  ТЕКСТ:<ИМЯ_ФАЙЛА_С_МЕНЮ               - перейти (прочитать) в новое меню.
-+  ТЕКСТ:<<ИМЯ_ФАЙЛА_С_МЕНЮ stdin        - перейти (прочитать) в новое меню из stdin .
-+  ТЕКСТ:<ИМЯ_ФАЙЛА_С_МЕНЮ:ДЕЙСТВИЕ      - перейти и попутно выполнить действие
-+  ТЕКСТ:<МЕНЮ_УСПЕХА:команда:<МЕНЮ_НЕУД. - По успеху в МЕНЮ_УСПЕХА по неудаче
-+  Управляюцие символы:                  - Смотри по ^H
-+
-+  МЕНЮ может иметь имя SELF - это то же самое меню (текущее).
-+                       PRED - предыдущее в диалоге.
-+       после знака <   -    - не тереть старое меню
-+       после знака <   +    - не перерисовытать старое меню
-+ Формат строк меню:   ( новое )
-+  !&ДЕЙСТВИЕ                - Запуск в парралель с pipe-ами.
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
#define USAGE()         { puts (head); exit (0) ;}
#include "mu.h"
#include "tty.h"
/*#include "dpi.h" */
#include <signal.h>			/* Сигналы обмена */
#include <fcntl.h>
#include <sys/syslog.h>
#include <ctype.h>
#include <locale.h>
#include <unistd.h>
#include <setjmp.h>
#include <time.h> // time_t
#define BACK     1                      /* отступ влево для пометки       */
#define MARK    '>'			/* символ пометки */
#define unify( c) ( ((c) & ~0240 ) & 0377 )
/*
** Типы функций
*/
int     Ks;
extern  int kioutf;
extern  int Red;
int     Stop;
WINDOW  *Win;
char    *rmcup;
char    *pidFirstMU = "pidFirstMU";     /* первый в стеке mu  */
int Null(){
}
int (*DPR_CLEAN)();
/*
*+ main()       Головной модуль
*/
int main (argc, argv)
	int             argc;		/* Character pointers to and count */
	char          **argv;		/* pk->command line arguments */
{
	static char    *firstmenu = ".root.u";
	struct maska   *ms;
	char           *cp;		/* char pointer */
	char           *file = "prot.u";
	long           jmpErr;
	int            keyHelp=0,fFMU;
	char          *pFMUc;
	setlocale(LC_ALL, "ru_RU.UTF-8");
	for (argv++, argc--;(cp = *argv) != NULL; argv++, argc--) {
		while (*cp != 0 ) {
			switch (*cp++) {
			    case 'v':
			       { puts ("ver 3.0-2024-02-03\n"); exit (0) ;}
			    case 'h':
				USAGE();
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
	Win = initscr();
	Draw=OFF;          /* Проверять environ */
	dpbeg ();
	rmcup = tigetstr("rmcup");     // Переключить на терминал для scroll dp_io.c
	mu_set (ON);
	if(getenv("P") != NULL && !strcmp(getenv("P"),"P"))
		Pc++;
	Xdim = dpd ()->xdim;
	Ydim = dpd ()->ydim;
	Rew = ON;
	Del = ON;
	dpo (_CL);
	if ( (jmpErr = setjmp (Ext)) || setjmp (Env)) {
stop:
		dpend ();
		mu_set (OFF);
		closelog(); /* vsi */
		if (keyHelp )
		   system ("mu -h");
		if ( NULL != (pFMUc = getenv (pidFirstMU)) ){  //первый в стеке mu
		    sscanf(pFMUc,"%d",&fFMU);
		    if ( fFMU != getpid() ){
			killpg(getpgid(0), SIGTERM);                 // завершить все процессы по f10
		    }
		}
		endwin();
		system("clear");
		return (0);
	}
	if ( !isatty(fileno(stdin)) )          // если не /dev/tty
	   firstmenu = RSTDIN;
	if ((ms = grep (firstmenu, OFF)) == NULL) {
	   keyHelp=1;
	   err ("Нет такой маски %s или %s", firstmenu, RSTDIN);
	   longjmp (Ext, _K0);
	}
	signal (SIGCLD,  chld_int);
	signal (SIGALRM, clck_int);
	signal (SIGPIPE, pipe_int);
	DPR_CLEAN = Null;
	openlog(getlogin(), 0, 0); /* vsi */
	Stop = 0;
	for (;;){
	   choise (ms);
	   if ( Stop++ >= 2 )
	       goto stop;
	}
}
struct maska *choise (struct maska *m){      //  *+ choise ()    Выбор в меню
	extern WINDOW  *Win;
	MEVENT event;
	register int    cc;          /* команда */
	struct pol     *to, *pol, *save, *pl;
	int             c,poz,y;
	struct maska *mm, *mt;
	static char    *acts[] = {
				  "F1", "Esc", "F3", "F9", "F12", "RETURN", NULL
	};
	static char    *helps[] = {
				   " Выдать справку ЭТУ ",
				   " Вход/выход в/из меню ",
				   " Перерисовать экран ",
				   " Вход во встроенное меню ",
				   " Вызвать редактор ",
				   " Выполнить действие ",
				   NULL
	};

	Stop++;
	Maska = m;
	if (m->dir & HLP)
		return((struct maska *)ON);
	if(m->make)
	       dosystem(m->make,OFF);
	if(m->dir & RUN )
		send_task(0,0);   /* Запуск задачи */
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
	mousemask(ALL_MOUSE_EVENTS, NULL); // Get all the mouse events
	keypad(Win, TRUE);
//        err("<<<%s>>>",m->menu);
	for (;;) {
		e_item (pol);
		c = dpi ();
//                        err("пришло %0X",c);
BEGIN:
		switch (c) {
		   case KEY_MOUSE:
		       if(getmouse(&event) == OK){   /* When the user clicks mouse button */
			   y = event.y - m->y;
			   if ( event.bstate & BUTTON1_DOUBLE_CLICKED ){
			       if ( y == pol->y ){
				   goto goCR;
			       }else if ( y == m->MM ) {  //строка менюMышь
				   switch (MenuMouse(event.x - m->x)){
				       case  1: longjmp (Ext, _K0); // F10-Выход
				       case  2: goto goEsc        ; // ESC-Вверх
				       case  3: goto goCR         ; // Enter-Делать
				       case  4: goto goK3         ; // F3-Перерисовать
				       case  5:                     // F9-bash
					   dosystem ("/bin/bash -f", OFF);
					   goto goK3;
				   }
			       }
			   }
			   for (mm=(struct maska *)Null , mt = Head; mt != NULL; mt = mt->next){
			       if (mt->dir & DISPLAY){
				   if ( (struct maska *)Null == mm ) mm = mt;
///**/                               err("<<<1 %s %d %d   %d %d  %d %d  MM=%d y=%d %d %d >>>",mt->menu,event.x,event.y ,mt->x,mt->y ,mt->xW,mt->yW,m->MM,y,mm->sq,mt->sq);
				   if ( event.x >  mt->x  && event.y >  mt->y  // мышь в окне
				     && event.x <= mt->xW && event.y <= mt->yW  ){
///**/                               err("<<<2 %s %d %d   %d %d  %d %d  MM=%d y=%d %d %d >>>",mt->menu,event.x,event.y ,mt->x,mt->y ,mt->xW,mt->yW,m->MM,y,mm->sq,mt->sq);
				       if ( mm->sq > mt->sq ){
					   mm = mt;   // Из первого меньшего внутри
///**/                                       err("<<<3 %s>>>",mt->menu);
				       }
				   }
			       }
			   }
			   if ( (struct maska *)Null != mm && Maska != mm ){
///**/                                err("<<<4 %s %d %d   %d %d  %d %d >>>",mm->menu,event.x,event.y ,mm->x,mm->y ,mm->xW,mm->yW);
				l_item ();/* стерли пометку */
				Maska = m = mm;
				pol = m->pol;
				y = event.y - m->y;
			   }
			   for (to = pol->next; to != pol; to = to->next){
				if ( y == to->y && to->key & DSP ){
				   l_item ();/* стерли пометку */
				   pol = to;
				   break;
				}
			   }
		       }
		       continue;
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
//                        if (pl = (struct pol*) setjmp (Exec)) {
			if (pl = ( struct pol *)setjmp (Exec)) {
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
		    case  27:          /* Esc or Alt
		    case _nl:          /* Вверх по стеку */
		    case _K(2):        /* Вверх по стеку */
goEsc:                  dpo (' ');     /* стирание метки MARK */
			c = (long int) m;
			goto execut;
		    case _cr:          /* выполнить операцию */
goCR:                   dpo (' ');      /* стирание метки MARK */
			l_item ();/* стерли пометку */
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
				return (m);
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
		    case ctrl (A):         // Перерисовать экран
		    case _K(3):
goK3:                   dpbeg();
			dpo (_CL);
			break;
		    case KEY_F(10):
			longjmp (Ext, _K0);	/* Закончить задачу */
		    case KEY_F(1):          /* minihelp */
			dpline (0, Ydim - 1, acts, helps);
			ceol (0, Ydim - 2);
			if (m->help) {
				register struct maska *o = m->help;
				help (o->x, o->y, &o->pol->t, &o->pol->d, OFF);
			}else{
				ceol (0, Ydim - 1);
			}
			continue;
		    default:            /* Искать команду с такой буквы без переключения на русский*/
			if(m->dir & MSK){
				if(!(*pol->t))
					*(pol->t +1) = '\0';
				*pol->t = c;
				poz = 1;
		 ED:            Red = R_SO | R_US | R_TAIL | R_NEXT ;
				c = red (m->x + pol->x, m->y + pol->y, pol->e, pol->l, poz);
//                                c = red (m->x + pol->x, m->y + pol->y, pol->t, pol->l, poz);
				if( c == (int) _cr )
					c = (int) KEY_RIGHT;
				if(pol->key & ENV)
					setenv (pol->d + 1, pol->t, 1);
				goto BEGIN;
			}
			if ( isprint (c) ) {
				cc = toupper (c);
				for (to = pol->next; to != pol; to = to->next){
					if ((( to->h == cc ) || to->h == c ) && to->key & DSP ) {
						l_item ();
						pol = to;
						break;
					}
			       }
			}
			continue;
		}                       /* endswitch */
		display ();
//                display (OFF);
	}				/* endfor(;;) */
}
int execute (register struct pol *pol){         //   *+ execute ()   Выполнение строк меню ( старое )
	register char  *s, *end, *v, *b;
	char           *oblom, buf[L_SIZ],tmp[L_SIZ];
	struct maska   *new, *ret, *m;
	int            l;
	time_t         beginT ,endT;

	if ((s = pol->d) == NULL){
		err("Пустая команда");
		return ( OFF );
	}
	if (*s == '&')			/* подстановка в строку */
		longjmp (Env, (long int) pol->t);  /* Тащим строку вверх */
	m = Maska;
	if ( *s == '$' && pol->key & ENV) {                /* подстановка в строку */
		char           *ss, *se , *sn;
		int             x, y, num, lb, le;

		if( (sn = ss = index (pol->t, '[')) != 0 ){
		   for (num = 0, se = pol->e; 0 != *se ;)  buf[num++] = *se++;
		   buf[num] = '\0';
		   for (num = 0;  *sn != ']' && 0 != *s; sn++ , num++ );
		   x = m->x + pol->x + (ss - pol->t) - tUTF8(pol->t ,ss - pol->t);
		   y = m->y + pol->y;
		   Red = R_SO | R_TAIL | R_NEXT ;
		   red (x+2, y, buf, num-1, 0);            //   V
		   for (b=buf; ' ' == *b && 0 != *b; b++)  // "   xxxx   "
			;                                  //       V
		   for (v=buf; 0 != *b; v++, b++)          // "xxxx      "
		       *v=*b;                              //
		       *v=0;                               //          V
		   for (; b != v ; b--)                    // "xxxx   000"
			*b=0;                              //       V
		   for (; ' ' == *b ; b--)                 // "xxxx000000"
		       *b=0;
//                   printf("<%s><%s><%s>\n",buf,v,b);
		   setenv (s + 1, buf, 1);
		   setenv (strcat(strcpy(tmp,"MU_"),s+1), "", 1);
		   lb = strlen(buf);
		   le = strlen(pol->e);
		   if ( le < lb){
		       FREE   (pol->e);
		       MALLOC (pol->e, buf, lb+1);
		   }
		   for (num = 0, se = pol->e; 0 != buf[num] ; num++ ,se++) *se = buf[num];
		   *se = '\0';
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
			return ((long int)m);
		else if (!strcmp (s, "SELF"))   /* Это же меню */
			return (ON);
		if ((new = grep (s, OFF)) == NULL) {
			err ("Нет такой маски %s", s);
			return (OFF);
		}
		if (new->dir & STACK)           /* уже вызывалась */
			return ((long int)new);
		delmenu (m,ON);
		ret = (struct maska *)choise (new);
		if (ret == new || ret == m)
				return (ON);
		return ((long int)ret);
	}
	if( pol->key & SYS1 ){          /* vsi */
	    time (&beginT); // note time before execution
	    syslog(LOG_INFO|LOG_LOCAL0, "START %s <%0X>", pol->t ,pol);     /* vsi */
	}
	dosystem (s, *s == ' ' ? OFF : ON);
	if( pol->key & SYS1 ){          /* vsi */
	   int h,m,s;
	   time (&endT); // note time after execution
	   double duration = difftime (endT,beginT);
	   s=(int)duration%60;
	   m=((int)duration%3600)/60;
	   h=duration/3600;
	   syslog(LOG_INFO|LOG_LOCAL0, "STOP  %s <%0X> %d:%.2d:%.2d", pol->t, pol ,h,m,s);     /* vsi */
	}
	return (ON);
}
int dosystem (char *s,int key){        //  *+ dosystem ()  Выполнить команду системы.
       extern WINDOW  *Win;
	int             i;
	char           *ssetenv();

	if(!(s=ssetenv(s)))    /* Смотрим !setenv XXXX=yyyy */
	   return(0);
	dpo (_CL);
	DPIS ("Минуточку...\n");
	dpend ();    /**/
	signal (SIGCLD, SIG_DFL);
	system ("clear" );
	system ("echo Минуточку..." );
	i = system (s) >> 8;
	signal (SIGCLD, chld_int);
	dpbeg ();    /**/
	if (key) {
		mousemask(0, NULL); // Get all the mouse events
		DPIS ("---нажми кнопку---");
		dpi ();
		mousemask(ALL_MOUSE_EVENTS, NULL); // Get all the mouse events
	}
	dpo (_CL);
	Rew = ON;
	Del = ON;
	return (i);
}
int in_esc (int c){        //  *+ in_esc()     Выбрать и выполнить команду
	if(c == F_CLEAR){
		if(Maska->dir & MSK){
			clear_pol(Maska);
			write_task(c);
			draw_pol (Maska, OFF);
		}
		return (ON);
	}
	if(c == (int)_K1 || (int)_K(9) || (c == F_DO && Maska->dir & OLD)){
		return(in_menu ());/* Встроенное меню */
	}
	if(c == F_DO || c == (int)_K(9) ){
		if (Maska->help) {
			register struct maska *o = Maska->help;
			c = help (o->x, o->y, &o->pol->t, &o->pol->d, ON);
			help (o->x, o->y, &o->pol->t, &o->pol->d, OFF);
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
int in_menu (){        //  *+ in_menu ()   Встроенное меню
	static char    *acts[] = {
				  "exit", "shell", "files", "cd", "nothing", 0
	};
	static char    *helps[] = {
				   "выход из системы",
				   "выход в Шелл",
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
		dosystem ("/bin/bash -f", OFF);
		break;
	    case 2:
		dosystem ("ls -lsa|less", ON);
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
void e_item (register struct pol *k){       //  *+ e_item ()    Выделение строк
       if ( 0x0 == k ){
	   printf ("ошибка описания поля в маске %s", Maska->menu);
	   longjmp (Ext, 222);
	}
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
void l_item (){         //  *+ l_item () Гашение строки
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
void drawline (struct maska *ms){        // *+ drawline ()  Рисовать строку
	register char  *s;// = ms->cur->t;
	register char  *e;// = ms->cur->e;
	register int    x;// = ms->x + ms->cur->x;
	register int    l;// = ms->cur->l;
	int             key, keyE=0;

       if ( ms && ms->cur ){
	   s = ms->cur->t;
	   e = ms->cur->e;
	   x = ms->x + ms->cur->x;
	   l = ms->cur->l;
       }else{
	   printf("Ошибка в Рисовать строку drawline ms=%0X ms->cur=%0X",ms, ms->cur);
	   longjmp(Ext,777);
       }
       if( ms->cur->key & MENU){
	   dpo('>');
       }else{
	   dpo(' ');
       }
       key=0;
       for (;*s && x++ < Xdim;s++){
	       if ( ']' == *s ){
		   keyE=1;
		   key=0;
	       }
	       if  ( key  ) {
		   if ( 0 != *e ){
		       if ( fUTF8(e) )
			   dpo (*e++);
		       dpo (*e++);
		   }else{
		       dpo (*s);
		   }
	       }else{
		   if ( keyE && 0x0 != e && 0 != *e ){ // RSTDIN
		       dpo ('>');
		   }else
		       dpo (*s);
	       }
	       if ( '[' == *s ){
		   if ( NULL != e ){
		       key++;
		   }
	       }
	       l--;
	}
	if( l > 0 )
		if(Pc)
			dpn(l,'_');
		else
			dpn(l,' ');
}
#include <stdarg.h>
void err (char *fmt,...){          //  *+ err ()       Выдать ошибку
	va_list         args;
	FILE            _strbuf;
	char            buf[200];
	int             yd=Ydim - 2, i;

/*        _strbuf._flag = _IOWRT /* +  _IOSTRG /* bvg for SCO*/;
/*        _strbuf._ptr = buf;
/*        _strbuf._cnt = 200;
/*        _doprnt (fmt, &args, &_strbuf);/**/
/*        putc ('\0', &_strbuf);           /**/
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
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
		help (o->x, o->y, &o->pol->t, &o->pol->d, OFF);
	}
	if ( i == F_CLEAR && Maska->dir & MSK ){
		clear_pol(Maska);
		draw_pol(Maska,1);
	}
	return;
}
