#include   "sys/errno.h"
#include <curses.h>
#include <setjmp.h>			/* Обработка ошибочных явлений */
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
	char            x, y, l;	/* Координаты поля и его длинна */
	char           *t;		/* Куда писать текст */
	struct poz     *next;		/* Следущее поле */
};
struct pol {
	char            x, y, l;	/* Координаты поля и его длинна */
	char           *t;		/* Куда писать текст */
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

extern WINDOW  *Win;
