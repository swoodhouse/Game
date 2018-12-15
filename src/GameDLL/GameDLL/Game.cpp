// heavy refactoring needed. an on Attractors.cpp too i think

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

inline BDD logicalImplication(const BDD& a, const BDD& b) {
    return (!a) + b;
}

BDD Game::representNonPrimedMutVars() const {
    BDD bdd = attractors.manager.bddOne();

    int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size());
    int end = i + numMutations * bits(koVars.size()); // off by one???????????
    for (; i < end; i++) {
        BDD var = attractors.manager.bddVar(i);
        bdd *= var;
    }

    return bdd;
}

ADD Game::renameMutVarsRemovingPrimes(const ADD& states) const {
    std::vector<int> permute(Cudd_ReadNodeCount(attractors.manager.getManager()));
    std::iota(permute.begin(), permute.end(), 0);

    int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size());
    int end = i + numMutations * bits(koVars.size()); // off by one???????????
    for (; i < end; i++) {
        permute[i + end] = i;
    }

    return states.Permute(&permute[0]);
}
// maybe do purely at the level of bits?
BDD Game::chooseRelation(int level) const {
    BDD bdd = attractors.manager.bddZero();

    for (int var = 0; var < numMutations; var++) {
        for (int val = 1; val < koVars.size(); val++) {
			BDD unprimedMutVal = representMutation(var, val);
            BDD choice = representChosenMutation(level, val);
			BDD primedMutZero = representPrimedMutation(var, 0); // this needs to be written

            BDD otherPrimedUnchanged = attractors.manager.bddOne();
			for (int var2 = 0; var2 < numMutations; var2++) {
				if (var2 != var) {
					int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size()) + var2 * bits(koVars.size());
					int end = i + bits(koVars.size());
					int offset = numMutations * bits(koVars.size()); // off by one???????????
					for (; i < end; i++) {
						BDD unprimeBit = attractors.manager.bddVar(i);
						BDD primedBit = attractors.manager.bddVar(i + offset);
						otherPrimedUnchanged *= logicalEquivalence(unprimeBit, primedBit);
					}
				}
			}

			bdd += logicalEquivalence(choice, unprimedMutVal * primedMutZero * otherPrimedUnchanged);
		}
    }
    // choice_level = unprimedMut_n  /\ primedMut_n = 0 /\ primedMut_i/=n = unprimedMut_i
    
    return bdd;
}

ADD Game::unmutate(int level, const ADD& states) const {
    ADD add = states * chooseRelation(level).Add();
    add = add.ExistAbstract(representNonPrimedMutVars().Add()); // not sure if this will work. not sure if maxabstract will either.. might have to implement my own exist abstract again here which returns 0 or .. or use clever min/max/negation..
    add = renameMutVarsRemovingPrimes(add);
    return add;
}

void Game::forceMutationLexicographicalOrdering(BDD& S) const {
    BDD ordering = attractors.manager.bddOne();

    int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size());
    int b = bits(koVars.size()); // you actually can use fewer bits that this, as you can represent one fewer choice each time
    for (int m = 0; m < numMutations - 1; m++) {
        BDD var1 = attractors.manager.bddVar(i);
        BDD var2 = attractors.manager.bddVar(i + b);
        S *= logicalImplication(!var1, !var2);

    // as a test of this function print out the indices it goes through

        int stop = i + b - 1;
        int j = i;
        for(; i < stop; i++) {
            BDD varA2 = attractors.manager.bddVar(i + 1);
            BDD varB2 = attractors.manager.bddVar(i + b + 1);

            for (; j < i; j++) {
                BDD varA1 = attractors.manager.bddVar(j);
                BDD varB1 = attractors.manager.bddVar(j + b);
                // this is currently a non-strict ordering, allows equality.
                //S *= logicalImplication(logicalEquivalence(varA1, varB1), logicalImplication(!varA2, !varB2));
                S *= logicalImplication(logicalEquivalence(varA1, varB1), (!varA2) * varB2);
            }
        }
    }

    S *= ordering;
}

void Game::removeInvalidTreatmentBitCombinations(BDD& S) const {
    int b = bits(oeVars.size());
    int theoreticalMax = (1 << b) - 1;

    for (int val = oeVars.size(); val <= theoreticalMax; val++) {
        S *= !representTreatment(val);
    }
}

