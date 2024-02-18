#include "mu.h"
#include "tty.h"
void s_pol (int x,int y,int l,char *p){      // *+ s_pol()      Запомнить поля
	register struct pol *n;
	register char  *s;
	static struct pol *save;
	static short    nom;
	static char   utf8[] ="йцукенгшщзхъфывапролджэячсмитьбюЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭ/ЯЧСМИТЬБЮ.ёЁ";
	static char   ascii[]="qwertyuiop[]asdfghjkl;'zxcvbnm,.QWERTYUIOP[]ASDFGHJKL;'<ZXCVBNM,./`~";

	n = (struct pol *) malloc (SIZ(pol));
	if (Maska->pol == NULL) {
		Pol = Maska->pol = save = n;
		nom = 0;
		Maska->cur = n;
	}
	n->x = x;
	n->y = y;
	n->l = l;
	if(Maska->dir & TAB)
		n->key = DSP | LEFT | PTAB;
	else
		n->key = DSP | LEFT;
	n->nom = nom++;
	if (Maska->dir & OLD) {
		GREP(s,p,':');
		if (*s == ':') {
			/* найти точку и взять уровень */
			if( *(s -1)  == '.')
			   n->key |= SYS1; /* vsi */
			l = n->l - (s - p) - 1;
			CALLOC(n->d,++s,l);
			if (*n->d == '<')
			       n->key |= MENU;
			n->l = (s - p) - 1;
			CALLOC(n->t, p, n->l);
		} else {
			CALLOC (n->t, p, n->l);
			n->d = NULL;
		}
		if (fUTF8(n->t)){
		   n->h=0;
		   for(s = utf8 ; *s != 0; s++  ){
		      if( *(s) == *(n->t+1) ){
			  n->h = ascii[(s-utf8)/2];
			  break;
		      }
		   }
	       }else{
		   n->h = *n->t;
	       }
	} else {
		n->t = (char*)malloc (n->l+1);
		*n->t = '\0';
		n->d = NULL;
	}
//       if (n->d && *n->d == '$' && index(n->t,'[') != 0) {  /* Подстановка из environ */
       if ( index(n->t,'[') != 0) {  /* Подстановка из environ */
	   n->key |= ENV;
       }
	n->next = save->next;
	save->next = n;
	n->back = save;
	save = n->next->back = n;
	l = l - tUTF8(p ,l) + 1;
	Maska->xW = l+Maska->x > Maska->xW ? l+Maska->x : Maska->xW ;
	Maska->yW = y+Maska->y > Maska->yW ? y+Maska->y : Maska->yW ;
}
void s_tab (int x,int y,int l,char *p,int key){     //  *+ s_tab()      Запомнить таблицу или рамку
	register struct poz *n;  /**/
	static struct poz *save_tab, *save_ram;

	n = (struct poz *) malloc (SIZ(poz));
	bzero(n,sizeof (struct poz));
	if (key) {
		if (Maska->tab == NULL)
			Maska->tab = save_tab = n;
		save_tab = save_tab->next = n;
	} else {
		if (Maska->ram == NULL)
			Maska->ram = save_ram = n;
		save_ram = save_ram->next = n;
	}
	n->x = x;
	n->y = y;
	n->l = l;
	CALLOC (n->t, p, n->l);
	n->next = NULL;
	l = l - tUTF8(p ,l) +1 ;
	Maska->xW = l+Maska->x > Maska->xW ? l+Maska->x : Maska->xW ;
	Maska->yW = y+Maska->y > Maska->yW ? y+Maska->y : Maska->yW ;
}
struct maska *grep (char *menu,int key){        // *+ grep()       Поиск меню по дереву или возврат адреса пустой ячейки.
	register struct maska *l;
	register char  *s, *file=menu;
	static   char  save[L_SIZ];

