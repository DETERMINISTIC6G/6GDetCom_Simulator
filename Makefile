INET ?= ../../inet

.PHONY: doc

all: checkmakefiles
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles:
	cd src && opp_makemake -f --deep -KINET_PROJ=$(INET) -DINET_IMPORT -I'$$(INET_PROJ)/src' -L'$$(INET_PROJ)/src' -l'INET$$(D)' LDFLAGS += -Wl,--allow-multiple-definition CXXFLAGS += -std=c++17
	
makefiles-so:
	cd src && opp_makemake --make-so -f --deep -KINET_PROJ=$(INET) -DINET_IMPORT -I'$$(INET_PROJ)/src' -L'$$(INET_PROJ)/src' -l'INET$$(D)' LDFLAGS += -Wl,--allow-multiple-definition CXXFLAGS += -std=c++17

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

neddoc:
	cd .. && opp_neddoc --verbose --doxygen deterministic6g/ inet/

doc:
	cd doc/src && ./docker-make html && echo "===> file:$$(pwd)/_build/html/index.html"