void Game::removeInvalidMutationBitCombinations(BDD& S) const {
    int b = bits(koVars.size());
    int theoreticalMax = (1 << b) - 1;

    for (int var = 0; var < numMutations; var++) {
        for (int val = koVars.size(); val <= theoreticalMax; val++) {
            S *= !representMutation(var, val);
		}
    }
}

BDD Game::nMutations(int n) const {
    BDD bdd = attractors.manager.bddOne();

    for (int var = 0; var < n; var++) {
        BDD isMutated = !representMutation(var, 0);
        bdd *= isMutated;
    }
    for (std::vector<int>::size_type var = n; var < koVars.size(); var++) {
        BDD isNotMutated = representMutation(var, 0);
        bdd *= isNotMutated;
    }
    return bdd;
}

ADD Game::untreat(int level, const ADD& states) const {
   // i think actually for untreat, since we have only one variable, it can be a permute.
   // then no need for an exist of treatment vars
   // this has the effect of remembering the treatment by storing it in remember@level and removing treatment var
   // also try a multiply-and-exist version that should behave identically

    std::vector<int> permute(Cudd_ReadNodeCount(attractors.manager.getManager()));
    std::iota(permute.begin(), permute.end(), 0);

    int i = attractors.numUnprimedBDDVars * 2; // refactor out
    int j = i + bits(oeVars.size()) + numMutations * 2 * bits(koVars.size()) + level * bits(oeVars.size());

    for (int n = 0; n < bits(oeVars.size()); n++) { // duplication
        permute[n + i] = n + j;
    }

    return states.Permute(&permute[0]);
}

BDD Game::buildMutantSyncQNTransitionRelation() const {
    BDD bdd = attractors.manager.bddOne();

    int k = 0;
    int o = 0;
    
    for (std::vector<int>::size_type v = 0; v < attractors.ranges.size(); v++) {
		if (attractors.ranges[v] > 0) {
			const auto& iVars = attractors.qn.inputVars[v];
			const auto& iValues = attractors.qn.inputValues[v];
			const auto& oValues = attractors.qn.outputValues[v];
			std::vector<BDD> states(attractors.ranges[v] + 1, attractors.manager.bddZero());
			for (long unsigned int i = 0; i < oValues.size(); i++) {
				states[oValues[i]] += attractors.representStateQN(iVars, iValues[i]);
			}

			BDD targetFunction = attractors.manager.bddOne();
             
			for (std::vector<int>::size_type val = 0; val <= attractors.ranges[v]; val++) {
				BDD vPrime = attractors.representPrimedVarQN(v, val);
				targetFunction *= logicalEquivalence(states[val], vPrime);
			}
    
			// assuming koVars and oeVars are disjoint. and sorted. so at some point we need to call sort
			if (koVars[k] == v) {
				BDD isMutated = attractors.manager.bddZero();
				for (int lvl = 0; lvl < numMutations; lvl++) {
					isMutated += representMutation(lvl, v);
				}
				bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 0) * attractors.representUnprimedVarQN(v, 0), targetFunction);
				k++;
				// do i need to also set unprimed........... if you don't, when you run backwards you can unmutate spontanously
				// if you do.. 
			}
			else if (oeVars[o] == v) {
				BDD isTreated = attractors.manager.bddZero();
				isTreated += representTreatment(v);
				int max = *std::max_element(attractors.ranges.begin(), attractors.ranges.end());
				bdd *= isTreated.Ite(attractors.representPrimedVarQN(v, max) * attractors.representUnprimedVarQN(v, max), targetFunction);
				o++;
			}
			else {
				bdd *= targetFunction;
			}
		}
	}
    return bdd;
}

ADD addSetDiffMax(const ADD& a, const ADD& b) {
    return a.OneZeroMaximum(b) * a + b.OneZeroMaximum(a) * b;
    // if a = b = 0 then this gives zero
    // if a > b then a + 0 = a
    // if b > a then 0 + b = b
}

ADD addSetDiffMin(const ADD& a, const ADD& b) {
    return (-a).OneZeroMaximum(-b) * a + (-b).OneZeroMaximum(-a) * b;
    // if -a = -b = 0 then 0
    // if -a > -b then a + 0 = a
    // if -b > -a then 0 + b = b
}

