INET ?= ../../inet
ZMQ_INCLUDE ?= ../../zeromq-4.3.5/include       # Pfad zu den ZeroMQ Header-Dateien
ZMQ_LIB ?= ../../zeromq-4.3.5/build/lib/Debug             # Pfad zu den ZeroMQ Bibliotheken

CXXFLAGS += -I$(ZMQ_INCLUDE)  # Pfad zu den ZeroMQ-Header-Dateien
LDFLAGS += -L$(ZMQ_LIB) -lzmq

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
	cd src && opp_makemake -f --deep -KINET_PROJ=$(INET) -DINET_IMPORT -I'$$(INET_PROJ)/src' -L'$$(INET_PROJ)/src' -l'INET$$(D)' \
	-I$(ZMQ_INCLUDE) -L$(ZMQ_LIB) -lzmq  # pfade
	
makefiles-so:
	cd src && opp_makemake --make-so -f --deep -KINET_PROJ=$(INET) -DINET_IMPORT -I'$$(INET_PROJ)/src' -L'$$(INET_PROJ)/src' -l'INET$$(D)' \
	-I$(ZMQ_INCLUDE) -L$(ZMQ_LIB) -lzmq    # <-- ZeroMQ Pfade und Lib hinzufügen

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