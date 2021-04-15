
SUBDIRS=statm statm-core statm-msg statm-mpx statm-errinfo statm-ham statm-stat

.PHONY: subdirs $(SUBDIRS)

all clean: subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS):
	cd $@/; $(MAKE) TEST=$(TEST) CONFIGURATION=$(CONFIGURATION) $(MAKECMDGOALS)

statm: statm-core statm-stat

statm-ham: statm-msg statm-mpx statm-stat

statm statm-msg statm-mpx statm-ham statm-stat: statm-errinfo

statm-core: statm-msg statm-mpx

