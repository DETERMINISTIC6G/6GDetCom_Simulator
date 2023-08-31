INET ?= ../../inet

all: checkmakefiles
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles:
	cd src && opp_makemake -f --deep -KINET_PROJ=$(INET) -DINET_IMPORT -I'$$(INET_PROJ)/src' -L'$$(INET_PROJ)/src' -l'INET$$(D)'

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

docold:
	doxygen Doxyfile

neddoc:
	cd .. && opp_neddoc deterministic6g/ inet/ -o deterministic6g/public

doc:
	@cd doc/src && $(MAKE)
	@doxygen doxy.cfg
