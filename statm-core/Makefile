
-include ../makefile.defs

COPTS = -g -Wall -c -fmessage-length=0 -fPIC $(UNIX_FLAGS)
CPPOPTS = -g -Wall -c -fmessage-length=0 -fPIC $(UNIX_FLAGS)
IDIR = -I. \
	-I$(PRODUCT_DIR)/statm-include \
	-I$(PRODUCT_DIR)/statm-sql-parser \
	-I$(PRODUCT_DIR)/statm-msg \
	-I$(PRODUCT_DIR)/statm-job \
	-I$(PRODUCT_DIR)/statm-mpx \
	-I$(PRODUCT_DIR)/statm-errinfo \
	-I$(PRODUCT_DIR)/statm-ham \
	-I$(PRODUCT_DIR)/statm-stat \
	-I/usr/include/libxml2
CFLAGS = $(DEF_FLAGS) $(COPTS) $(IDIR)
CPPFLAGS = $(DEF_FLAGS) $(CPPOPTS) $(IDIR)
LOPTS = -shared
LIBS = -lrt -lxdbdodbc -lxdbdapi -lstatmsg -lstatmpx -lstaterrinfo -luuid -lpthread `xml2-config --libs`
LPATH = -L$(PRODUCT_DIR)/lib -L$(PRODUCT_DIR)/../xdbd/lib

OBJS += \
	./StatmController.o \
	./StatmLocalClient.o \
	./StatmWorker.o \
	./StatmWorkingClient.o \
	./StatmClientHandler.o \
	./StatmClientDriver.o \
	./StatmStatDriver.o \
	./StatmChkStatDir.o \
	./StatmInitStatAdapter.o \

all: libstatcore.so

libstatcore.so: $(OBJS) $(USER_OBJS)
	$(CXX) $(LOPTS) $(LPATH) -o $@ $(OBJS) $(LIBS)
	cp $@ ../lib

$(OBJS):

clean:
	-@rm -f $(OBJS) libstatcore.so
	
