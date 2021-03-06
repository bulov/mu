#include "mu.h"
#include <ctype.h>
#define N_HELP (15 * sizeof (char*))
static int      Com;			/* Обработка командочек */

char*
ssetenv(char *p)
{
   char *nam, *val, *stp = p,  *s;
   char    buf[L_SIZ];

   for (; *p == ' ' && *p; p++);      /* Лидирующие пробелы */
   if(!strncmp(p,"setenv",6)){
       s = strcpy(buf,p);
       if(nam=index(s,' ')){
	   if(val=index(++nam,' ')){
	       *val = 0;
	       if(stp=index(++val,';')){
		   *stp++ = 0;
		   stp = p + (val + strlen(val) - buf +1);
		   setenv (nam, val, 1);
		   Draw=ON;            /* Проверять environ */
		   return(ssetenv(stp));
	       }else{
		   stp = p + (val + strlen(val) - buf);
	       }
	   }
       }
   }
   return(stp);
}
int readmenu (char *name,int key){         //  *+ readmenu ()  Считать меню из файла
	register        i, j;
	char           *ss, *from, *strcpy ();
	char            b0[L_SIZ], b1[L_SIZ], b2[L_SIZ], b3[L_SIZ];
	char           *p = b0, *up = b1, *du = b2, *lo = b3;
	FILE           *fp;
	char          **pt, **pd;
	char           *file;
	char           *menu;

	if (key){
		file = File;
		strcpy (menu = File = (char*)malloc (strlen (name)+1), name);
	}
	if ((fp = fopen (File, "r")) == NULL) {
		if ( (ss = getenv("MENU")) != 0 ){
		   strcat (strcpy (b0, ss), File);
		   if ((fp = fopen (b0, "r")) == NULL) {
			   FREE (File);
			   if(file != NULL)
				   File = file;
			   return (ON);
		   }
		}else{
		   return (ON);
		}
		if (key) {
			strcpy (File = (char*)malloc (strlen (b0+1)), b0);
		}
	}
	fgets (b0, L_SIZ, fp);
	if(*b0 != '{' ){
		grep (File, ON);
		Maska->dir |= OLD;
		Str = -1;
	}
	goto old;
	while (fgets (b0, L_SIZ, fp) != NULL) {
old:            i = strlen (p = b0);
		if (*(p + i - 1) == '\n')
			*(p + i-- - 1) = '\0';
		switch (*p) {
		    case 0:
			continue;
		    case '{':		/* Начало описания маски */
			if ((ss = index (++p, ':')) == 0)
				ss = menu - 1;  /* Имя */
			grep (++ss, ON);
			switch (*p) {
			    case 'O':	/* OLD - Старое меню */
				Maska->dir |= OLD;
				Str = -1;
				continue;
			    case 'T':   /* TAB - Таблица */
				Maska->dir |= TAB;
			    case 'M':	/* MSK - Маска меню */
				Maska->dir |= MSK;
				Str = -2;
				bzero (du, L_SIZ);
				continue;
			    case 'H':	/* HLP - Help меню */
				Maska->dir |= HLP;
				Maska->pol = (struct pol *) malloc (SIZ (pol));
				pt = (char **) (Maska->pol->t = (char*)malloc (N_HELP));
				bzero (pt, N_HELP);
				pd = (char **) (Maska->pol->d = (char*)malloc (N_HELP));
				bzero (pd, N_HELP);
				Maska->x = 0;
				Maska->y = Ydim - 1;
				Str = 0;
				continue;
			}
			continue;
		    case ':':		/* Описание полей */
			Com = 0;
			par (++p);
			continue;
		    case '}':		/* Конец описания маски */
			if (Maska->dir & MSK)
				break;
			continue;
		    case '@':		/* x,y */
			sscanf (p + 1, "%d%d", &Maska->x, &Maska->y);
			Maska->x = Maska->x > Win->_maxx ? Win->_maxx -1: Maska->x;
			Maska->y = Maska->y > Win->_maxy ? Win->_maxy -1: Maska->y;
			continue;
		    case '#':		/* Комментарий или строка в подсказку */
			if (Maska->dir & OLD) {
				i = i > Xdim - Maska->x ? Xdim - Maska->x : i;
				p[i + 1] = '\0';
				s_tab (0, ++Str, i, p + 1, ON);
			}
			continue;
		    case '!':		/* команды, выполняемые при входе в меню */
			Maska->make = (char*)malloc (i);
			strcpy (Maska->make, p + 1);
			continue;
		    case 's':           /* setenv */
		       if( (p = ssetenv(p) ) )
			   continue;
		}
		if (Maska->dir & MSK) {
			for (from = b0 - 1, ss = up - 1; *++ss = *++from;) {
				if (*from == '\t') {	/* Замена табуляций */
					--ss;
					for (j = 8; j--;)
						*++ss = ' ';
				}
			}
			bzero (ss, L_SIZ - (ss - up));
			p = up;
			up = lo;
			lo = du;
			du = p;
			p = lo;
			fpm (up, lo, du);
		} else if (Maska->dir & OLD) {
			if (++Str < Ydim)
				s_pol (Maska->x == 0 ? 20 : 0, Str, i, p, ON);
		} else if (Maska->dir & HLP) {
			ss = index (p, ':');
			*ss++ = '\0';
			*pt++ = strcpy (malloc (strlen (p) + 1), p);
			*pd++ = strcpy (malloc (strlen (ss) + 1), ss);
		}
	}
	fclose (fp);
	if(file != NULL)
		File = file;
	return(0);
}
/*
*+ stir()        Упаковать символы в целое
*/
stir (ss, j)
	char            ss[];

