
-include ../makefile.defs

CFLAGS = -fPIC -Wall -c -g $(UNIX_FLAGS)
IDIR = \
	-I$(PRODUCT_DIR)/statm-include \

CXXFLAGS = $(DEF_FLAGS) $(CFLAGS) $(IDIR)
LDIR = \
	-L$(PRODUCT_DIR)/lib \

LOPTS = -shared
LIBS = 

OBJS = \
	StatmErrInfo.o \
	StatmErrApi.o \

library: libstaterrinfo.so

-include .deps/*.d

libstaterrinfo.so: $(OBJS)
	$(CXX) $(LOPTS) $(LDIR) $(LIBS) $(OBJS) -o $@
	-@cp $@ ../lib

$(OBJS):

clean:
	-@rm -rf libstaterrinfo.so $(OBJS)

