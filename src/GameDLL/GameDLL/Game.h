#pragma once

// One thing I thought of that the user needs to be able to specify: the function for the score. I assume currently it is just -Apoptosis, but we might want Proliferation-Apoptosis or something.

// also need to be able to have oe and ko on mutation but just ko on treat

// umutate remains, table building remains

// https://github.com/emil-e/rapidcheck


/*
*i think we have a problem in general in that i haven't allowed zero to represent no mutation? well i have in some places but don't think i am doing +1 on vals* also means the range needs to be bits(oeVars.size() + 1) right?
also, treatments need to be kos, not oes. and.. 

*turn off lexicographical ordering and ensure results are unchanged, just duplicated*
this might break unmutate too if you are not careful.. but then we reintersect anyway right
you want to fill from the left hand side

// don't want to have to keep calling this


// WE NEED TO UPDATE THIS
BDD Game::representMutation(int var, int mutation) const {
    BDD bdd = attractors.manager.bddOne();
   
    int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size()); // +
      //var * 2 * bits(koVars.size()); // * 2 to allow space for primed mutation //countBitsMutVar(var); // different for treatment vars.. need to also count mut vars first // but i don't think we interleave.. we have mut 1..n then mut' 1..n
    int b = bits(koVars.size()); // you actually can use fewer bits that this, as you can represent one fewer choice each time
    for (int n = 0; n < b; n++) {
        BDD var = attractors.manager.bddVar(i);
        if (!nthBitSet(mutation, n)) {
            var = !var;
        }
        bdd *= var;
        i++;
    }
   
    return bdd;
}

 */
class Game {
    const int numMutations;
    const int numTreatments;
    const int height;
    const bool maximisingPlayerLast;
    const std::vector<int> koVars;
    const std::vector<int> oeVars;

    const Attractors attractors;
    const BDD mutantTransitionRelation;
    const BDD unmutateRelation; // needed?
    const ADD scoreRelation;

    ADD buildScoreRelation(int apopVar) const; // done
    ADD renameAddingPrimes(const ADD& add) const; // done
    ADD immediateBackMax(const ADD& states) const; // done
    ADD immediateBackMin(const ADD& states) const; // done
    ADD backMin(const ADD& states) const; // done
    ADD backMax(const ADD& states) const; // done
    ADD scoreAttractors(int numMutations) const; // done.. except taking max not mean
    BDD buildMutantSyncQNTransitionRelation() const; // done.. except no.. i need zero to mean no mutation i think!!!
    BDD representTreatment(int val) const; // done.. except no.. i need zero to mean no mutation i think!!!
    BDD representMutation(int var, int val) const; // done.. except zero for no mutation i think!!!!!!!!
    BDD representPrimedMutation(int var, int val) const; // done.. except zero for no mutation i think!!!!!!!!    
    BDD representChosenTreatment(int level, int treatment) const; // done
    BDD representChosenMutation(int level, int mutation) const; // done
    BDD nMutations(int n) const; // done
    ADD untreat(int level, const ADD& states) const; // done
    void removeInvalidTreatmentBitCombinations(BDD& S) const; // done
    void removeInvalidMutationBitCombinations(BDD& S) const; // done
    void forceMutationLexicographicalOrdering(BDD& S) const; // done.. but i suspect bugs
    BDD representNonPrimedMutVars() const; // done
    ADD renameMutVarsRemovingPrimes(const ADD& states) const; // done
    ADD unmutate(int level, const ADD& states) const; // done
    BDD chooseRelation(int level) const; // done but sure there are bugs
  ////////////////////////////////////////////////////////////////////////////////////////



    
    // also.. it seems i had to change Attractors to represent full range not minimum..maximum
    // make sure Attractors::randomState still works, too. It won't - see note in Game.cpp

public:
    Game(std::vector<int>&& rangesV, std::vector<int>&& koVarsV, std::vector<int>&& oeVarsV, int apopVar, QNTable&& qn, int depth, bool maximisingPlayerGoesLast) :
      koVars(std::move(koVarsV)), oeVars(std::move(oeVarsV)), attractors(std::move(rangesV), std::move(qn)),
      mutantTransitionRelation(buildMutantSyncQNTransitionRelation()),
	scoreRelation(buildScoreRelation(apopVar)),
	height(depth),
	maximisingPlayerLast(maximisingPlayerGoesLast),
	numMutations(height % 2 != 0 && !maximisingPlayerGoesLast ? (height / 2) + 1 : (height / 2)), // test these two lines..
	numTreatments(height % 2 != 0 && maximisingPlayerGoesLast ? (height / 2) + 1 : (height / 2))
    {      
    }; // done

    ADD minimax() const; // done
};
