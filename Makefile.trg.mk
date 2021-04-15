
LIBSUBDIRS=xdbd-nmgen-mq xdbd-nmgen-sdl

.PHONY: libsubdirs $(LIBSUBDIRS)

library: libsubdirs

libsubdirs: $(LIBSUBDIRS)

$(LIBSUBDIRS):
	cd $@/; $(MAKE) TEST=$(TEST) CONFIGURATION=$(CONFIGURATION) $(MAKECMDGOALS)

clean: libsubdirs

