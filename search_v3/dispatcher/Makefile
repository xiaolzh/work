ifeq ($(dd),debug) 
export dd := debug
export BIN_DIR := ./debug/
export INSTALL_FLAG=debug
else  #release
export BIN_DIR := ./release/
export INSTALL_FLAG=release
endif

export DESTDIR=dispatcher_install/

all: build doc

help:
	@echo  'TODO: add help for dispatcher make'

build:
	@echo 'build all executable files'
	$(MAKE) -C libs;
	$(MAKE) -C db_reader;
	$(MAKE) -C business_layer;
	$(MAKE) -C common_data_handler;
	$(MAKE) -C data_handler;
	$(MAKE) -C dispatcher;

doc:
	@echo 'TODO: add doc for dispatcher make'

clean:
	@echo 'clean up the generated files'
	$(MAKE) -C libs clean;
	$(MAKE) -C db_reader clean;
	$(MAKE) -C business_layer clean;
	$(MAKE) -C common_data_handler clean;
	$(MAKE) -C data_handler clean;
	$(MAKE) -C dispatcher clean;

install: install-bin

install-bin:
	@echo 'install dispatcher to $(DESTDIR)'
	cd deploy;python setup.py -p $(DESTDIR) -f $(INSTALL_FLAG);cd ..;

install-doc:
	@echo 'TODO install doc of dispatcher'

.PHONY: help all build doc clean install install-bin install-doc
