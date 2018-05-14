// Copyright (c) Microsoft Research 2017
// License: MIT. See LICENSE

#pragma once

struct QNTable {
    std::vector<std::vector<int>> inputVars;
    std::vector<std::vector<std::vector<int>>> inputValues;
    std::vector<std::vector<int>> outputValues;

    QNTable(std::vector<std::vector<int>>&& inputVarsV, std::vector<std::vector<std::vector<int>>>&& inputValuesV, std::vector<std::vector<int>>&& outputValuesV) :
        inputVars(std::move(inputVarsV)), inputValues(std::move(inputValuesV)), outputValues(std::move(outputValuesV)) {}
};

// all ADDs here are 0/1 ADDs, equivalent to BDDs
// may be better to keep as BDDs and then convert? for now, do that
class Attractors {
    /*const std::vector<int> minValues;*/
    const std::vector<int> ranges;
    //const QNTable qn;
    const BDD nonPrimeVariables;
    const BDD primeVariables;

public: // move this
    const int numUnprimedBDDVars;
    const Cudd manager;

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
    std::list<BDD> attractors(const BDD& transitionBdd, const BDD& statesToRemove) const;
    std::string prettyPrint(const BDD& attractor) const;

//public:
    Attractors(/*std::vector<int>&& minVals,*/ std::vector<int>&& rangesV /*, QNTable&& qnT*/) :
        /*minValues(std::move(minVals)),*/ ranges(std::move(rangesV)),
        numUnprimedBDDVars(countBits(ranges.size())),//minValues.size())),
        manager(numUnprimedBDDVars * 2),
        nonPrimeVariables(representNonPrimeVariables()), primeVariables(representPrimeVariables())
    {
        manager.AutodynEnable(CUDD_REORDER_GROUP_SIFT); // seems to beat CUDD_REORDER_SIFT
    };
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