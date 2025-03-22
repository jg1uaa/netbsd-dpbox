###############################################################################
#                                                                             #
# This is the Makefile for DPBOX					      #
#     	      	      	      	      	      	      	      	      	      #
# DPBOX should compile automatically on Linux, NetBSD and MacOS.      	      #
# Simply type: make clean; make dep; make all  	      	      	      	      #
#     	      	      	      	      	      	      	      	      	      #
# The system type is configured through existance of the following defines:   #
# __linux__, __NetBSD__, __macos__    	      	      	      	      	      #
#     	      	      	      	      	      	      	      	      	      #
# For other systems, check the conditional defines made for __linux__ in      #
# the source files and add appropriate code for your system.  	      	      #
# Please send a diff to the maintainer to get your adaptions included in the  #
# source tree.	      	      	      	      	      	      	      	      #
#                                                                             #
# Uncomment NOSHELL if you don´t want a shell access from within dpbox	      #
#                                                                             #
#NOSHELL	= -DNO_SHELL
#                                                                             #
# That's all you have to configure                                            #
#     	      	      	      	      	      	      	      	      	      #
# changed 30.03.2000 J.Schurig, DL8HBS                                        #
#     	      	      	      	      	      	      	      	      	      #
###############################################################################

INSTALL       	= install -c
INSTALL_SPROGRAM = ${INSTALL} -s -m 700
INSTALL_PROGRAM = ${INSTALL} -m 700
INSTALL_WPROG	= ${INSTALL} -m 755
INSTALL_DATA  	= ${INSTALL} -m 644
INSTALL_CONFIG  = ${INSTALL} -m 600
MKDIR 	      	= ${INSTALL} -d -m 700
MKDIRWR	      	= ${INSTALL} -d -m 755
MKDIRDEF      	= ${INSTALL} -d
RMDIR 	      	= rm -fr
RM    	      	= rm -f
LN    	      	= ln -sf
SAVE   	      	= tar -pSPcz
RESTORE       	= tar -pSPxz

# These are the standard locations for dpbox
#prefix		= /usr/local
# SuSE has chosen an installation in /usr, so, for sake of building a new
# binary rpm, let's change it, too
prefix		= /usr
confdir       	= /etc/dpbox
initdir       	= /etc/rc.d
doclink	      	= /usr/doc/packages/dpbox
spooldir      	= /var/spool/dpbox
logdir	      	= /var/log/dpbox

cgidir		= /usr/local/httpd/cgi-bin
httpdir		= /usr/local/httpd/htdocs

bindir       	= $(prefix)/bin
sbindir       	= $(prefix)/sbin
sharedir   	= $(prefix)/share/dpbox
docdir		= $(prefix)/share/dpbox/doc
landir   	= $(prefix)/share/dpbox/language

savedate	:= $(shell date +%Y%m%d-%H%M%S)
savefile      	= /var/tmp/dpbox-saveinst

# temp install dir, used for cgi script conversion
tmpinstdir     	= /tmp/dpboxinst

DEFS		= $(NOSHELL)
CC		= gcc
CPP		= $(CC) -E
CFLAGS		= -Wall -O2 -funsigned-char -fomit-frame-pointer -fno-strength-reduce
CRAWLERCFLAGS	= -Wall -O3 -funsigned-char -fomit-frame-pointer -fno-strength-reduce
LD		= $(CC)
LDFLAGS		= -s
LIBS		= -lm

HDRS		= filesys.h box.h box_file.h box_mem.h box_send.h box_sf.h \
                  box_sub.h box_sys.h boxdbox.h boxbcm.h box_tim.h box_init.h \
		  boxglobl.h boxlocal.h box_inou.h box_serv.h boxbcast.h \
		  boxfserv.h yapp.h crc.h huffman.h pastrix.h sort.h tools.h \
                  init.h dpbox.h ifacedef.h status.h linpack.h shell.h main.h \
		  box_garb.h box_logs.h box_wp.h box_rout.h box_scan.h md2md5.h

