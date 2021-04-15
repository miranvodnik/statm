
-include ../makefile.defs

CPPOPTS = -g -fPIC -c -Wall $(UNIX_FLAGS)
IDIR = -I. \
	-I$(PRODUCT_DIR)/statm-include \
	-I$(PRODUCT_DIR)/statm-sql-parser \
	-I$(PRODUCT_DIR)/statm-msg \
	-I$(PRODUCT_DIR)/statm-job \
	-I$(PRODUCT_DIR)/statm-mpx \
	-I$(PRODUCT_DIR)/statm-core \
	-I$(PRODUCT_DIR)/statm-errinfo \
	-I$(PRODUCT_DIR)/statm-ham \
	-I/usr/include/libxml2
CPPFLAGS = $(DEF_FLAGS) $(CPPOPTS) $(IDIR)
LOPTS = -shared
LIBS := -lrt -lstatcore -lxdbdodbc -lxdbdapi -lstatmsg -lstatstat -lstatmpx -lstaterrinfo -luuid -lpthread `xml2-config --libs`
LDIR = -L$(PRODUCT_DIR)/lib -L$(PRODUCT_DIR)/../xdbd/lib

OBJS += \
	./statm-main.o 

all: statm

statm: $(OBJS)
	$(CXX) $(OBJS) $(LDIR) $(LIBS) -o $@
	cp $@ ../bin
	cp $@ ../bin/statm-client
	cp $@ ../bin/statm-worker

$(OBJS):

clean:
	-@rm -f statm $(OBJS)
	
