#/*#######################################################################*/
#/*#                                                                     #*/
#/*#                                                                     #*/
#/*#            Copyright (c) 2014 Iskra Tel                             #*/
#/*#                                                                     #*/
#/*#                  Support Software System                            #*/
#/*#                                                                     #*/
#/*#                                                                     #*/
#/*# Name        : xdbdmpx.mk                                            #*/
#/*#                                                                     #*/
#/*# Description : FTP client library                                    #*/
#/*#                                                                     #*/
#/*# Code        : XXXX - XXXXXXX                                        #*/
#/*#                                                                     #*/
#/*# Date        : Nov 2014                                              #*/
#/*#                                                                     #*/
#/*# Author      : Miran Vodnik ITWEN1                                   #*/
#/*#                                                                     #*/
#/*# Compilation : xdbdmpx.mk - project makefile                         #*/
#/*#                                                                     #*/
#/*# Revisions   :                                                       #*/
#/*#                                                                     #*/
#/*#######################################################################*/

#/***************************************************************/
#/*          UNIX - SCCS  VERSION DESCRIPTION                   */
#/***************************************************************/
#/*static char  unixid_XXXX[] = "%W%	%D%	xdbdmpx.mk";*/

-include ../makefile.defs

# -Werror             = report any warning as error
COPTS = -fPIC -Wall -c -g $(UNIX_FLAGS)
IDIR = \
	-I$(PRODUCT_DIR)/statm-include \
	-I$(PRODUCT_DIR)/statm-errinfo \
	-I$(PRODUCT_DIR)/statm-msg \
	-I$(PRODUCT_DIR)/statm \

CPPFLAGS = $(DEF_FLAGS) $(COPTS) $(IDIR)
LDIR = \
	-L$(PRODUCT_DIR)/lib \

LOPTS = -shared
LIBS = -lstatmsg

OBJS = \
	StatmRunnable.o \
	StatmRunningContext.o \
	StatmBaseJob.o \
	StatmMessageQueue.o \
	StatmWorkingThread.o \

library: libstatmpx.so

-include .deps/*.d

libstatmpx.so: $(OBJS)
	$(CXX) $(LOPTS) $(LDIR) $(LIBS) $(OBJS) -o $@
	-@cp $@ ../lib

$(OBJS):

clean:
	-@rm -rf libstatmpx.so $(OBJS)

