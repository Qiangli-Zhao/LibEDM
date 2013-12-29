INCDIR = -I../../
LIBS = -lm -lstdc++ -lep
LIBDIRS = -L../../
CFLAG = -O3 -DUNIX -DLOG_TRAINING
CCFLAG = -O3 -DUNIX -DLOG_TRAINING
LDFLAG = -O3
CC = gcc
CCC = g++
TARGETS = EnsTrain EnsPredict
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

#EnsTrain:	../../bpnn.o ../../C45.o ../../NaiveBayes.o ../../DataFile.o ../../EnsPruner.o ../../result.o ../../Statistic.o ../../BagGener.o ../../Ensemble.o ../../FS.o EnsTrain.o
EnsTrain:EnsTrain.o
	$(CCC) $(LDFLAG) -o $@ $^ $(LIBDIRS) $(LIBS)

#EnsPredict:	../../bpnn.o ../../C45.o ../../NaiveBayes.o ../../DataFile.o ../../EnsPruner.o ../../result.o ../../Statistic.o ../../BagGener.o ../../Ensemble.o ../../FS.o EnsPredict.o
EnsPredict:EnsPredict.o
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
	$(CC) $(CFLAG) -c $< $(INCDIR)
.cc.o:
	$(CCC) $(CCFLAG) -c $< $(INCDIR)
.cpp.o:
	$(CCC) $(CCFLAG) -c $< $(INCDIR)
