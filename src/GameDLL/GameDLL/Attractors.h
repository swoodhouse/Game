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

    BDD representState(const std::vector<bool>& values) const;
    BDD representNonPrimeVariables() const;
    BDD representPrimeVariables() const;
    int countBits(int end) const;
    BDD representUnprimedVarQN(int var, int val) const;
    BDD representPrimedVarQN(int var, int val) const;
    BDD representStateQN(const std::vector<int>& vars, const std::vector<int>& values) const;
    BDD representSyncQNTransitionRelation(const QNTable& qn) const;
    BDD renameRemovingPrimes(const BDD& bdd) const;
    BDD renameAddingPrimes(const BDD& bdd) const;
    BDD randomState(const BDD& S) const;
    void removeInvalidBitCombinations(BDD& S) const;
    BDD immediateSuccessorStates(const BDD& transitionBdd, const BDD& valuesBdd) const;
    BDD forwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const;
    BDD immediatePredecessorStates(const BDD& transitionBdd, const BDD& valuesBdd) const;
    BDD backwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const;
    
    std::list<BDD> attractors(const BDD& transitionBdd,  const BDD& statesToRemove) const;
    std::string prettyPrint(const BDD& attractor) const;

    BDD fixpoints(const BDD& transitionBdd) const;

    // temp
    Attractors() {};

    Attractors(const std::vector<int>& minVals, const std::vector<int>& rangesV, const QNTable& qnT, int numVars)
    {
         std::cout << "in attractors ctor" << std::endl;
      minValues = minVals;
      ranges = rangesV;
      qn = qnT;
      numUnprimedBDDVars = countBits(rangesV.size());
      numBDDVars = numVars;
      manager = Cudd(numBDDVars);
      nonPrimeVariables = representNonPrimeVariables();
      primeVariables = representPrimeVariables();
      
      // new, trying to change this to solve efficency issue on breast cancer model in Game mode
      //manager.AutodynEnable(CUDD_REORDER_GROUP_SIFT); // seems to beat CUDD_REORDER_SIFT
      manager.AutodynEnable(CUDD_REORDER_GROUP_SIFT_CONV); // CUDD_REORDER_GROUP_SIFT_CONV better than GROUP_SIFT it seems, CUDD_REORDER_SYMM_SIFT_COV not as good. CUDD_REORDER_SYMM_SIFT probably not as good. CUDD_REORDER_SIFT_CONVERGE good? maybe not as group_sif_conv. Can I get debugging information for the reordering.. actually, maybe some of these were better than CUDD_REORDER_GROUP_SIFT_CONV
	 // try CUDD_REORDER_GROUP_SIFT_CONV? CUDD_REORDER_SYMM_SIFT_CONV? CUDD_REORDER_SYMM_SIFT? CUDD_REORDER_SIFT_CONVERGE? CUDD_REORDER_SIFT?

	 std::cout << "leaving attractors ctor" << std::endl;
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