ADD Game::renameAddingPrimes(const ADD& add) const {
    int *permute = new int[Cudd_ReadNodeCount(attractors.manager.getManager())];
    int i = 0;
    for (; i < attractors.numUnprimedBDDVars; i++) {
        permute[i] = i + attractors.numUnprimedBDDVars;
        permute[i + attractors.numUnprimedBDDVars] = i + attractors.numUnprimedBDDVars;
    }

    for (; i < Cudd_ReadNodeCount(attractors.manager.getManager()); i++) {
        permute[i] = i;
    }
    
    ADD r = add.Permute(permute);
    delete[] permute;
    return r;
}

ADD Game::immediateBackMax(const ADD& states) const {
    ADD add = renameAddingPrimes(states); // uses attractors.primeVariables/nonPrime
    add *= mutantTransitionRelation.Add();
    return add.MaxAbstract(attractors.primeVariables.Add());
}

ADD Game::immediateBackMin(const ADD& states) const {
    ADD add = renameAddingPrimes(states); // uses attractors.primeVariables/nonPrime
    add *= mutantTransitionRelation.Add();
    return -((-add).MaxAbstract(attractors.primeVariables.Add()));
}

ADD Game::backMax(const ADD& states) const {
    ADD reachable = attractors.manager.addZero();
    ADD frontier = states;

    while (!frontier.IsZero()) {
        ADD back = immediateBackMax(frontier);
		frontier = addSetDiffMax(back, reachable);
		reachable = reachable.Maximum(back);
    }
    return reachable;
}

ADD Game::backMin(const ADD& states) const {
    // repeatedly do immediatePre then max seems correct. as long as transition relation has koVar' = koVar

    ADD reachable = attractors.manager.addZero();
    ADD frontier = states;

    while (!frontier.IsZero()) {
        ADD back = immediateBackMin(frontier);
        frontier = addSetDiffMin(back, reachable);
        reachable = -((-reachable).Maximum(-back));
    }
    return reachable;
}

ADD Game::scoreAttractors(int numMutations) const {
   ADD states = attractors.manager.addZero();

   BDD treatment = maximisingPlayerLast ? !representTreatment(0) : representTreatment(0);
   BDD initial = nMutations(numMutations) * treatment;

   removeInvalidTreatmentBitCombinations(initial); // refacotr this out.. can be computed once too
   removeInvalidMutationBitCombinations(initial);
   forceMutationLexicographicalOrdering(initial);
   
   std::unordered_set<int> variablesToIgnore {}; // primedMutations, chosenTreatments, chosenMutations, ...

   std::list<BDD> att = attractors.attractors(mutantTransitionRelation, !initial, variablesToIgnore);

   for (const BDD& a : att) {
        ADD max = (a.Add() * scoreRelation).FindMax(); // replace this with iterative mean computation
        ADD scoredLoop = max * a.Add(); // refactor out repeated Add calls
        states += scoredLoop; // this is ok because each state is tagged by its mutations
    }
    return states;
}

// go over comments in github version
ADD Game::minimax() const {
    int height = this->height;
    int numTreatments = this->numTreatments;
    int numMutations = this->numMutations;
    bool maximisingPlayer = this->maximisingPlayerLast;

	// refactor out
	std::unordered_set<int> variablesToIgnore{}; // primedMutations, chosenTreatments, chosenMutations, ...
	 
    ADD states = scoreAttractors(numMutations);

    for (; height > 0; height--) { // do i have an off by one error
        if (maximisingPlayer) {
            numTreatments--;
            states = backMin(states);
            states = untreat(numTreatments, states);

            BDD att = attractors.manager.bddZero();
			BDD initial = nMutations(numMutations) * representTreatment(0); // refactor this duplication away
			removeInvalidTreatmentBitCombinations(initial); // refacotr this out.. can be computed once too
			removeInvalidMutationBitCombinations(initial);
			forceMutationLexicographicalOrdering(initial);

            for (const BDD& a : attractors.attractors(mutantTransitionRelation, !initial, variablesToIgnore)) {
                att += a;
            }
            states *= att.Add(); // what happens here if we have duplicate states? well, chosen mut is retained and tagged so there are no duplicates
			// ok.. so if we keep going like this i guess what we end up with is attractors at the top level, plus chosen muts and treatments from that point and the score they lead to. yeah.. so no need for chooseMaxTreatment. Although there is a need to maintain a list of BDDs if you want to retain attractor information..
        }
        else {
            numMutations--;
            states = backMax(states);
			states = unmutate(numMutations, states);

            BDD att = attractors.manager.bddZero();

			BDD treatment = numTreatments > 0 ? !representTreatment(0) : representTreatment(0); // refactor away duplication
			BDD initial = nMutations(numMutations) * treatment; // refactor this duplication away
			removeInvalidTreatmentBitCombinations(initial); // move all these out to one function
			removeInvalidMutationBitCombinations(initial);
			forceMutationLexicographicalOrdering(initial);
        
            for (const BDD& a : attractors.attractors(mutantTransitionRelation, !initial, variablesToIgnore)) {
                att += a;
            }
            states *= att.Add();
        }

        maximisingPlayer = !maximisingPlayer;
    }
    return states; // we can print the final add which is just combinatorial choicevars mapping to score, as a compact csv like i do with attractors. remember to subtract one from the scores
}

