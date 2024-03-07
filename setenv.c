#include <unistd.h>
#include <stdio.h>
#include "mu.h"
extern char   **environ;
void mu_set(int key){      //  *+ mu_set()     Извлеч и сохранить внешние переменные
	register char **ep = environ;
	register char  *cp, *dp, *en, *ev;
	int             i;
	char           *Setenv = ".setenv.u";   /* Сохранить окружение */
	char           *Config = "/.config/";  /* Читать окружение $Home */
	char           *MU = "MU";              /* Первая переменная */
	char            tmp[L_SIZ],buf[L_SIZ], *val, *index();
	FILE           *fp;		/* Дескриптор файла v */

	fp = NULL;
	if(key){        /* Начало работы */
	       {
		   extern char *pidFirstMU;    /* первый в стеке mu  */
		   if ( NULL == getenv (pidFirstMU)){
		       pid_t pFMU;
		       char buf[32];
		       pFMU=getpid();        // рекурсивный вызов mu
		       sprintf(buf,"%d",pFMU);
		       setenv (pidFirstMU, buf, 1);  //  killpg(pFMU, SIGTERM); mu.c завершить все процессы по f10
		       getpgid(0);        // рекурсивный вызов mu c одной группой
		   }
		}
		setenv  (MU, MU, 1);
		if ((fp = fopen (strcat(strcat(strcpy(tmp,getenv("HOME")),Config),Setenv), "r")) != NULL) {
			while (fgets (buf, L_SIZ, fp) != NULL) {
				if (!(i = strlen (buf)))
					continue;
				if (buf[i - 1] == '\n')
					buf[i - 1] = '\0';
				val = index (buf, '=');
				if( val != 0 ){
				   *val = '\0';
				   if ( NULL == getenv (buf)){           // если нет
					setenv (buf, val + 1, 1);
				   }
				}
			}
		    fclose (fp);
		}
		if ((fp = fopen (Setenv, "r")) != NULL) {
			while (fgets (buf, L_SIZ, fp) != NULL) {
				if (!(i = strlen (buf)))
					continue;
				if (buf[i - 1] == '\n')
					buf[i - 1] = '\0';
				val = index (buf, '=');
				if( val != 0 ){
				   *val = '\0';
				   if ( NULL == getenv (buf)){           // если нет
					setenv (buf, val + 1, 1);
					setenv (strcat(strcpy(tmp,"MU_"),buf), "", 1);
				   }
				}
			}
			fclose (fp);
		}
	}else{          /* Конец работы */
	   if( !fp ){
		   if ((fp = fopen (Setenv, "w")) == NULL)
			   return;
	   }
	   for (; *ep; ep++) {
		   for (cp = MU, dp = *ep; *cp && *cp == *dp; cp++, dp++)
			   continue;
		   if (*cp != 0 || *dp != '_')
			   continue;
		   for (dp++, en=tmp; '=' != *dp && 0 != *dp;en++,dp++)
			  *en = *dp;
		   *en = 0;
		   if ( NULL != (ev=getenv (en=tmp))){
//                   printf("<%s=%s>\n",en,ev);
			fputs (en,   fp);
			fputc ('=',  fp);
			fputs (ev,   fp);
			fputc ('\n', fp);
		   }
	   }
	   fclose(fp);
	}
}
