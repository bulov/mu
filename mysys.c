#include   <stdio.h>
#include   <fcntl.h>
#include   <sys/types.h>                  /* Новые типы переменых */
#include   <sys/socket.h>
#include   <sys/stat.h>
/* FILE   *mysys(char *command, FILE **fe) /**/
FILE   *mysys(char *command)
{
   int     i=0;
   int     par[2], chl[2], ero[2]; /* pipes to parent, child */
   FILE    *fr;

   pipe (ero);
   pipe (par);
   pipe (chl);

   if ((i = fork ()) == 0){        /* Раздвояемся */
/*     close   (0);        dup     (chl[0]);   /* dup pipe read to standart input */
       close   (1);        dup     (par[1]);   /* pipe на запись */
       close   (2);        dup     (ero[1]);   /* err */
/*     close   (chl[0]);   close   (chl[1]); /**/
       close   (par[1]);   close   (par[0]);
       close   (ero[1]);   close   (ero[0]);

       exit(system (command));

   }
/* close (chl[0]);   /**/
   close (par[1]);
   close (ero[1]);
   if( ( fr = fdopen(par[0], "r") ) == NULL )
       perror("fdopen");
/*   if( (*fe = fdopen(ero[0], "r") ) == NULL )
       perror("fdopen");
*/
   return( fr);
}
#ifdef EBUG
main(int argc, char **argv)
{
   int     i = 0;
   char    buf[100];
   FILE    *fr,    *fe;
   char    *str="(l | cut -c30- | sort )";

   if(argc > 1 )
       str = *++argv;
   fr = mysys(str, &fe);
   i=0;
   printf("stdout\n");
   while((fgets(buf, sizeof buf, fr ) != NULL) ){
       printf("%d %s",i++, buf);
   }
   printf("stderr\n");
   while((fgets(buf, sizeof buf, fe ) != NULL) ){
       printf("%d %s",i++, buf);
   }
   fclose(fr);
   fclose(fe);
}
#endif
