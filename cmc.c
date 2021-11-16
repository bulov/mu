/*
 *  П/Программы для связи задач mu <=>  task
 *
 */
#include "mu.h"                         /* Местные штучки */
#include "tty.h"
extern int errno;
int collect(char *b){  //  *+ collect()    Собрать в буфер поля отправки
	struct pol     *l;
	char           *c, *bs, *bn, *bb=b;

	l = Maska->pol;                     /* Поля ввода */
	if(Maska->divide)
		*b++ = Maska->divide;
	do {
		c = l->t;
		if (l->key & TIM  ) {  /* подставить время дату */
			char mas[128];
			tab_date(mas,l->d,1);
			strncpy(l->t,mas,l->l);
		}
		if((l->key & DSP) && !(l->key & PTAB)){
			if(l->key & CHAR){
				bs = b;
				bn = b + l->l;
				while( *c && *c == ' ')
					c++;
				while( *c )
					*b++ = *c++;
				b--;
				while( *b == ' ' && b > bs)
					b--;
				b++;
				while( bn > b)
					*b++ = 0xff;
			}else{
				if(!Maska->divide){
					bs = b + l->l -1;
					while( *c )     /* в конец */
						c++;
					while( ( !*c || *c == ' ') && c != l->t)
						c--;
					while( *c && *c != ' ' && c >= l->t)
						*bs-- = *c--;
					while( bs >= b)
						*bs-- = '0';
					b += l->l;
				}else{
					bs = b;
					while( *c && *c == ' ')
						c++;
					while( *c && *c != ' ')
						*b++ = *c++;
					if( bs == b)
					       *b++ = '0';
				}
			}
			if(Maska->divide)
				*b++ = Maska->divide;
		}
		l = l->next;
	} while (l != Maska->pol);
	*b++ = '\n';
	*b = '\0';
	return(b - bb);       /* Длина отправлямой записи */
}
void miracle(unsigned char *s){ //  *+ miracle()    Записать полученную запись в роля
	char           *c,*bs;
	struct pol     *p;
	int             key_blan, n, nn, nd;

	p = Maska->pol;     /* Поля ввода */
	do {
		n = p->l;
		key_blan = 0;
		if(p->key & CHAR){
			if(Maska->divide && *s == Maska->divide )
				s++;
			c = p->t;
			*c =  '\0';
			if(!(p->key & LEFT))
				key_blan++;
			while( *s && n--){   /* СТРОКА */
				if( *s != 0xff && (*s != ' ' || key_blan) ){
					*c++ = *s;
					 key_blan++;
				}
				s++;
			}
			*c =  '\0';
			while( *--c && *c == ' ' && c != p->t){
				*c =  '\0';
			}
			if(Maska->divide){
				while(*s && *s != Maska->divide )
					s++;
			}
			if(n == -2)
				err ("Переполнение поля %d", p->nom+1);
		}else{
			c = p->t;
			if(Maska->divide){
				while( *s && *s == Maska->divide )  /* До числа */
					s++;
			}
			if(p->key & LEFT){
				while( *s && *s != Maska->divide && n--)    /* Число */
					*c++ = *s++;
				*c =  '\0';
			}else{
				if(Maska->divide){
					while( *s && *s != Maska->divide)    /* Число */
						s++;
				}
				bs = s--;
				c = p->t + n ;
				*c-- =  '\0';
				while( *s != Maska->divide && n--)    /* Число */
					*c-- = *s--;
				s = bs;
				nn = n;
				while( nn-- > 0 )    /* Число */
					*c-- = ' ';
			}
			if(n == -1)
				err ("Переполнение числового поля %d", p->nom);
		}
		if(Maska->divide){
			while( *s && *s != Maska->divide ){         /* До числа */
				s++;
			}
		}
		p = p->next;
	} while (p != Maska->pol);
}
int command (struct  task   *ts, struct packet  *pk){  // *+ command ()   Интерпритация комманд от задач.
	int     info;
	info = pk->info >> 8;
	pk->text[pk->lenth] = '\0';
	switch(pk->comm){
	   case ENTER  :    /* Завершение команды ENTER */
	   case RESET  :    /* После KOR только ENTER и RESET */
	   case MESSG  :    /* Сообщение в строке состояния */
	   case TXT_STR:    /* 201 Сообщение о начале данных */
	   case TXT_END:    /* 202 Сообщение о конце данных */
	   case DIALOG :    /* 700 Запись не найдена */
		if(pk->lenth > 3 )
			pk->text[2] = ' ';
		err(&pk->text[-1]);
		break;
	   case ANSWER :    /* Диалоговый ответ */
		miracle (pk->text);
		if(pk->info){
			register struct pol *l = Maska->pol ;
			do {
				if(info--)
					break;
				l = l->next;
			} while (l != Maska->pol);
			Maska->cur = l;
		}
		break;
	   case LPR    :    /* Безформатная печать */
/*
/* Что то типа этого
/**/
/*                 dpend ();
/* #! /bin/sh -
/* ## pre - ФИЛЬТР локальной печати на принтер с альт. кодировкой IBM/PC
/* ##stty litout             >/dev/tty
/* echo '[5i'            >/dev/tty
/* cat  $* | dd conv=topc  >/dev/tty  2>/dev/null
/* echo '[4i'            >/dev/tty
/* ##stty -litout            >/dev/tty
/*                 dpbeg ();
/**/
		break;
	   case DOK_DST:    /* Установить счетчик документов */
	   case DOK_BDT:    /* Установить счетчик документов */
		break;
	   case N_ROLL :    /* Вывод на DST ROLL блокирoван */
	   case Y_ROLL :    /* Вывод на DST ROLL разрешен */
/*         case KWT:        /* Команды от HOST к BDT */
		if(info == ACK_KWT){                /* Положительное квитирование */
			return(0);
		}else if( info == NAK_IND){     /* Отрицательное квитирование */
			err(&pk->text[0]);
		}else if( info == NAK_LPR){     /* Отрицательное квитирование */
			err(&pk->text[0]);
		}else{
			if(Maska->dir & TAB){
				struct pol *l = Maska->cur;
				do{
					if(l->key & PTAB)
						break;
					l = l->next;
				}while (l != Maska->pol);
				strncpy(l->t,&pk->text[0],l->l);
				l->t[l->l] = 0;
				l = l->next;
				while (l != Maska->pol){
					if(l->key & PTAB)
						break;
					l = l->next;
				}
				if(pk->comm == Y_ROLL){
					while (l != Maska->pol){
						*l->t = 0;
						l = l->next;
					}
				}
				Maska->cur = l;
				if(Maska->cur == Maska->pol)
					return(Y_ROLL);
				else
					return(N_ROLL);
			}else{
				dpp(0,info-1);
				dpo(_CD);
				dps(&pk->text[0]);
				Disp = 1;
				return(pk->comm );
			}
		}
		break;
	   case TEXT:                           /* Вывод текста */
		if(info == TXT_BDT){            /* Текст на индикацию */
			err(&pk->text[0]);
		}else if(info == TXT_LPR){     /* Текст на печать */
			err(&pk->text[0]);
		}
		break;
	   case START:                          /* Старт FP */
	   {
		struct pol *n;

		Task_Tab = Maska->task;
		n = (struct pol *) malloc (SIZ(pol));
		n->d = (char*)strncpy(malloc(pk->lenth +2),&pk->text[-1],pk->lenth +1);
		longjmp(Exec,(long int) n);
	   }
	   default:
		err("comm нет команды");
		break;
	}
	return(0);
}

