#pragma once
/*
Think about table building. Doing a backwards for every attractor is not feasible. Is it equivalent anyway to symbolic vmcai with constraints over ko and oe vars? But will that constrain enough. You could do one one those per level. Got to experiment really. And work out what the constraints are

Why is it not just normal vmcai with those special vars added? And the corresponding target functions changed. I think it is. Can you do better than that? Yes, added the constraints that at max n are mutated. Actually if we insist on disjoint ko and oe then we probably will be ok.. only one side of range affected. And we only have to worry about a handful of genes exploding. Make sure also that you do not include the ko or genes in the table itself

First version of everything implemented. Certainly bugs
Likely sources of bugs: indices errors, exist abstraction working unexpectedly, ....
Table building and debugging remains then
Other probably source of error: not uniformally having zero mean empty and n+1 mean n
Try Simon test model and test vars
*3. game.. can try a symbolic vmcai over ko oe vars*
*/

//void Game::test() {
//    const Cudd manager;
//    manager.AutodynEnable(CUDD_REORDER_GROUP_SIFT); // who knows what best choice now is
//    ADD a = manager.constant(0.0);
//    ADD a1 = manager.addZero();
//    ADD b = manager.constant(1.0);
//    ADD b1 = manager.addOne();
//    ADD c = manager.constant(0.1);
//
//    std::cout << (a == b);
//
//    ADD f = manager.constant(5.0);
//    //    The following fragment of code illustrates how to build the ADD for the function $f = 5x_0x_1x_2x_3$.
//    for (int i = 3; i >= 0; i--) {
//        ADD var = manager.addVar(i);
//        f *= var;
//    }
//    f.PrintMinterm();
//}

/// what would be good to put into here
//// i think test 1 would be to do ORDER symbolically, and with 0/1 ADDs
//void Game::tempTest() const {
//    const Cudd manager;
//    ADD a = manager.addOne();
//    a.PrintMinterm();
//    manager.DumpDot(std::vector<ADD>(a));
//    // test how +, -, *, max, min are working, and if that is how I expected
//    // ADD UnivAbstract(const ADD& cube), OrAbstract(const ADD& cube), ADD ExistAbstract(const ADD& cube)
//    // FindMax(), FindMin()
//    // void PrintMinterm() const;
//    // then, do a first run of minimax with height = 0, 1, 2
//}
//


// One thing I thought of that the user needs to be able to specify: the function for the score. I assume currently it is just -Apoptosis, but we might want Proliferation-Apoptosis or something. 

// umutate remains, table building remains

// https://github.com/emil-e/rapidcheck


/*
*i think we have a problem in general in that i haven't allowed zero to represent no mutation? well i have in some places but don't think i am doing +1 on vals* also means the range needs to be bits(oeVars.size() + 1) right?
also, treatments need to be kos, not oes. and.. 

*turn off lexicographical ordering and ensure results are unchanged, just duplicated*



// One thing I thought of that the user needs to be able to specify: the function for the score. I assume currently it is just -Apoptosis, but we might want Proliferation-Apoptosis or something. 

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
public: // temp
	const Attractors attractors;
    const int numMutations;
    const int numTreatments;
    const int height;
    const bool maximisingPlayerLast;
    const std::vector<int> koVars;
    const std::vector<int> oeVars;

    
    const BDD mutantTransitionRelation;
    const BDD unmutateRelation; // needed?
    const ADD scoreRelation;

	static int calcNumMutations(int height, bool maximisingPlayerGoesLast);
	static int calcNumTreatments(int height, bool maximisingPlayerGoesLast);

	std::vector<int> attractorsIndicies() const;
	std::vector<int> treatmentVarIndices() const;
	std::vector<int> unprimedMutationVarsIndices() const;
	std::vector<int> primedMutationVarsIndices() const;
	std::vector<int> chosenTreatmentsIndices() const;
	std::vector<int> chosenMutationsIndices() const;

	std::vector<std::vector<int>> unprimedMutationVarsIndicesWindowed() const;
	std::vector<std::vector<int>> primedMutationVarsIndicesWindowed() const;
	/*std::vector<std::vector<int>> chosenTreatmentsIndices() const;
	std::vector<std::vector<int>> chosenMutationsIndices() const;*/

    ADD buildScoreRelation(int apopVar) const; // done
    ADD renameBDDVarsAddingPrimes(const ADD& add) const; // done
    ADD immediateBackMax(const ADD& states) const; // done
    ADD immediateBackMin(const ADD& states) const; // done
    ADD backMin(const ADD& states) const; // done
    ADD backMax(const ADD& states) const; // done
	ADD scoreLoop(const BDD& loop, const ADD& scoreRelation) const;
    ADD scoreAttractors(int numMutations) const; // done.. except taking max not mean
    BDD buildMutantSyncQNTransitionRelation() const; // done.. except no.. i need zero to mean no mutation i think!!!
    BDD representTreatment(int val) const; // done.. including zero mut and bits
	BDD representTreatmentNone() const; //	done
    BDD representMutation(int var, int val) const; // done.. including zero mut and bits
	BDD representMutationNone(int var) const; // done
    BDD representPrimedMutation(int var, int val) const; // done.. including zero mut and bits
	BDD representPrimedMutationNone(int var) const; // done
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

	Game(std::vector<int>&& minVals, std::vector<int>&& rangesV, QNTable&& qn, std::vector<int>&& koVarsV, std::vector<int>&& oeVarsV, int apopVar, int depth,
		bool maximisingPlayerGoesLast) :
      koVars(std::move(koVarsV)), oeVars(std::move(oeVarsV)), attractors(std::move(minVals), std::move(rangesV), std::move(qn)),
      mutantTransitionRelation(buildMutantSyncQNTransitionRelation()),
	scoreRelation(buildScoreRelation(apopVar)),
	height(depth),
	maximisingPlayerLast(maximisingPlayerGoesLast),
	numMutations(calcNumMutations(height, maximisingPlayerGoesLast)), // test these two lines..
	numTreatments(calcNumTreatments(height, maximisingPlayerGoesLast))
    {
    }; // done

    ADD minimax() const; // done
};