BDD Game::representTreatment(int treatment) const {
    BDD bdd = attractors.manager.bddOne();
    int i = attractors.numUnprimedBDDVars * 2; // refactor out

    int b = bits(oeVars.size());
    for (int n = 0; n < b; n++) { // duplication
        BDD v = attractors.manager.bddVar(i);
    if (!nthBitSet(treatment, n)) {
        v = !v;
    }

    bdd *= v;
    i++;
    }
    
    return bdd;
}

BDD Game::representMutation(int var, int mutation) const {
    BDD bdd = attractors.manager.bddOne();
    
    int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size()) +
            var * bits(koVars.size()); // * 2 to allow space for primed mutation //countBitsMutVar(var); // different for treatment vars.. need to also count mut vars first
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

BDD Game::representPrimedMutation(int var, int mutation) const {
    BDD bdd = attractors.manager.bddOne();
    
    int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size()) + bits(koVars.size()) +
            var * bits(koVars.size());
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

BDD Game::representChosenTreatment(int level, int treatment) const { // in this case var is the val..
    BDD bdd = attractors.manager.bddOne();

    int i = attractors.numUnprimedBDDVars * 2 +
            bits(oeVars.size()) + 
            numMutations * 2 * bits(koVars.size()) +
            level * bits(oeVars.size());
    // * 2 to allow space for primed mutation //countBitsMutVar(var); // different for tre
    
    int b = bits(oeVars.size());

    for (int n = 0; n < b; n++) { // duplication
        BDD v = attractors.manager.bddVar(i);
		if (!nthBitSet(treatment, n)) {
			v = !v;
		}

		bdd *= v;
		i++;
    }
    
    return bdd;
}

// also.. you can definitely save bits
// zero needs to represent no mut. in mut vars, but not here.
BDD Game::representChosenMutation(int level, int mutation) const { // in this case var is the val..
    BDD bdd = attractors.manager.bddOne();
    // refactor out
    int i = attractors.numUnprimedBDDVars * 2 +
            bits(oeVars.size()) +
            numMutations * 2 * bits(koVars.size()) + // * 2 to allow space for primed mutation
            numTreatments * bits(oeVars.size()) +
            level * bits(koVars.size());
    int b = bits(koVars.size());
    for (int n = 0; n < b; n++) { // duplication
        BDD var = attractors.manager.bddVar(i);
		if (!nthBitSet(mutation, n)) {
			var = !var;
		}

		bdd *= var;
		i++;
    }
    
    return bdd;
}

ADD Game::buildScoreRelation(int apopVar) const {
    ADD score = attractors.manager.addZero();

    for (int val = 0; val <= attractors.ranges[apopVar]; val++) { // what if it is a ko/oe var with a range of 0?
        ADD selector = attractors.representUnprimedVarQN(apopVar, val).Add(); // how efficent is conversion, again. should we just rewrite attractors in terms of 0/1 ADDs?
        score = selector.Ite(attractors.manager.constant(val + 1), score); // maybe IteConstant
        // PRETTY SURE IT WILL HAVE TO BE VAL + 1, ZERO IS FOR UNREACHED STATES
    }

    return score;
}