{
	register        i;
	union ic {
		int             i;
		char            c[4];
	}               ic;

	j = j > 4 ? 4 : j;
	for (ic.i = i = 0; i < j; i++)
		ic.c[i] = ss[j - i - 1];
	return (ic.i);
}
/*
*+ par()        Разбор строки описания поля
*/
par (p)
	char           *p;
{
	int             val, k_parse = 0;
	char           *sz, *ss;

	for (sz = ss = p; *p != ':' && *p; p++) {	/* :nd:help=file */
		if (*p == '=') {
			*p++ = '\0';
			sz = p;
		}
	}
	if (*p == ':') {
		k_parse++;
		*p = '\0';
	}
	if (isdigit (*ss)) {
		Com++;
		Pol = fin (atoi (ss));
	} else {
		val = ss != sz ? p - sz : 0;
		switch (stir (ss, ss != sz ? sz - ss - 1 : p - ss)) {
		    case 'make':        /* Выполнить следущую маску */
			if (Com) {      /* Запуск задач в поле   */
				MALLOC (Pol->d, sz, val);
			} else {        /* Запуск задач в маске */
				MALLOC (Maska->prl, sz, val);
			}
			break;
		    case 'task':
			if (Com) {      /* Запуск задач в поле   */
				MALLOC (Pol->d, sz, val);
				Pol->key |= TASK;
			} else {        /* Запуск задач в маске */
				MALLOC (Maska->prl, sz, val);
			}
			break;
		    case 'chld':        /* Поискать сынишку */
			Maska->dir |= CHLD;
			break;
		    case 'run':        /* Запуск по входу в маску */
			Maska->dir |= RUN;
			break;
		    case 'help':        /* Строка подсказки */
			Maska->help = grep (sz, OFF);
			break;
		    case 'n_ex':	/* Работа в парралель */
			Maska->dir &= ~EXIT;
			break;
		    case 'full':        /* Дополнять поля */
			Maska->dir |= FULL;
			break;
		    case 'date':        /* подставить время дату */
			Pol->key |= DATE;
			Pol->d = strcpy (malloc (strlen (sz) + 1), sz);;
			break;
		    case 'time':
			Pol->key |= TIM;
			Pol->d = strcpy (malloc (strlen (sz) + 1), sz);;
			break;
		    case 'env':
			Pol->key |= ENV;
			Pol->d = strcpy (malloc (strlen (sz) + 1), sz);;
			break;
		    case 'ntab':
			vc(sz, OFF, PTAB);
			break;
		    case 'tab':
			vc(sz,  ON, PTAB);
			break;
		    case 'char':        /* Символьное поле */
			vc(sz,  ON, CHAR);
			break;
		    case 'num':         /* числовое поле */
			vc(sz, OFF, CHAR);
			break;
		    case 'left':        /* Выравнивать по левому краю */
			vc(sz,  ON, LEFT);
			break;
		    case 'rigt':        /* Выравнивать по правому краю */
			vc(sz, OFF, LEFT);
			break;
		    case 'ndsp':        /* Не корректировать поле */
			vc(sz, OFF,  DSP);
			break;
		    case 'div':	/* Символ разделитель полей */
			switch (stir (sz, val)) {
			    case 'ndiv':
				Maska->divide = '\0';
				break;
			    case '\\n':
				Maska->divide = '\n';
				break;
			    case '\\t':
				Maska->divide = '\t';
				break;
			    default:
				Maska->divide = *sz;
			}
		}
	}
	if (k_parse)
		par (++p);
}
void vc(char *p,int key,int val){
	int     nn = 0, ns = 0;

	if (!strncmp (p,"all",3)){
		if(key == OFF){
			clr_v (Pol, val);
		}else{
			set_v (Pol, val);
		}
		return;
	}
	if(!isdigit (*p)){
		if(key == OFF){
			Pol->key &= ~val;
		}else{
			Pol->key |=  val;
		}
		return;
	}else{
		while(isdigit (*p))
			nn = nn * 10 + *p++ - '0';
		if(*p == '-'){
			p++;
			while(isdigit (*p))
				ns = ns * 10 + *p++ - '0';
		}
		if(nn > ns )
			ns = nn;
		Pol = fin (nn);
		while(nn <= ns ){
			if(key == OFF){
				Pol->key &= ~val;
			}else{
				Pol->key |=  val;
			}
			Pol = Pol->next;
			nn++;
		}
	}
}
