LibEDM
======

It a C++ library for ensemble based machine learning. It is a toolkit that provides functions covering all facets of ensemble researches, which includes algorithms of data processing, single classifier learners, ensemble methods, ensemble pruning, predicting and statistical tests, etc. This library is intended to be useful in research and is carefully designed to simply work with real-world applications. 

Read Manual.pdf for user instruction. Go into ./examples/ for examples.



Four base classifiers:

1) Back-Propagation Neural Networks (BPNN);

2) C4.5 decision trees;

3) Supported Vector Machine;

4) Naïve Bayes;



Two ensemble:

1) Bagging;

2) AdaBoost;



Three stream based incremental ensemble:

1) SEA;

2) AWE;

3) ACE;



Seven ensemble pruner:

1) Select Best;

2) Forward Selection (FS);

3) GASEN (GA);

4) Oriented Order (OO);

5) Margin Distance Minimization (MDSQ);

6) Pattern Mining based Ensemble Pruning (PMEP);

7) Cluster-based Pruning (CPF);



Supported Data File Formats:

1) C4.5 & CSV;

2) ARFF


Other Functions:

1) Cross validation;

2) Genetic Algorithm;

3) Two statistical methods: The Friedman test &	The Bergmann- Hommel test
