#pragma once

// to build a combined model from many sub-models, use ITE. f_x'(s) = if m = x then n else f(x)
// namespaces?

class Game {
    const std::vector<int> koVars;
    const std::vector<int> oeVars;
    const std::vector<QNTable> qnT;
    const int height;

    const Attractors attractors;
    const /*ADD*/BDD mutantTransitionRelation; // 0/1 ADD, so really a BDD. maybe represent like that firstly should probably be a BDD, at least in this initial version that favours clarity over speed
    const ADD unmutateRelation; // 0/1 ADD, so really a BDD. maybe represent like that
    const /*BDD*/ADD untreatRelation; // what about untreat.. you always get rid of all so that's easier // for now treat same as mutations, and have stacking treatments. for now.
    const ADD scoreRelation;
    
    ADD buildScoreRelation(int apopVar, int apopRange) const;
    BDD representMutationVar(int var, int val) const;
    BDD representTreatmentVar(int var, int val, int numMutVars) const;
    BDD /*ADD*/ mutantSyncQNTransitionRelation() const; // sync only for now
    BDD invalidMutationBitCombinations(int numMutations) const; // firstly needs to replciate removeInvalidBitCombinations, to remove invalid bit combos.. but also we need to enforce non-repeats and lexigraphical ordering..
                                                // do we do on treatment here too.. rename?
    BDD invalidTreatmentBitCombinations(int numMutations, int numTreatments) const;
    // forceLexicographicalOrder
    BDD/*ADD*/ nMutations(int n) const; // don't want to keep recomputing
    BDD/*ADD*/ nTreatments(int n, int numMutVars) const; // don't want to keep recomputing

    ////////////////////




    /*BDD*/ADD representPrimedMutationVarZero(int var) const; // sets to the earliest chosen var that is zero
    /*BDD*/ADD representChosenMutation(int level) const; // temp
    //BDD representChosenTreatment(int level) const; // temp
    //BDD representChosenMutation(int level, int mutation) const;
    /*BDD*/ADD representChosenTreatment(int level, int treatment) const;
    /*BDD*/ADD buildUntreatRelation() const;

    
    //ADD backMin(const ADD& states) const;
    //ADD backMax(const ADD& states) const;
    ADD back(const ADD& states) const;
    ADD chooseMaxTreatment(const ADD& states, int level) const;
    ADD chooseMinMutation(const ADD& states, int level) const;
    
    //BDD tempAtMostNMutations(int n) const;

public:
    Game(std::vector<int>&& rangesV, std::vector<int>&& koVarsV, std::vector<int>&& oeVarsV,
         std::vector<QNTable>&& qnTables, int depth) : koVars(std::move(koVarsV)), oeVars(std::move(oeVarsV)), height(depth), qnT(qnTables),
        attractors(std::move(rangesV))  //manager(...), mutantTransitionRelation(buildMutantSyncQNTransitionRelation()), unMutateRelation(...), unTreatRelation(...), scoreRelation(buildScoreRelation())
    {
    };

    void computeAttractorsExplict(const std::string& outputPath, const std::string& header) const; // do with #combos attractor computations
    void computeAttractorsSymbolicAll(const std::string& outputPath, const std::string& header) const; // do with one attractor compution
    void computeAttractorsSymbolicN(const std::string& outputPath, const std::string& header, bool maximisingPlayer) const; // do with #height attractor computations
    // atm i have bool maximisingPlayerFirst here, bool maximisingPlayerLast elsewhere...
    
    ADD minimax(int height, bool maximisingPlayerLast) const;
    ADD valueIteration(int height, bool maximisingPlayerLast) const;  
};