	GREP(s,menu,' ');       /* <file menu */
	if (*s == ' ') {
		*s++ = '\0';
		menu = s;
	}
	for (l = Head; l != NULL; l = l->next){
		if (!strcmp (menu, l->menu)) {
			if (key == ON){
				free_mas (l);
			}
			return (l);
		}
	}
	if (!key) {
		if (readmenu (file, ON))
			return (NULL);
		if( strcmp(menu,save) == 0 ) {
		   err("Внутренний цикл");
		   return(NULL);
		}
		strcpy(save,menu);
		return (grep (menu, OFF));
	}
	l = (struct maska *) malloc (SIZ(maska));
	bzero(l,SIZ(maska));
	l->file = File;
	l->dir |= EXIT;
	l->divide = ' ';
	MALLOC(l->menu, menu, strlen (menu));
	l->next = Head;
	return (Maska = Head = l);
}
void free_mas (struct maska   *m){      //  *+ free_mas()   Освободить память из под меню
	register struct poz *p, *ps;
	register struct pol *l, *ls;

	if(m->task)
		clear_tab(&m->task,OFF);
	for (p = m->tab; p != NULL; p = ps) {	/* Постоянная часть */
		ps = p->next;
		FREE (p->t);
		FREE (p);
	}
	for (p = m->ram; p != NULL; p = ps) {	/* Рамка */
		ps = p->next;
		FREE (p->t);
		FREE (p);
	}
	if (m->dir & HLP) {
		char          **p;

		for (p = (char **) m->pol->t; *p; p++)
			FREE (*p);
		for (p = (char **) m->pol->d; *p; p++)
			FREE (*p);
		FREE (m->pol->t);
		FREE (m->pol->d);
	} else {
		l = m->pol;             /* Поля ввода */
		do {
			ls = l->next;
			FREE (l->t);
			if ( NULL != l->e )
			   FREE (l->e);
			FREE (l->d);
			FREE (l);
			l = ls;
		} while (l != m->pol);
	}
	FREE (m->make);
	m->make = NULL;
	m->x = m->y = 0;
	m->tab = m->ram = NULL;
	m->pol = NULL;
	Maska = m;
}
void drawmenu (struct maska   *m, int key_env){  // *+ drawmenu()   Рисовать меню
	register char  *c;
	struct pol     *l;
	struct poz     *p;

	m->dir |= DISPLAY;              /* Маска отображается на дисплее */
	if (m->dir & OLD) {
		attron (A_REVERSE);
		dpc (BLUE);
	}
	for (p = m->tab; p != NULL; p = p->next) {	/* Постоянная часть */
		dpp (m->x + p->x, m->y + p->y);
		dps (p->t);
		if (m->dir & OLD){
//                   dpn ( m->xW - p->l - (m->x + p->x) + tUTF8(p->t, p->l) -1 , ' ');
		   dpn ( Xdim - p->l - (m->x + p->x) + tUTF8(p->t, p->l) -1 , ' ');
		}
	}
	if (m->dir & OLD) {
		attroff(A_REVERSE);
		dpc (BLUE);
	}
	draw_pol(m,key_env);
	if( m->ram != NULL ){
	       if (m->dir & OLD)
		       dpc (BLACK);
	       attron (A_REVERSE);
	       attron (A_ALTCHARSET);
	       if(Pc)
		       attron ( A_UNDERLINE );
	       for (p = m->ram; p != NULL; p = p->next) {      /* Рамка */
		       dpp (m->x + p->x, m->y + p->y);
		       c = p->t;
		       while (*c)
			       dpo (*c++);
	       }
	       if(Pc)
		       attroff ( A_UNDERLINE );
	       attroff (A_ALTCHARSET);
	       attroff (A_REVERSE);
	}
	if (m->help) {
		register struct maska *o = m->help;

		help (o->x, o->y, &o->pol->t, &o->pol->d, OFF);
	}
}
void draw_pol (struct maska   *m, int key_env){ //  *+ draw_pol()   Рисовать pole
	register char  *c;
	struct pol     *l;
	struct poz     *p;
	int            nUTF8,i;

	l = m->pol;			/* Поля ввода */
	if (m->dir & MSK) {
		attron ( A_UNDERLINE );
	}
       do {
	   if (l->key & ENV && key_env) {
	       char           *sn, *p, *getenv ();
	       if (l->d && *l->d == '$' ) {  /* Подстановка из environ */
		   sn = getenv (l->d + 1);
		   sn = sn ? sn : "";
		   if (m->dir & OLD ) {
		       GREP(p,l->t,'[');       /* текст[   ]:$ENV */
		       if ( NULL != p ){
			   if ( NULL != l->e)  FREE (l->e);
			   l->e = strcpy (malloc (strlen (sn) + 1), sn);
//                           for (*p++ = '['; *p != ']'; p++){
//                                   *p = ' ';
//                           }
		       }
		   }else{
		       strncpy(l->t,sn,l->l);
		       *(l->t + l->l) = '\0';
		   }
	       }else{
		   err("пустое поле ENV ждем 'текст[   ]:$ENV' пришло '%s'", l->t );
	       }
	   }
//           if (0x0 != l && ( l->key & DATE || l->key & TIM)  ) {  /* подставить время дату */
	   if (( l->key & DATE || l->key & TIM)  ) {  /* подставить время дату */
	       char mas[128];
	       tab_date(mas,l->d,1);
	       strncpy(l->t,mas,l->l);
	   }
/*           if( l->key & MENU){
/*               dpp (m->x + l->x -3, m->y + l->y);
/*               dps("<  ");
/*           }else{
*/
	       dpp (m->x + l->x , m->y + l->y );
/*           }
*/           m->cur = l;
	   drawline (m);
	   l = l->next;
	} while (l != m->pol && 0x0 != l );
	if (m->dir & MSK) {
		attroff ( A_UNDERLINE );
	}
}
void display (){       //  *+ display()    Рисовать все меню
	register struct maska *m;

	if (Rew == OFF) {
		Rew = ON;
		return;
	}
	for (m = Head; m != NULL; m = m->next){
		if (m->dir & DISPLAY){
			drawmenu (m, Draw);
		}
	}
	Draw = OFF;
}
void delmenu (struct maska   *m,int key){   //  *+ delmenu ()   Тереть меню
	register char  *c;
	struct pol     *l;
	struct poz     *p;

	if (Del == OFF) {
		Del = ON;
		return;
	}
	m->dir &= ~DISPLAY;             /* Маска не отображается на дисплее */
	if(key){
		dpo (_CL);
		return;
	}else{
		for(p = m->tab; p != NULL; p = p->next){
			dpp(m->x+p->x,m->y+p->y);
			c = p->t;
			while(*c++)
				dpo(' ');
			if(m->dir & OLD)
				dpn(Xdim - p->l - (m->x + p->x) +1, ' ');
		}
		for( p = m->ram; p != NULL; p = p->next){
			dpp(m->x+p->x,m->y+p->y);
			c = p->t;
			while(*c++)
				dpo(' ');
		}
		l = m->pol;		/* Поля ввода */
		do {
			dpp(m->x + l->x, m->y + l->y);
			c = l->t;
			while(*c++)
				dpo(' ');
			l = l->next;
		} while (l != m->pol);
	}
}
/*
*+ file()       Записать в файл структуры tab ram и pol
*/
/*
file (){
{
	struct maska   *m;
	struct poz     *p;
	struct pol     *l;
	FILE           *fd;
	char            file[256], *o, *s;

	for (m = Head; m != NULL; m = m->next) {
		if (m->file != o) {
			if (fd)
				fclose (fd);
			strcpy (file, m->file);
			if (s = index (file, '.'))
				strcat (s, ".v");
			else
				strcat (file, ".v");
			if ((fd = fopen (m->file, "w")) == NULL) {
				DPIM ("Не могу открыть ");
				dps (m->file);
				dpi ();
				ceol (0, dpd ()->ydim - 1);
				return;
			}
		}
		fprintf (fd, "static struct poz %st[] = {\n", m->menu);
		for (p = m->tab; p != NULL; p = p->next)
			fprintf (fd, "%2.0d,%2.0d,%2.0d,%s\n", p->x, p->y, p->l, p->t);
		fprintf (fd, " 0, 0, 0, NULL\n};\n");
		fprintf (fd, "static struct poz %sr[] = {\n", m->menu);
		for (p = m->ram; p != NULL; p = p->next)
			fprintf (fd, "%2.0d,%2.0d,%2.0d,%s\n", p->x, p->y, p->l, p->t);
		fprintf (fd, " 0, 0, 0, NULL\n};\n");
		fprintf (fd, "static struct pol %sl[] = {\n", m->menu);
		l = m->pol;
		do {
			fprintf (fd, "%2.0d,%2.0d,%2.0d,%s\n", l->x, l->y, l->l, l->t);
			l = l->next;
		} while (l != m->pol);
		fprintf (fd, " 0, 0, 0, NULL\n};\n");
		o = m->file;
	}
}
*/
struct pol *fin(int i){    //  *+ fin()        По номеру найти указатель на поле
	register struct pol *l = Maska->pol;
	register int    k = 0;

	do {
		if( ++k == i )          /* Порядковый номер поля */
			break;
		l = l->next;
	} while (l != Maska->pol);
	return(l);
}
void set_v(register struct pol *l, int i){  //  *+ set_v()      Установить ключ для всех значений
	do {
		l->key |= i;
		l = l->next;
	} while (l != Maska->pol);
}
void clr_v(register struct pol *l, int i){  //  *+ clr_v()      Очистить ключ для всех значений
	do {
		l->key &= ~i;
		l = l->next;
	} while (l != Maska->pol);
}
void clear_pol(struct maska   *m){ //  *+ clear()    Очистить все поля.
	struct pol     *l;

	l = m->pol;                     /* Поля ввода */
	do {
		*l->t = '\0';
		l = l->next;
	} while (l != m->pol);
}
