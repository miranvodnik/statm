
-include ../makefile.defs

COPTS = -g -Wall -c -fmessage-length=0 -fPIC $(UNIX_FLAGS)
CPPOPTS = -g -Wall -c -fmessage-length=0 -fPIC $(UNIX_FLAGS)
IDIR = -I. \
	-I$(PRODUCT_DIR)/statm-include \
	-I$(PRODUCT_DIR)/statm-errinfo \
	-I/usr/include/libxml2 \

CFLAGS = $(DEF_FLAGS) $(COPTS) $(IDIR)
CPPFLAGS = $(DEF_FLAGS) $(CPPOPTS) $(IDIR)
LOPTS = -shared
LIBS = -lxdbdodbc
LPATH = -L$(PRODUCT_DIR)/../xdbd/lib/

OBJS = \
	./StatisticalAdapterThread.o \
	./StatisticalAdapter.o 

all: libstatstat.so

# Tool invocations
libstatstat.so: $(OBJS) $(USER_OBJS)
	$(CXX) $(LOPTS) $(LPATH) -o $@ $(OBJS) $(LIBS)
	cp $@ ../lib

# Other Targets
clean:
	-@rm -f $(OBJS) libstatstat.so
