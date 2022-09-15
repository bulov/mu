#include   "sys/errno.h"
#include <curses.h>
#include <setjmp.h>			/* Обработка ошибочных явлений */
#include <stdlib.h>
#include <string.h>
#include "rob.h"


/*
** Symbol Definitions
*/
#define OFF      0                       /* Выключить ключ */
#define ON       -1                      /* Его включить */
#define L_SIZ   128			/* Длинна стоки  */
					/* for maska->dir */
#define OLD     0x01                    /* Старое меню */
#define MSK     0x02                    /* Маска */
#define HLP     0x04                    /* Help меню */
#define TAB     0x08                    /* Таблица */
#define STACK   0x10                    /* Маска в стеке (для больших прыжков ) */
#define CHLD    0x20                    /* Поискать сынишку */
#define EXIT    0x40                    /* Завершить выполнение задачи */
#define DISPLAY 0x80                    /* Маска включена */
#define RUN     0x100                   /* Запуск по входу в маску */
#define FULL    0x800                   /* Дополнять поля */
					/* for pole->key */
#define TASK    0x10                    /* Запуск задач */
#define DSP     0x20                    /* Поле можно корректировать */
#define ENV     0x40                    /* environ */
#define LEFT    0x80                    /* Выравнивать по левому краю */
#define CHAR    0x100                   /* Символьное поле */
#define DATE    0x200                   /* Дата время */
#define TIM     0x400                   /* Дата время */
#define PTAB    0x800                   /* Работа с таблицей */
#define MENU    0x1000                  /* Вызов меню */
#define SYS1    0x2000                  /* Вызов syslogd */

/*
** Structrure Definitions
*/
struct poz {
	char            x, y, l;        /* Координаты поля и его длинна */
	char           *t;		/* Куда писать текст */
	struct poz     *next;		/* Следущее поле */
};
struct pol {
	char            x, y, l;        /* Координаты поля и его длинна */
	char           *t;		/* Куда писать текст */
	char           *e;              /* Куда писать текст environ*/
	char           *d;              /* Строка действий */
	struct pol     *next;		/* Следущее поле */
	struct pol     *back;		/* Предыдущее поле */
	int             key;		/* Описатель поля */
	short           nom;            /* Номер поля */
	short           lev;            /* Уровень syslog */
};
struct maska {
	struct maska   *next;		/* Указатель на следущую маску */
	struct maska   *help;           /* Строка подсказки */
	struct poz     *ram;		/* Графика ( построение таблицы) */
	struct poz     *tab;		/* Постоянная часть */
	struct pol     *pol;		/* Заполняемые поля */
	struct pol     *cur;            /* Текущее поле */
	char           *file;		/* Имя файла */
	char           *menu;		/* Имя структуры */
	char           *make;           /* Выполнить при входе в меню */
	char           *prl;            /* Запустить в параллель */
	char            divide;         /* Символ разделитель полей */
	int             x, y;           /* Точка привязки */
	int             dir;            /* Управление */
	struct task {
		struct task    *next;	/* Указатель на следущую задачу */
		char           *name;   /* Имя транзакции(строка под запуск) */
		int             count;  /* Из скольки масок вызвана */
		short           lenth;	/* Длина записи */
		long            time;	/* Время последнего обращения */
		int             pid;	/* Индентификатор процесса */
		int             dfw;	/* Канал на запись */
		int             dfr;    /* Канал на чтение */
		int             dfe;    /* Канал на чтение ошибок */
		int             err;	/* Ошибки завершение */
	}              *task;		/* Обслуживающая задача */
};
/*
** Global Variables
*/
#ifdef MAIN
#       define  EXTERN
#else
#       define  EXTERN  extern
#endif
extern errno;
EXTERN WINDOW       *Win;
EXTERN struct maska *Head;		/* Заголовок списка масок */
EXTERN struct maska *Maska;             /* Текущая маска */
EXTERN struct task  *H_task;            /* Заголовок списка задач */
EXTERN struct task  *Task_Tab;          /* Связать с задачей отцом */
EXTERN struct pol   *Pol;               /* Указатель на тек. поле */
EXTERN char         *File;              /* Имя файла */
EXTERN char          Str;               /* Текущая строка маски */
EXTERN int           Rew;               /* перерисовка меню включена */
EXTERN int           Del;               /* Удаление старого меню включено */
EXTERN int           Disp;              /* Перерисовать меню */
EXTERN int           Fp;                /* Протокол в файл */
EXTERN int           Xdim, Ydim;        /* Размер экрана */
EXTERN jmp_buf       Env, Ext, Exec;    /* Environment ptr for longjump */
EXTERN jmp_buf       E_pipe, E_clck;    /* Environment ptr for longjump */
EXTERN int           Pc;                /* Работа с PC VGA */
EXTERN int           Draw;              /* Проверять environ */
/*
** Типы функций
*/
struct maska   *grep ();
struct pol     *fin ();
char           *index (), *getenv (), *strcpy();
long            time ();
/*
** Macro Definitions
*/
#define SIZ(S)          sizeof ( struct S )
#define DPIS(S)         (attron(A_REVERSE), dps(S), attroff(A_REVERSE), dpo('\b'));
#define DPIM(S)         (attron(A_REVERSE), dpmsg(0, Ydim - 1, S), attroff(A_REVERSE),dpo('\b'));
#define GREP(DD,SS,VV)  for (DD = SS; *DD && *DD != VV; DD++);
#define FREE(M)         {errno = 0; free(M);if(errno)err("free");}
#define MALLOC(M,V,L)   {M=(char*)malloc(L+1);if(M == NULL)err("malloc");else strcpy  (M,V);}
#define CALLOC(M,V,L)   {M=(char*)malloc(L+1);if(M == NULL)err("malloc");else{strncpy (M,V,L); *(M+L)='\0';}}
#define TIME(W)         {struct tm      *t;        long            lt;  \
			time (&lt);  t = localtime (&lt);               \
			printmsg ("\n W  19%02d/%02d/%02d %02d:%02d",   \
			t->tm_year, 1 + t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min);}

