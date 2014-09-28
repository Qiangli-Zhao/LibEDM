LibEDM is an open-source library developed in C++ and aims to provide a uniform
platform for developing and evaluating ensemble/pruning related algorithms and
methods. It can also serve as a toolkit for applying these techniques to real-
world problems.


Environment Requirements

LibEDM can work with Visual C++ 2005 (VC2003, VC2008 or higher version should 
also work) for MS- Windows, or GNU and Intel C++ for Linux. It should work on 
any platform that supports ISO C++, such as UNIX.


Compiling

For MS-Windows, open libedm.sln in Microsoft Visual C++ 2005 and execute build 
command, then a C++ library, libEDM.lib, will be built in directory 
libedm___Win32_release (the output directory varys depend on your projects 
setting, such as win32 or win64, debug or release); 

For Linux, input ¡°make¡± in libEDM directory, then a C++ library, libEDM.so, 
will be built in the same directory.


Using LibEDM

To use LibEDM, users should add the LibEDM library to their own projects and
make sure all LibEDM source directories have been added to the project's
including paths. The header file paths for each kind of classes are listed
below:

1) /						All base classes of LibEDM.
2) /classifiers				Classes for base classifiers.
3) /ensembles				Classes for ensembles
4) /FileFormats				Classes for supporting different data file formats.
5) /IncrementalEnsembles	Classes for ensembles that can be trained incrementally.
6) /pruners					Classes for ensemble pruners.
7) /utilities				Utility classes for Cross-Validation, statistical
							tests, Genetic Algorithm, string manipulating and
							date-time manipulating.


Header files

To use a class of LibEDM, users should include header files of all depended
class to their source file. For example, if a user want to use class CUCIData,
which is to support the C45-format data files, he should include these LibEDM's
header files into their source file:

	Obj.h			(base class for all)
	zString.h		(for string manipulating)
	DateTime.h		(for date and time manipulating)
	RandSquence.h	(used for creating a random sequence of instances)
	DataSet.h		(base class for all data file format)
	UCIData.h		(definition of class CUCIData)

also the depended header from STD-C++ should be included, so user's source file
should include these files:

	#include <string>
	#include <algorithm>
	#include <iostream>
	#include <fstream>
	#include <istream>
	#include <sstream>
	#include <ctime>
	#include <cstring>
	using namespace std;
	#include "Obj.h"
	#include "zString.h"
	#include "DateTime.h"
	#include "RandSequence.h"
	#include "DataSet.h"
	#include "UCIData.h"
	using namespace libedm;

To know which header files should be included, please reference description of
each class in LibEDM users' manual.


Examples

The best way to learn LibEDM is to examine and use the examples. To compile the 
examples, entering the examples directory and input make command for Linux/UNIX 
(LibEDM must be built before building examples); or opening the workplace file 
examples.sln in MS-VC++ for MS-Windows, then make all projects.

To run an example on Linux, users should add libEDM directory to their library loading 
path, i.e. LD_LIBRARY_PATH, or they can just copy LibEDM.so to /lib or /usr/lib.

Detailed description for these examples can be found in corresponding chapters 
of Users' Manual:

Name of example		Source file			Chapter number in Users' Manual

dataset				example1.cpp		(6.3)Reading and extracting from a data
										file (C45 format).
										
classifier			example2.cpp		(10.5)Training a BPNN from data files,
										then saving it in files.
										
classifier1			example2.1.cpp		(10.5)Restoring a BPNN from archives.

Ensemble			example3.cpp		(12.4)Creating a Bagging ensemble from
										training data files, then using it to
										predict a test data set.
										
Pruner				example4.cpp		(16.9)Creating a Bagging ensemble and
										pruning it through Select-Best method,
										then using the pruned ensemble to
										predict.
										
Pruner1				example4.1.cpp		(16.9)Creating a Bagging ensemble and
										pruning it through Select-Best and MDSQ
										separately. Because both these pruning
										methods can share all base-classifiers¡¯
										prediction results for the validation
										set, so total pruning-time for all these
										methods can be saved.
										
Statistic			example5.cpp		(17.7.4)Statistical comparison of
										multiple classifier methods by Friedman
										test and Bergmann-Hommel test.
										
CrossValidate		example6.cpp		(17.4.3)Testing RPROP, a BP-neural-
										network training method, on an ARFF-
										format data file by using three-folder
										cross-validation.
										
IncEnsemble			example7.cpp		(14.3.3)Training an ACE ensemble
										incrementally and use it to predict.
