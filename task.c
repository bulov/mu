/*
 *  Программа связи задач mu <=> task
 *
 *  $Log: task.c,v $
 *  Revision 1.1.1.1  2006/04/12 06:02:42  bvg
 *  import
 *
 * Revision 1.2  91/10/31  20:33:53  root
 * *** empty log message ***
 * 
 * Revision 1.1  91/06/15  12:28:08  root
 * Initial revision
 * 
 *
 */
#define NOFILES_MIN        64
#include <sys/param.h>			/* Параметры системы */
#include <sys/types.h>			/* Новые типы переменых */
#include <sys/file.h>                   /* Работа с файлами */
#include <sys/time.h>			/* select ()+Структуры времени */
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include "tty.h"
#include <sys/signal.h>                     /* Сигналы обмена */
#include "rob.h"                        /* Эмуляция robotron */
#include "mu.h"                         /* Местные штучки */
#define CLOSE(F)        if (F){ close (F);FD_CLR(F, &Rd);}
#define PIPE(C,F,N)     {pipe (C);F=C[N];fcntl(F,F_SETFD,1);}
static char    *F_w, *F_r;
static int      F_v;
static fd_set   Rd;
extern int errno;
/*
*+ clck_int()   Timer interrupt handler
*/
clck_int ()
{
	signal (SIGALRM, clck_int);	/* Setup the timeout */
	longjmp (E_clck, 1);		/* Tell rpack to give up */
}
/*
*+ chld_int()   Zombie state eksel-moksel
*/
chld_int ()
{
	int             pid, code;
	register struct task *ts;

	signal (SIGCLD, chld_int);
	pid = wait (&code);
	for (ts = H_task; ts != NULL; ts = ts->next) {
		if (ts->pid == pid) {
			ts->pid = -pid;	/* Нужно чистить тавлицу */
			/* read_task(ON);
			/*
			/*if(ts->err = code) /* Код завершения != 0 ошибка */
			/*        err ("Завершение task %s err = %d", ts->name, ts->err >> 8);
			/**/
			break;
		}
	}
}
/*
*+ pipe_int()   Pipe interrupt handler
*/
pipe_int ()
{
	signal (SIGPIPE, pipe_int);	/* Setup the pipe interupt */
	/* err ("Завершение ");         /**/
	longjmp (E_pipe, 1);		/* Tell rpack to give up */
}
/*
*+ word()       Разбор строки на запуск задачи и подстановка environ
*/
static char *
word (p)
	char          **p;
{
	char           *c;

	for (; **p == ' ' && **p; (*p)++);	/* Лидирующие пробелы */
	c = *p;
	for (; **p != ' ' && **p; (*p)++);	/* Само слово */
	if (**p != '\0') {
		**p = '\0';
		(*p)++;
	}
	return (c);
}
/*
*+ parse()      Разбор строки на запуск задачи
*/
void
parse (p, argv, argc)
	char           *p, *argv[];
	int            *argc;
{
	char           *c, buf[L_SIZ];

	F_r = NULL;
	F_v = 0;
	F_w = NULL;
	while (*p) {
		switch (*(c = word (&p))) {
		    case '<':		/* Перенаправление ввода */
			F_r = word (&p);
			break;
		    case '>':		/* Перенаправление вывода */
			F_v = O_WRONLY | O_CREAT | O_TRUNC;
			if (*(++c) == '>')	/* Дозапись */
				F_v = O_WRONLY | O_CREAT | O_APPEND;
			F_w = word (&p);
			break;
		    case '$':           /* Подстановка environ */
			    c = strcpy(buf,getenv (++c));
			    parse(c, argv, argc );
			break;
		    default:
			argv[(*argc)++] = c;
		}
	}
	argv[*argc] = 0;
}
/*
*+ send_task()  Запустить или послать сообщение транзакции по имени
*/
send_task (key, comm)
{
	struct task    *ts;
	int             i, argc = 0;
	int             par[2], chl[2], ero[2];	/* pipes to parent, child */
	char            buf[L_SIZ], *argv[100];
	struct maska   *m = Maska;

		err("Действий нет");
		return(0);

	return(0);   /*bvg!!!*/

	if(!m->prl[0]){
		err("Действий нет");
		return(0);
	}
	if (!(m->task)) {
		strcpy (buf, m->prl);
		parse (buf, argv, &argc);
		if(m->dir & CHLD ){
			for (ts = H_task; ts != NULL; ts = ts->next) {
				if (strcmp (ts->name, argv[0]) == 0){   /* Работает */
					m->task = ts;
					ts->count++;
					break;
				}
			}
		}
	}
	if (ts = m->task) {
	       if (setjmp (E_pipe)) {  /* Если завершилась */
		       CLOSE (ts->dfw);
		       CLOSE (ts->dfr);
		       CLOSE (ts->dfe);
		       goto cont;
	       }
	       if (key)
		       return(write_task (comm)); /* Передать пакет старой транзакции */
	       return(0);
	}
	ts = (struct task *) malloc (SIZ (task));
	bzero (ts, SIZ (task));
	ts->next = H_task;
	Maska->task = H_task = ts;
	MALLOC (ts->name, argv[0], strlen (argv[0])); /* Имя транзакции */
cont:   if (!F_r)                       /* Закрывать по exit */
		PIPE (chl, ts->dfw, 1);
	if (!F_w){
		PIPE (par, ts->dfr, 0);
		FD_SET(ts->dfr, &Rd);         /* for select */
	}
	PIPE (ero, ts->dfe, 0);
	FD_SET(ts->dfe, &Rd);                 /* for select */
	if ((ts->pid = fork ()) == 0)        /* Раздвояемся */
	{
		static char    *mes = "Не могу открыть файл %s";
		char            mas[80];

		close (2);
		dup(ero[1]);
		close (0);		/* child process executes here */
		if (F_r) {
			if (open (F_r, O_RDONLY) < 0) {
				sprintf (buf, mes, F_r);
				write(2,buf,strlen(buf));
				_exit (-1);
			}
		} else {
			dup (chl[0]);	/* dup pipe read to standart input */
		}
		close (1);
		if (F_w) {
			if (open (F_w, F_v, 0644) < 0) {
				sprintf (buf, mes, F_w);
				write(2,buf,strlen(buf));
				_exit (-1);
			}
		} else {
			dup (par[1]);	/* pipe на запись */
		}
		if (execvp (ts->name, argv) == -1){
				sprintf (buf,"Ошибка запуска %s errno=%d", ts->name,errno);
				write(2,buf,strlen(buf));
				_exit(-1);
		}
	}				/* parent process executes here */
	ts->count++;
	if (!F_r)
		close (chl[0]);
	if (!F_w)
		close (par[1]);
	close (ero[1]);
	if (key)
		return(write_task (comm));         /* Посылаем запись */
	return(0);
}
/*
*+ clear_tab()  Очистить вход в таблице
*/
clear_tab (tts,key)
	struct task   **tts;
{
	struct task    *tt, *ts;
	register struct maska *m;

