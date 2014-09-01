INCDIR = -I. -I./classifiers -I./ensembles -I./FileFormats -I./IncrementalEnsembles -I./pruners -I./utilities
LIBS = -lm -lstdc++ -ledm
LIBDIRS = -L.
CFLAG = -O3 -shared -fPIC
CCFLAG = -O3 -shared -fPIC
LDFLAG = -O3
CC = gcc
CCC = g++
CLASS_DIR = classifiers
ENSEMBLE_DIR = ensembles
FORMAT_DIR = FileFormats
INCENSEMBLE_DIR = IncrementalEnsembles
UNTILITY_DIR = utilities
PRUNER_DIR = pruners
SRCDIRS = $(CLASS_DIR) $(ENSEMBLE_DIR) $(FORMAT_DIR) $(INCENSEMBLE_DIR) $(UNTILITY_DIR) $(PRUNER_DIR)
TARGETS = libedm.so
SUBDIRS = examples
MAKEFILE = makefile
ALLFILES = Prediction.o Ensemble.o DataSet.o EnsemblePruner.o \
	./$(UNTILITY_DIR)/zString.o ./$(UNTILITY_DIR)/DateTime.o ./$(UNTILITY_DIR)/RandSequence.o ./$(UNTILITY_DIR)/Statistic.o ./$(UNTILITY_DIR)/GA.o \
	./$(CLASS_DIR)/bpnn.o ./$(CLASS_DIR)/C45.o ./$(CLASS_DIR)/NaiveBayes.o ./$(CLASS_DIR)/GaussNaiveBayes.o ./$(CLASS_DIR)/b-svm.o \
	./$(FORMAT_DIR)/ArffData.o ./$(FORMAT_DIR)/UCIData.o \
	./$(ENSEMBLE_DIR)/CustomEnsemble.o ./$(ENSEMBLE_DIR)/Bagging.o ./$(ENSEMBLE_DIR)/AdaBoost.o \
	./$(PRUNER_DIR)/SelectBest.o ./$(PRUNER_DIR)/Gasen.o ./$(PRUNER_DIR)/FS.o ./$(PRUNER_DIR)/OrientOrder.o ./$(PRUNER_DIR)/MDSQ.o ./$(PRUNER_DIR)/cluster.o ./$(PRUNER_DIR)/PMEP.o ./$(PRUNER_DIR)/SelectAll.o \
	./$(INCENSEMBLE_DIR)/ACE.o ./$(INCENSEMBLE_DIR)/AWE.o ./$(INCENSEMBLE_DIR)/FCAE.o ./$(INCENSEMBLE_DIR)/SEA.o ./$(INCENSEMBLE_DIR)/SEA_OP.o ./$(INCENSEMBLE_DIR)/Win.o


.SUFFIXES: .c .cpp .cc

all:srcdirs $(TARGETS) svm.o subdirs

srcdirs:
	@for dir in $(SRCDIRS) ;\
		do \
			echo " " ;\
			echo Making in directory $$dir ;\
			(cd $$dir; make -f $(MAKEFILE)) ;\
		done

libedm.a:$(ALLFILES)
	ar r $@ $^

libedm.so:$(ALLFILES)
	$(CCC) -shared -o $@ $^

subdirs:
	@for dir in $(SUBDIRS) ;\
		do \
			echo " " ;\
			echo Making in directory $$dir ;\
			(cd $$dir; make -f $(MAKEFILE)) ;\
		done

kensm:kensm.o svm.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

clean:
	@for dir in $(SRCDIRS) ;\
		do \
			if [ ! -d $$dir -o ! -s $$dir/$(MAKEFILE) ] ; then \
				continue ; fi ; \
			echo " " ;\
			echo Making clean in directory $$dir ;\
			(cd $$dir; make clean -f $(MAKEFILE)) ;\
		done
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
	$(CC) $(CFLAG) -c $< $(INCDIR)
.cc.o:
	$(CCC) $(CCFLAG) -c $< $(INCDIR)
.cpp.o:
	$(CCC) $(CCFLAG) -c $< $(INCDIR)
