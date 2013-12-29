INCDIR = -I../ -I../classifiers -I../ensembles -I../FileFormats -I../IncrementalEnsembles -I../pruners -I../utilities
LIBS = 
LIBDIRS = 
CFLAG = -O3
CCFLAG = -O3
LDFLAG = -O3
CC = gcc
CCC = g++
TARGETS = main
SUBDIRS = 
MAKEFILE = makefile


.SUFFIXES: .c .cpp .cc

all:$(TARGETS) subdirs

subdirs:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making in directory $$dir ;\
			(cd $$dir; make -f $(MAKEFILE)) ;\
		done

main:ArffData.o UCIData.o

clean:
	/bin/rm -f $(TARGETS) *.o
	@for dir in $(SUBDIRS) ;\
		do \
			if [ ! -d $$dir -o ! -s $$dir/$(MAKEFILE) ] ; then \
				continue ; fi ; \
			echo " " ;\
			echo Making clean in directory $$dir ;\
			(cd $$dir; make clean -f $(MAKEFILE)) ;\
		done

.c.o:
	$(CC) $(CFLAG) $(INCDIR) -c $<
.cc.o:
	$(CCC) $(CCFLAG) $(INCDIR) -c $<
.cpp.o:
	$(CCC) $(CCFLAG) $(INCDIR) -c $<