	ts = *tts;
	if(key){
		for (m = Head; m != NULL; m = m->next){
			if(m->task == ts)
				m->task = NULL;
		}
		ts->count = 0;
	}
	if( --ts->count > 0){
		*tts = NULL;
		return;
	}
	*tts = NULL;
	FREE (ts->name);
	CLOSE (ts->dfw);
	CLOSE (ts->dfr);
	CLOSE (ts->dfe);
	if( ts == H_task)
		H_task = ts->next;
	else
		for (tt = H_task; tt != NULL; tt = tt->next) {
			if(tt->next == ts ){
				tt->next = ts->next;
				break;
			}
		}
/*      if(ts->pid > 0 )                 /**/
/*              kill(ts->pid, SIGTERM);  /**/
	FREE (ts);
	bzero (ts, SIZ (task));  /**/
}
/*
*+ write_task() Послать сообщение транзакции
*/
write_task (comm)
{

	struct task    *task;
	struct packet   pk;

	if ( (task = Maska->task) == 0 )
		return(0);
	if (setjmp (E_pipe)) {  /* Если завершилась */
		CLOSE (task->dfw);
		CLOSE (task->dfr);
		CLOSE (task->dfe);
		return(1);
	}
	bzero(&pk,HEAD);
	strncpy(pk.text,Maska->menu,N_L);   /* Имя структуры */
	pk.lenth = task->lenth = N_L + collect (&pk.text[N_L]); /* Длина отправлямой записи */
	pk.comm = comm;                 /* Клавиша F6-F20 */
	pk.keep = Maska->cur->nom;          /* Номер текущего поля */
	write (task->dfw, &pk, pk.lenth + HEAD);  /* Саму запись */
	if(Fp)
		write (Fp, &pk, pk.lenth + HEAD);
	time (&task->time);		/* Время последнего обращения */
	return(1);
}
/*
*+ read_task()  Получить сообщение от транзакции
*/
#include <sys/socket.h>
read_task (key,flag)
{
	struct task    *ts;
	struct packet   pk;
	fd_set          fdset;
	int             i, ndfs, count, roll = 0;
	char            buf[2048];
	struct timeval  tim;

	if(!(ts = Maska->task) || ts->pid == 0)
		return(0);
	FD_ZERO(&fdset);
	if(ts->dfe)
		FD_SET(ts->dfe, &fdset);         /* for select */
	if(ts->dfr)
		FD_SET(ts->dfr, &fdset);         /* for select */
	tim.tv_usec = 0;
	if(key == ON)
		tim.tv_sec  = 0;
	else
		tim.tv_sec  = 25;
/*        if(!(ndfs = select (NOFILE, &fdset, 0, 0, &tim))) /** bvg for SCO */
	if(!(ndfs = select (NOFILES_MIN, &fdset, 0, 0, &tim)))
		return(0);
	if (ts->dfr && FD_ISSET (ts->dfr, &fdset)){
		i = 50;
		do {
			if(roll != N_ROLL ){ /* Читать Блок Сообщений */
				ioctl (ts->dfr, FIONREAD, &count);
				if(count < HEAD )
					break;
			}
			read (ts->dfr, &pk, HEAD);
			if(pk.keep != (short) 'mu'){
				i = read (ts->dfr, pk.text, count-HEAD);
				if(pk.text[count-HEAD-1] == '\n')
					pk.text[count-HEAD-1] = '\0';
				else
					pk.text[count-HEAD] = '\0';
				err (&pk);
				if(Fp){
					write (Fp, &pk, count);
					write (Fp, "\n",1);
				 }
			}else{
				if(Fp)
					write (Fp, &pk, HEAD);
				if (pk.lenth > 0){
					read (ts->dfr, pk.text, pk.lenth);
					if(Fp){
						write (Fp, pk.text, pk.lenth);
						write (Fp, "\n",1);
					 }
				}
				if(flag){ /* сброс если не все прочитали */
					if(pk.comm != Y_ROLL ){
						i++;
						roll = N_ROLL;
						continue;
					}else
						goto pid_test;
				}
				if((roll = command (ts, &pk)) == N_ROLL)
					continue;
				if( Maska->dir & DISPLAY){
					if( Disp ){
						drawmenu (Maska,OFF);
						Disp = 0;
					}else{
						draw_pol (Maska,OFF);
					}
				}
				if( roll == Y_ROLL)
					goto pid_test;
			}
		} while (ts->pid < 0 || i-- );
	}
	if (ts->dfe && FD_ISSET (ts->dfe, &fdset)) {
		ioctl (ts->dfe, FIONREAD, &count);
		if(count){
			i = read (ts->dfe, &buf[1], count);
			buf[0] = '>';
			buf[i + 1] = '<';
			buf[i + 2] = '\0';
			err (buf);
		}
	}
pid_test:
	if (ts->pid < 0){       /* Завершилась удачно */
		if((i = ts->err >> 8))
			err ("Завершение task %s err = %d", ts->name, i);
		clear_tab (&Maska->task,ON); /* Чистим таблицу */
		return (0);
	}
	return (1);

}
