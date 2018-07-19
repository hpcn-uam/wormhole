SUBDIRS := $(wildcard */.)
SUBDIRSCLEAN=$(addsuffix clean,$(SUBDIRS))

export FLAGS := -O3

all: $(SUBDIRS) 
$(SUBDIRS):%:
	$(MAKE) -C $@

clean: $(SUBDIRSCLEAN)
$(SUBDIRSCLEAN):
	$(MAKE) -C $(dir $@) clean

.PHONY: all $(SUBDIRS) clean $(SUBDIRSCLEAN)