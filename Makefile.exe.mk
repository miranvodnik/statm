
EXESUBDIRS=xdbd xdbd-test

.PHONY: exesubdirs $(EXESUBDIRS)

executable: exesubdirs

exesubdirs: $(EXESUBDIRS)

$(EXESUBDIRS):
	cd $@/; $(MAKE) TEST=$(TEST) CONFIGURATION=$(CONFIGURATION) $(MAKECMDGOALS)

clean: exesubdirs