OBJS		= filesys.o box.o box_mem.o box_send.o box_sf.o box_sub.o \
                  box_sys.o boxdbox.o boxbcm.o box_tim.o boxglobl.o boxlocal.o \
		  box_inou.o box_serv.o boxbcast.o boxfserv.o yapp.o crc.o \
		  huffman.o pastrix.o sort.o box_file.o tools.o status.o \
		  shell.o init.o linpack.o main.o box_init.o box_garb.o box_logs.o \
		  box_wp.o box_rout.o box_scan.o md2md5.o

SRCS		= filesys.c box.c box_file.c box_mem.c box_send.c box_sf.c \
      	      	  box_sub.c box_sys.c boxdbox.c boxbcm.c box_tim.c boxglobl.c \
		  boxlocal.c box_inou.c box_serv.c boxbcast.c boxfserv.c \
		  yapp.c crc.c huffman.c pastrix.c sort.c tools.c status.c \
		  shell.c init.c linpack.c main.c box_init.c box_garb.c box_logs.c \
		  box_wp.c box_rout.c box_scan.c md2md5.c
		
OTHER_SRCS	= dpputlinks.c dpcodeconv.c dpgate.c crawler.c dpmybbs.c

all:		dpbox	crawler	cgi	toolbox

csource:	$(SRCS)

.c.o:	
		$(CC) $(CFLAGS) $(DEFS) -c $<

.c:
		$(CC) $(CFLAGS) $*.c -o $*

crawler:	crawler.c
		$(CC) $(CRAWLERCFLAGS) crawler.c -o crawler

toolbox:	dpmybbs

cgi:		dpputlinks dpcodeconv dpgate

dpbox:		$(OBJS)
		$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o dpbox

instexe:	all
		@echo Copying executables...
		@$(INSTALL_SPROGRAM) dpbox $(sbindir)
		@$(INSTALL) -s -m u+s,a+x dpgate $(bindir)
		@$(INSTALL) -s -m a+x dpputlinks $(bindir)
		@$(INSTALL) -s -m a+x dpcodeconv $(bindir)
		@$(INSTALL) -s -m a+x dpmybbs $(bindir)
		@$(INSTALL_SPROGRAM) crawler $(spooldir)/crawler
		@echo done...
		
