
-include ../makefile.defs

COPTS = -g -Wall -c -fmessage-length=0 -fPIC $(UNIX_FLAGS)
CPPOPTS = -g -fPIC -c -Wall -Wno-deprecated  $(UNIX_FLAGS) -D_LARGE_FILE_API `xml2-config --cflags`
IDIR = -I.
CFLAGS = $(DEF_FLAGS) $(COPTS) $(IDIR)
CPPFLAGS = $(DEF_FLAGS) $(CPPOPTS) $(IDIR)
LOPTS = -shared
LIBS = 
LPATH = 

OBJS = ./StatmMessages_xdr.o 

all: libstatmsg.so

StatmMessages_xdr.o: StatmMessages_xdr.c
	$(CC) $(CFLAGS) $(NO_WARNINGS) StatmMessages_xdr.c

StatmMessages_xdr.c StatmMessages.h: StatmMessages.x
	rpcgen StatmMessages.x

libstatmsg.so: $(OBJS)
	$(CXX) $(LOPTS) -o $@ $(OBJS) $(USER_OBJS) $(LIBS)
	cp $@ ../lib

$(OBJS):

clean:
	-@rm -f libstatmsg.so $(OBJS) \
	StatmMessages.h StatmMessages_clnt.c StatmMessages_svc.c StatmMessages_xdr.c
	
