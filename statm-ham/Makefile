
-include ../makefile.defs

CFLAGS = -fPIC -Wall -c -g $(UNIX_FLAGS)
IDIR = \
	-I$(PRODUCT_DIR)/statm-include \
	-I$(PRODUCT_DIR)/statm-mpx \
	-I$(PRODUCT_DIR)/statm-errinfo \

CXXFLAGS = $(DEF_FLAGS) $(CFLAGS) $(IDIR)
LDIR = \
	-L$(PRODUCT_DIR)/lib \

LOPTS = -shared
LIBS = 

OBJS = \
	StatmHamThread.o \

library: libstatham.so

-include .deps/*.d

libstatham.so: $(OBJS)
	$(CXX) $(LOPTS) $(LDIR) $(LIBS) $(OBJS) -o $@
	-@cp $@ ../lib

$(OBJS):

clean:
	-@rm -rf libstatham.so $(OBJS)

