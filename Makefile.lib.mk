
LIBSUBDIRS=xdbd-stat xdbd-core xdbd-sql-parser xdbd-msg xdbd-mpx xdbd-api xdbd-odbc xdbd-nmgen-lib xdbd-nmgen xdbd-sysgen xdbd-errinfo xdbd-ham

.PHONY: libsubdirs $(LIBSUBDIRS) clean

library: libsubdirs

libsubdirs: $(LIBSUBDIRS)

$(LIBSUBDIRS):
	@echo "Directory "$@
	@cd $@/ && $(MAKE) TEST=$(TEST) CONFIGURATION=$(CONFIGURATION) $(MAKECMDGOALS)

xdbd-odbc: xdbd-api xdbd-errinfo

xdbd-sql-parser xdbd-api xdbd-ham: xdbd-core xdbd-msg xdbd-mpx xdbd-errinfo

xdbd-core: xdbd-stat

xdbd-stat: xdbd-msg xdbd-mpx xdbd-errinfo

xdbd-nmgen: xdbd-sysgen

xdbd-sysgen: xdbd-nmgen-lib

clean: libsubdirs

