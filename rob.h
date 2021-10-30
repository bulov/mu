/*
 * Copyright (c) 1989 Regents of the Прессово-Арматурное Производство
 * of Тольятти. All rights reserved.  The Moscow software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *      @(#)rob.h       1.1  (Тольятти) 3/10/89  Булов В.Г. (bvg)
 *
 * Symbol Definitions
 *
 * Команды от HOST к DST
 */
#define ENTER   0x06			/* Завершение команды ENTER */
#define RESET   0x08			/* После KOR только ENTER и RESET */
#define MESSG   0x09			/* Сообщение в строке состояния */
#define TXT_STR 0x1B			/* 201 Сообщение о начале данных */
#define TXT_END 0x1A			/* 202 Сообщение о конце данных */
#define DIALOG  0x22			/* 700 Запись не найдена */
#define N_ROLL  0x0B			/* Вывод на DST ROLL блокирoван */
#define Y_ROLL  0x0C			/* Вывод на DST ROLL разрешен */
#define ANSWER  0x12			/* Диалоговый ответ */
#define LPR     0x20			/* Безформатная печать */
#define DOK_DST 0x21                    /* Установить счетчик документов */
/*
 * Команды от HOST к BDT
 */
#define KWT     0x0C
#define     ACK_KWT     0x40		/* Положительное квитирование */
#define     NAK_IND     0x41		/* Отрицательное квитирование */
#define     NAK_LPR     0x42		/* Отрицательное квитирование */
#define START   0x0D			/* Старт FP */
#define TEXT    0x10			/* Вывод текста */
#define     TXT_BDT     0x41		/* Текст на индикацию */
#define     TXT_LPR     0x42		/* Текст на печать */
#define ON_OFF  0x11			/* Машинный интерфейс */
#define     ON_         0x20            /* включить */
#define     OFF_        0x30            /* выключить */
#define DOK_BDT 0x15			/* Установить счетчик документов */
/*
** Symbol Definitions
*/
#define CLEAR           -1              /* См d_line() */
#define DISP            -2              /* См d_line() */
#define NAME_LENTH      4		/* Длина имени транзакции */
#define N_L             NAME_LENTH
#define STR_DST         64		/* Длина строки дисплея DST */
#define TOP_DST         14		/* Количество строк дисплея DST */
#define STR_BDT         16		/* Длина строки дисплея BDT */
#define STR_K_6310      80		/* Длина строки печати К-6310 */
#ifndef TRUE
#define TRUE           -1		/* Boolean constants */
#endif
#ifndef FALSE
#define FALSE           0
#endif
#define F_ROLL  0x83			/* Перелистывание вперед <- ROLL */
#define B_ROLL  0x84			/* Перелистывание назад  -> ROLL */
/*
** Structrure Definitions
*/
#define HEAD            12
#define MAX_BODY        505		/* Длина передаваемой записи */
struct packet {
	short           adres;		/* Адрес устройства */
	short           keep;		/* Резерв */
	short           comm;		/* Соманда */
	short           lenth;		/* Длина команды/данных */
	short           lenth_high;
	short           info;		/* Подтип команды */
	char            text[MAX_BODY];	/* Тело данных */
};
/*
** Macro Definitions
*/
#define ROL(x,y)  ((x >> y) & 0x0F)
#define S_C(x)    (((x & 0x0F) | ROL(x,8) << 4) & 0xFF)
#define C_S(x)    (((x & 0x0F) | 0x20) | ((ROL(x,4)) | 0x30) <<8)
#define PRMSG(fun)      printmsg("<fun> Нет команды %x \n",com);
#define KWIT(p)   (p->comm == 3  && p->info &( 1 << 6 ))
#define REC     &Pk->text[N_L]
#define FIN(n,x)    n(x,x)
#define FOUT(n,x)   n(x,&x)
/*
 * Tипы функций
 */
struct packet  *rd_rob ();
#ifdef RD_ROB
struct packet  *Pk;
#else
extern struct packet  *Pk;
#endif