install:      	all
		@echo Copying files...

		@$(MKDIRDEF) $(sbindir)
		@$(INSTALL_SPROGRAM) dpbox $(sbindir)
		@$(MKDIRDEF) $(bindir)
		@$(INSTALL) -s -m u+s,a+x dpgate $(bindir)
		@$(INSTALL) -s -m a+x dpputlinks $(bindir)
		@$(INSTALL) -s -m a+x dpcodeconv $(bindir)
		@$(INSTALL) -s -m a+x dpmybbs $(bindir)

		@$(MKDIRWR) $(docdir)
		@$(INSTALL_DATA) ../document/* $(docdir)
ifdef doclink
		@$(LN) $(docdir) $(doclink)
endif
		@$(MKDIR) $(confdir)
		@$(INSTALL_CONFIG) ../dpbox.ini $(confdir)
		@$(MKDIR) $(confdir)/system
		@$(INSTALL_CONFIG) ../system/* $(confdir)/system
		@$(MKDIR) $(confdir)/sf
		@$(INSTALL_CONFIG) ../sf/* $(confdir)/sf

		@$(MKDIR) $(spooldir)/indexes
		@$(MKDIR) $(spooldir)/infofiles
		@$(MKDIR) $(spooldir)/newmail
		@$(MKDIR) $(spooldir)/import
		@$(MKDIR) $(spooldir)/run
		@$(MKDIR) $(spooldir)/save

		@$(MKDIR) $(spooldir)/crawler
		@$(INSTALL_SPROGRAM) crawler $(spooldir)/crawler
		@$(INSTALL_DATA) ../crawler/stopwords $(spooldir)/crawler

		@$(INSTALL_PROGRAM) ../run/* $(spooldir)/run

		@$(MKDIRWR) $(spooldir)/fileserv
		@$(MKDIRWR) $(spooldir)/fileserv/incoming
		@$(MKDIR) $(spooldir)/fileserv/temp7pl
		@$(MKDIRWR) $(spooldir)/fileserv/newbin
		@$(MKDIR) $(spooldir)/privserv
		@$(MKDIR) $(spooldir)/privserv/temp7pl

		@$(MKDIR) $(sharedir)
		@$(MKDIR) $(landir)

		@$(INSTALL_DATA) ../language/* $(landir)
		@$(LN) DL.ALL $(landir)/BS.ALL
		@$(LN) DL.ALL $(landir)/DLX.ALL
		@$(LN) DL.ALL $(landir)/EY.ALL
		@$(LN) DL.ALL $(landir)/HB.ALL
		@$(LN) HELP.DL $(landir)/HELP.BS
		@$(LN) HELP.DL $(landir)/HELP.DLX
		@$(LN) HELP.DL $(landir)/HELP.EY
		@$(LN) HELP.DL $(landir)/HELP.HB
		@$(LN) HELP.DL $(landir)/HELP.OE1
		@$(LN) HELP_SYS.DL $(landir)/HELP_SYS.BS
		@$(LN) HELP_SYS.DL $(landir)/HELP_SYS.DLX
		@$(LN) HELP_SYS.DL $(landir)/HELP_SYS.EY
		@$(LN) HELP_SYS.DL $(landir)/HELP_SYS.HB
		@$(LN) HELP_SYS.DL $(landir)/HELP_SYS.OE1
		@$(LN) DL.ALL $(landir)/OE1.ALL

		@$(MKDIR) $(logdir)

		@$(MKDIR) $(spooldir)/stat
		@$(MKDIR) $(spooldir)/stat/extusers
		@$(MKDIR) $(spooldir)/server

		@$(LN) $(sharedir)/language $(spooldir)/language
		@$(LN) $(confdir)/sf $(spooldir)/sf
		@$(LN) $(confdir)/system $(spooldir)/system
		@$(LN) $(docdir) $(spooldir)/document
		@$(LN) $(logdir) $(spooldir)/proto
ifdef initdir
		@$(INSTALL_PROGRAM) ../dpbox.init.d $(initdir)/dpbox
		@$(LN) $(initdir)/dpbox $(initdir)/rc2.d/S56dpbox
		@$(LN) $(initdir)/dpbox $(initdir)/rc2.d/K56dpbox
		@$(LN) $(initdir)/dpbox $(initdir)/rc3.d/S56dpbox
		@$(LN) $(initdir)/dpbox $(initdir)/rc3.d/K56dpbox
		@$(LN) $(initdir)/dpbox $(sbindir)/rcdpbox
endif
		@$(MKDIR) $(sharedir)/http
		@$(INSTALL) -m a+r ../cgi/dpboxcgi/dptop.html $(sharedir)/http
		@$(INSTALL) -m a+r ../cgi/dpboxcgi/dpbottom.html $(sharedir)/http
		
		@if [ -e ${cgidir} ]; then \
		  echo "installing cgi-scripts"; \
		  $(RMDIR) $(tmpinstdir); \
		  $(MKDIR) $(tmpinstdir); \
		  if [ -e $(tmpinstdir) ]; then \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dpbbs >$(tmpinstdir)/dpbbs ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dpcheck >$(tmpinstdir)/dpcheck ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dpcmd >$(tmpinstdir)/dpcmd ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dpdir >$(tmpinstdir)/dpdir ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dpftr >$(tmpinstdir)/dpftr ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dplist >$(tmpinstdir)/dplist ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dplist.neu >$(tmpinstdir)/dplist.neu ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dploc >$(tmpinstdir)/dploc ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dpprefix >$(tmpinstdir)/dpprefix ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dpsertit >$(tmpinstdir)/dpsertit ; \
		    sed "s,/installdir,$(bindir)," <../cgi/cgi/dpuser >$(tmpinstdir)/dpuser ; \
		    sed "s,/installdir,$(bindir)," <../cgi/dpboxcgi/dphttp >$(tmpinstdir)/dphttp ; \
		    $(INSTALL) -m a+x,a+r $(tmpinstdir)/dphttp $(bindir); \
		    $(RM) $(tmpinstdir)/dphttp; \
		    $(INSTALL_WPROG) $(tmpinstdir)/* $(cgidir); \
		    $(INSTALL_DATA) ../cgi/dpboxcgi/*.html $(cgidir); \
		    $(RMDIR) $(tmpinstdir); \
		  fi \
		fi

		@if [ -e ${httpdir} ]; then \
		  echo "installing html pages"; \
		  $(MKDIRWR) $(httpdir)/dpbox/pic; \
		  $(INSTALL_DATA) ../cgi/dpbox/*.html $(httpdir)/dpbox; \
		  $(INSTALL_DATA) ../cgi/dpbox/pic/* $(httpdir)/dpbox/pic; \
		fi

		@echo dpbox is installed
		@echo Now edit the configuration files in $(confdir), at least $(confdir)/dpbox.ini !

clean:
	      	@rm -f  *.o *~ .*~ *.s dpbox crawler dpgate dpputlinks dpcodeconv dpmybbs .depend tags
		@echo Source directory of dpbox cleaned

instsave:
		@$(SAVE) --one-file-system --exclude "msglist.box" -f $(savefile).$(savedate).tgz \
		      	 $(confdir) \
			 $(logdir) $(spooldir)/indexes $(spooldir)/infofiles $(spooldir)/stat \
			 $(spooldir)/run $(spooldir)/save $(spooldir)/server $(spooldir)/privserv \
			 $(spooldir)/fileserv
		@$(RM) $(savefile).tgz
		@$(LN) $(savefile).$(savedate).tgz $(savefile).tgz
		@echo Configuration saved in $(savefile).tgz

instrestore:
		@cd /
		@$(RESTORE) -f $(savefile).tgz
		@echo Configuration restored from $(savefile).tgz

instclean:	instsave  
		@$(RM) $(sbindir)/dpbox
		@$(RM) $(bindir)/dpgate
		@$(RM) $(bindir)/dpputlinks
		@$(RM) $(bindir)/dpcodeconv
		@$(RM) $(bindir)/dpmybbs
		@$(RM) $(bindir)/dphttp
		@if [ -e ${httpdir} ]; then \
		  $(RMDIR) $(httpdir)/dpbox; \
		fi
		@if [ -e ${cgidir} ]; then \
		  $(RM) $(cgidir)/dpbbs; \
		  $(RM) $(cgidir)/dpcheck; \
		  $(RM) $(cgidir)/dpcmd; \
		  $(RM) $(cgidir)/dpdir; \
		  $(RM) $(cgidir)/dpftr; \
		  $(RM) $(cgidir)/dplist; \
		  $(RM) $(cgidir)/dplist.neu; \
		  $(RM) $(cgidir)/dploc; \
		  $(RM) $(cgidir)/dpprefix; \
		  $(RM) $(cgidir)/dpsertit; \
		  $(RM) $(cgidir)/dpuser; \
		fi

ifdef initdir
		@$(RM) $(initdir)/rc2.d/S56dpbox
		@$(RM) $(initdir)/rc2.d/K56dpbox
		@$(RM) $(initdir)/rc3.d/S56dpbox
		@$(RM) $(initdir)/rc3.d/K56dpbox
		@$(RM) $(sbindir)/rcdpbox
endif
		@$(RMDIR) $(docdir)
ifdef doclink
		@$(RM) $(doclink)
endif
		@$(RMDIR) $(confdir)
		@$(RMDIR) $(sharedir)
		@$(RMDIR) $(logdir)
		@$(RMDIR) $(spooldir)

		@echo dpbox completely uninstalled		

depend:       	dep

dep:
	      	@$(CPP) -M $(DEFS) $(SRCS) $(OTHER_SRCS) > .depend
#	      	@ctags $(SRCS) $(HDRS) $(OTHER_SRCS)

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
