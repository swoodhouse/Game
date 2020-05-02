// Copyright (c) Microsoft Research 2017
// License: MIT. See LICENSE

#pragma once

struct QNTable {
  /*const*/ std::vector<std::vector<int>> inputVars;
  /*const*/ std::vector<std::vector<std::vector<int>>> inputValues;
  /* const*/ std::vector<std::vector<int>> outputValues;

    /* QNTable(std::vector<std::vector<int>>&& inputVarsV, std::vector<std::vector<std::vector<int>>>&& inputValuesV, std::vector<std::vector<int>>&& outputValuesV) : */
    /*      inputVars(std::move(inputVarsV)), inputValues(std::move(inputValuesV)), outputValues(std::move(outputValuesV)) {} */

    QNTable(const std::vector<std::vector<int>>& inputVarsV, const std::vector<std::vector<std::vector<int>>>& inputValuesV, const std::vector<std::vector<int>>& outputValuesV) :
  inputVars(inputVarsV), inputValues(inputValuesV), outputValues(outputValuesV) {} // temp
  
  QNTable() {} // temp
  
    /* QNTable(const QNTable&) = delete; */
    /* QNTable& operator=(const QNTable&) = delete; */
};

struct Attractors {
  /*const*/ std::vector<int> minValues;
  /*const*/ std::vector<int> ranges;
  /*const*/ QNTable qn;
    /*const*/ int numUnprimedBDDVars;
    /*const*/ int numBDDVars;

    Cudd manager;
    /*const*/ BDD nonPrimeVariables;
    /*const*/ BDD primeVariables;

    BDD representState(const std::vector<bool>& values);
    BDD representNonPrimeVariables();
    BDD representPrimeVariables();
    int countBits(int end);
    BDD representUnprimedVarQN(int var, int val);
    BDD representPrimedVarQN(int var, int val);
    BDD representStateQN(const std::vector<int>& vars, const std::vector<int>& values);
    BDD representSyncQNTransitionRelation(const QNTable& qn);
    BDD renameRemovingPrimes(const BDD& bdd);
    BDD renameAddingPrimes(const BDD& bdd);
    BDD randomState(const BDD& S);
    void removeInvalidBitCombinations(BDD& S);
    BDD immediateSuccessorStates(const BDD& transitionBdd, const BDD& valuesBdd);
    BDD forwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd);
    BDD immediatePredecessorStates(const BDD& transitionBdd, const BDD& valuesBdd);
    BDD backwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd);
    
    std::list<BDD> attractors(const BDD& transitionBdd,  const BDD& statesToRemove);
    std::string prettyPrint(const BDD& attractor);

    BDD fixpoints(const BDD& transitionBdd);

    void initialiseLevels(const std::vector<int>& levels);
  
    // temp
    Attractors() {};
  Attractors(const std::vector<int>& minVals, const std::vector<int>& rangesV, const QNTable& qnT, int numVars, const std::vector<int>& levels)
    {
      minValues = minVals;
      ranges = rangesV;
      qn = qnT;
      numUnprimedBDDVars = countBits(rangesV.size());
      numBDDVars = numVars;
      //manager = Cudd(numBDDVars);
	    //manager = Cudd(); // temp
      
            initialiseLevels(levels);
      
      nonPrimeVariables = representNonPrimeVariables();

      std::cout << "printing minterms early.." << std::endl;
      nonPrimeVariables.PrintMinterm();

      primeVariables = representPrimeVariables();

      std::cout << "printing minterms early.." << std::endl;
      primeVariables.PrintMinterm();

      //manager.AutodynEnable(CUDD_REORDER_GROUP_SIFT_CONV); // play with different choices again
     };

// //Option 2: Dynamic reordering by window permutation
// Cudd_AutodynEnable(gbm, CUDD_REORDER_WINDOW2);
// Cudd_ReduceHeap(gbm, CUDD_REORDER_WINDOW2, 3000);

// //Option 5: Dynamic reordering by swapping
// Cudd_AutodynEnable(gbm, CUDD_REORDER_RANDOM);
// Cudd_ReduceHeap(gbm, CUDD_REORDER_RANDOM, 3000);

// //Option 6: No reordering
// Cudd_AutodynDisable (gbm);
  
    /* Attractors(std::vector<int>&& minVals, std::vector<int>&& rangesV, QNTable&& qnT, int numVars) : */
    /* minValues(std::move(minVals)), ranges(std::move(rangesV)), qn(std::move(qnT)), */
    /*   numUnprimedBDDVars(countBits(ranges.size())), */
    /*   numBDDVars(numVars), */
    /*     manager(numBDDVars), */
    /*     nonPrimeVariables(representNonPrimeVariables()), primeVariables(representPrimeVariables()) */
    /* { */
    /*     std::cout << "in attractors ctor" << std::endl; */
    /*     manager.AutodynEnable(CUDD_REORDER_GROUP_SIFT); // seems to beat CUDD_REORDER_SIFT */
    /* }; */

    // temp
    //Attractors(const Attractors&) = delete;
    //Attractors& operator=(const Attractors&) = delete;
};

inline int logTwo(unsigned int i) {
    unsigned int r = 0;
    while (i >>= 1) r++;
    return r;
}

inline int bits(unsigned int i) {
    return i == 0 ? 0 : logTwo(i) + 1;
}

inline bool nthBitSet(int i, int n) {
    return (1 << n) & i;
}

BDD logicalEquivalence(const BDD& a, const BDD& b);

std::string fromBinary(const std::string& bits, int offset);
