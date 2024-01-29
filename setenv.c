#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include "mu.h"
extern char   **environ;
/*char *index(str,cha)
	register char *str, cha;
{
	for ( ; *str != cha && *str != 0 ;   str++ )
	return ( str );
}
*/
/*
*+ blklen (), blkcpy (), blkend (), blkcat (), strspl ()
*/
int blklen (register char **av){
	register int i = 0;

	while (*av++)
		i++;
	return (i);
}
char **blkcpy (char **oav,register char ** bv){
	register char **av = oav;

	while (*av++ = *bv++);
	return (oav);
}
char **blkend (register char **up){

	while (*up)
		up++;
	return (up);
}
char **blkcat (char **up,char **vp){

	(void) blkcpy (blkend (up), vp);
	return (up);
}
char **blkspl (register char **up,register char **vp){
	register char **wp = (char **) malloc ((unsigned)
		((blklen (up) + blklen (vp) + 1)) * sizeof (char **));

	(void) blkcpy (wp, up);
	return (blkcat (wp, vp));
}
char *strspl (char *cp,char *dp){
	char           *ep;
	register char  *p, *q;

	for (p = cp; *p++;);
	for (q = dp; *q++;);
	ep = (char *) malloc ((unsigned) ((p - cp) + (q - dp) - 1));
	for (p = ep, q = cp; *p++ = *q++;);
	for (p--, q = dp; *p++ = *q++;);
	return (ep);
}
/*
*+ setenvMy()     Установить в окружение title=val
*+              не работает рекурсия path=$path/usr/bin; ???? bvg
*/
void setenvMy (char *b_name,char *b_val){
extern char   **environ;
	register char **ep = environ;
	register char  *cp, *dp;
	char           *blk[2], **oep = ep;
	char            name[L_SIZ], val[L_SIZ];

#define COMPRESS(SS,DD) {register char  *ss, *se;\
		for(ss = SS, se = DD; *se && *se == ' '; se++);\
		for(; *se; *ss++ = *se++);\
		*ss = '\0';\
		for(--ss; *ss == ' '; ss--)\
			*ss = '\0';\
	}
	COMPRESS(name,b_name);
	COMPRESS(val,b_val);
	for (; *ep; ep++) {
		for (cp = name, dp = *ep; *cp && *cp == *dp; cp++, dp++)
			continue;
		if (*cp != 0 || *dp != '=')
			continue;
		cp = strspl ("=", val);
		FREE (*ep);
		*ep = strspl (name, cp);
		FREE (cp);
		return;
	}
	blk[0] = strspl (name, "=");
	blk[1] = 0;
	environ = blkspl (environ, blk);
//        FREE ((char *) oep);
	setenv (name, val,1);
}
void mu_set(int key){      //  *+ mu_set()     Извлеч и сохранить внешние переменные
	register char **ep = environ;
	register char  *cp, *dp;
	int             i;
	char           *Setenv = ".setenv.u";   /* Сохранить окружение */
	char           *Name = "MU";            /* Первая переменная */
	char            buf[L_SIZ], *val, *index();
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
		unsetenv(Name);
		setenv  (Name, Name, 0);
//                setenv  (Name, Name, 1);
			if ((fp = fopen (Setenv, "r")) != NULL) {
			while (fgets (buf, L_SIZ, fp) != NULL) {
				if (!(i = strlen (buf)))
					continue;
				if (buf[i - 1] == '\n')
					buf[i - 1] = '\0';
				val = index (buf, '=');
				if( val != 0 ){
				   *val = '\0';
				   setenv (buf, val + 1, 1);
				}
			}
			fclose (fp);
		}
	}else{          /* Конец работы */
		for (; *ep; ep++) {
			for (cp = Name, dp = *ep; *cp && *cp == *dp; cp++, dp++)
				continue;
			if (*cp != 0 || *dp != '=')
				continue;
			for (ep++; *ep; ep++) {
				if( !fp )
					if ((fp = fopen (Setenv, "w")) == NULL)
						return;
				if(**ep != '_' ){
					fputs (*ep, fp);
					fputc ('\n', fp);
				}
			}
			return;
		}
	}
}
