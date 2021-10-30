#include <malloc.h>
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
blklen (av)
	register char **av;
{
	register int i = 0;

	while (*av++)
		i++;
	return (i);
}
char          **
blkcpy (oav, bv)
	char          **oav;
	register char **bv;
{
	register char **av = oav;

	while (*av++ = *bv++);
	return (oav);
}
char          **
blkend (up)
	register char **up;
{

	while (*up)
		up++;
	return (up);
}
char          **
blkcat (up, vp)
	char          **up, **vp;
{

	(void) blkcpy (blkend (up), vp);
	return (up);
}
char          **
blkspl (up, vp)
	register char **up, **vp;
{
	register char **wp = (char **) malloc ((unsigned)
		((blklen (up) + blklen (vp) + 1)) * sizeof (char **));

	(void) blkcpy (wp, up);
	return (blkcat (wp, vp));
}
char           *
strspl (cp, dp)
	char           *cp, *dp;
{
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
*+ setenv()     Установить в окружение title=val
*+              не работает рекурсия path=$path/usr/bin; ???? bvg
*/
setenv (b_name, b_val)
	char           *b_name, *b_val;
{
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
	setenv (name, val);
}
/*
*+ mu_set()     Извлеч и сохранить внешние переменные
*/
mu_set(key)
{
	register char **ep = environ;
	register char  *cp, *dp;
	int             i;
	char           *Setenv = ".setenv.u";   /* Сохранить окружение */
	char           *Name = "MU";            /* Первая переменная */
	char            buf[L_SIZ], *val, *index();
	FILE           *fp;		/* Дескриптор файла v */

	fp = NULL;
	if(key){        /* Начало работы */
		setenv (Name, Name);
			if ((fp = fopen (Setenv, "r")) != NULL) {
			while (fgets (buf, L_SIZ, fp) != NULL) {
				if (!(i = strlen (buf)))
					continue;
				if (buf[i - 1] == '\n')
					buf[i - 1] = '\0';
				val = index (buf, '=');
				if( val != 0 ){
				   *val = '\0';
				   setenv (buf, val + 1);
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
				if(**ep != '_'){
					fputs (*ep, fp);
					fputc ('\n', fp);
				}
			}
			return;
		}
	}
}