#define F_TAB   (int)_F6
#define F_CLEAR (int)_F7
#define F_HELP  (int)_F15
#define F_DO    (int)_F16
#define RSTDIN  "stdin"

extern WINDOW  *Win;

int dpp(int x, int y );
int dpo(int cc );
int dpline( int x, int y, char **list ,char **list2 );
int dpi();
int dpp(int x, int y );
void dpn(int n, int c );
void ceol(int x, int y);
void tab_date();                                       //  *+ tab_date()   Заглушка
void drawmenu (struct maska   *m, int key_env);        //  *+ drawmenu()   Рисовать меню
void draw_pol (struct maska   *m, int key_env);        //  *+ draw_pol()   Рисовать pole
int command (struct  task   *ts, struct packet  *pk);  //  *+ command ()   Интерпритация комманд от задач.
void clr_v(register struct pol *l, int i);             //  *+ clr_v()      Очистить ключ для всех значений
void clear_pol(struct maska   *m);                     //  *+ clear()      Очистить все поля.
void set_v(register struct pol *l, int i);             //  *+ set_v()      Установить ключ для всех значений
struct pol *fin(int i);                                //  *+ fin()        По номеру найти указатель на поле
int collect(char *b);                                  //  *+ collect()    Собрать в буфер поля отправки
void clear_tab (struct task   **tts,int key);          //  *+ clear_tab()  Очистить вход в таблице
void miracle(unsigned char *s);                        //  *+ miracle()    Записать полученную запись в роля
int write_task (int comm);                             //  *+ write_task() Послать сообщение транзакции
int read_task (int key,int flag);                      //  *+ read_task()  Получить сообщение от транзакции
int send_task (int key,int comm);                      //  *+ send_task()  Запустить или послать сообщение транзакции по имени
void err (char *fmt,...);                              //  *+ err ()       Выдать ошибку
void pipe_int ();                                      //  *+ pipe_int()   Pipe interrupt handler
void chld_int ();                                      //  *+ chld_int()   Zombie state eksel-moksel
void clck_int ();                                      //  *+ clck_int()   Timer interrupt handler
void dps(char *str );                                  //                  ВЫВОД СТРОКИ СИМВОЛОВ ДО \0
void dpmsg (int x,int  y,char   *s);
void dpc (int c);                                      //                  УСТАНОВИТЬ ЦВЕТ
void region(int tl,int bl);                            //                  Открыть окно
void dpbeg();
void dpend();                                          //                  ПЕРЕКЛЮЧЕНИЕ В ОБЫЧНЫЙ РЕЖИМ
void s_tab (int x,int y,int l,char *p,int key);        //  *+ s_tab()      Запомнить таблицу или рамку
int  help (int x,int y,char **list,char **list2,int key);
int  com (register char *p);
void fpm (char *u,char *l,char *d);
void s_pol (int x,int y,int l,char *p);                // *+ s_pol()       Запомнить поля
void mu_set(int key);                                  //  *+ mu_set()     Извлеч и сохранить внешние переменные
struct maska *choise (struct maska *m);                         //  *+ choise ()    Выбор в меню
int  dosystem (char *s,int key);                       //  *+ dosystem ()  Выполнить команду системы.
void e_item (register struct pol *k);                  //  *+ e_item ()    Выделение строк
void l_item ();                                        //  *+ l_item ()    Гашение строки
void drawline (struct maska *ms);                      // *+ drawline ()   Рисовать строку
int  execute (register struct pol *pol);               //   *+ execute ()  Выполнение строк меню ( старое )
int  in_esc (int c);                                   //  *+ in_esc()     Выбрать и выполнить команду
void delmenu (struct maska   *m,int key);              //  *+ delmenu ()   Тереть меню
int red (int x,int y,char *s,int mx,int pos);
int fUTF8( char *a );                                  //                  Проверка на Russian UTF8 символ
void display ();                                       //  *+ display()    Рисовать все меню
int tUTF8(char *cline ,int thiscol);                   //                  Сколько Russian UTF8 символов в строке до позиции thiscol  на экране
int in_menu ();                                        //  *+ in_menu ()   Встроенное меню
void free_mas (struct maska   *m);                     //  *+ free_mas()   Освободить память из под меню
struct maska *grep (char *menu,int key);               //  *+ grep()       Поиск меню по дереву или возврат адреса пустой ячейки.
int readmenu (char *name,int key);                     //  *+ readmenu ()  Считать меню из файла
int stir (char ss[],int j);                            //  *+ stir()       Упаковать символы в целое
void par (char *p);                                    //  *+ par()        Разбор строки описания поля
void vc(char *p,int key,int val);
int blklen (register char **av);
char **blkcpy (char **oav,register char ** bv);
char **blkend (register char **up);
char **blkcat (char **up,char **vp);
char **blkspl (register char **up,register char **vp);
char *strspl (char *cp,char *dp);
void setenvMy (char *b_name,char *b_val);
