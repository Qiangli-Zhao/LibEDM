INCDIR = -I../ -I../classifiers -I../ensembles -I../FileFormats -I../IncrementalEnsembles -I../pruners -I../utilities
LIBS = -lm -lstdc++ -ledm
LIBDIRS = -L../
CFLAG = -O3
CCFLAG = -O3
LDFLAG = -O3
CC = gcc
CCC = g++
TARGETS = dataset classifier classifier1 Ensemble Pruner Pruner1 Statistic CrossValidate IncEnsemble
SUBDIRS = 
MAKEFILE = makefile


.SUFFIXES: .c .cpp .cc

all:$(TARGETS) subdirs

run:
	./dataset
	./classifier
	./classifier1
	./Ensemble
	./Pruner
	./Pruner1
	./Statistic
	./CrossValidate
	./IncEnsemble

subdirs:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making in directory $$dir ;\
			(cd $$dir; make -f $(MAKEFILE)) ;\
		done

dataset:example1.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

classifier:example2.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

classifier1:example2.1.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

Ensemble:Example3.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

Pruner:Example4.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

Pruner1:Example4.1.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

Statistic:example5.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

CrossValidate:example6.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

IncEnsemble:example7.o ../svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

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
