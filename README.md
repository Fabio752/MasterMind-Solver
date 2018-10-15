# MasterMind-Solver
A MasterMind solver algorithm in C++ which uses two different approaches in order to solve efficiently small combinations and not to memory overflow for very large ones.
For small combinations the algorithm starts with a fixed guess (e.g 0011) and then based on the feedback works out another random combination which fits the requirement and so on.
For longer combinations (up to 5000) the algorithm still works but less efficiently, because it solves the problem with a binary search.
