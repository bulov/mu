.IGNORE:

DEST          = /u/bvg/bin

EXTHDRS       = rob.h \
		/usr/include/ctype.h \
		/usr/include/fcntl.h \
		/usr/include/sys/param.h \
		/usr/include/setjmp.h \
		/usr/include/sgtty.h \
		/usr/include/signal.h \
		/usr/include/stdio.h \
		/usr/include/sys/file.h \
		/usr/include/sys/ioctl.h \
		/usr/include/sys/param.h \
		/usr/include/sys/termio.h \
		/usr/include/sys/termios.h \
		/usr/include/sys/time.h \
		/usr/include/sys/ttychars.h \
		/usr/include/sys/ttydev.h \
		/usr/include/sys/types.h \
		./include/tty.h

HDRS          = mu.h

LDFLAGS       =

#CFLAGS        =  -g -I/usr/include/sys/ -I/usr/include/gcc/sys/ -I/usr/include/gcc/ -I/usr/include/ -DM_TERMINFO  -L/usr/local/lib/gcc
#CFLAGS        =  -O -DM_TERMINFO
CFLAGS        =  -g -DM_TERMINFO

LIBS          =  dp_io.new/dp_io.a -lcurses\
#
LIBS          =   -lcursesw


#LINKER        =    gcc
LINKER        =    gcc

CC            =    gcc

MAKEFILE      = Makefile

OBJS	      = cmc.o \
		dpline.o \
		fpm.o \
		mu.o \
		queues.o \
		readmenu.o \
		setenv.o \
		red.o \
		dp_io.o \
		task.o

PRINT	      = pr

PROGRAM       = mu

SRCS	      = cmc.c \
		dpline.c \
		fpm.c \
		mu.c \
		queues.c \
		readmenu.c \
		setenv.c \
		dp_io.c \
		red.c \
		task.c

all:            $(PROGRAM)

$(PROGRAM):     $(OBJS)
		@echo -n "Loading $(PROGRAM) ... "
		$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		strip $(PROGRAM)
		mv $(PROGRAM) $(DEST)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DEST=$(DEST) install
###
fpm.o:      mu.h
readmenu.o: mu.h
setenv.o:   mu.h
cmc.o:      mu.h tty.h rob.h
task.o:     mu.h tty.h rob.h
queues.o:   mu.h tty.h
mu.o:       mu.h tty.h
red.o:      mu.h tty.h
dpline.o:        tty.h
dp_io.o:         tty.h
