// heavy refactoring needed. an on Attractors.cpp too i think

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"
#include <set>

inline BDD logicalImplication(const BDD& a, const BDD& b) {
    return (!a) + b;
}

int Game::calcNumMutations(int height, bool maximisingPlayerGoesLast) {
	// temp!!!
	return 2;
	//return height % 2 != 0 && !maximisingPlayerGoesLast ? (height / 2) + 1 : (height / 2); // wrong
}

int Game::calcNumTreatments(int height, bool maximisingPlayerGoesLast) { // wrong
	//temp!!
	return 2;
	//return height % 2 != 0 && maximisingPlayerGoesLast ? (height / 2) + 1 : (height / 2);
}

std::vector<int> Game::attractorsIndicies() const {
	std::vector<int> v(attractors.numUnprimedBDDVars * 2);
	std::iota(v.begin(), v.end(), 0);
	return v;
}

std::vector<int> Game::treatmentVarIndices() const {
	std::vector<int> v(bits(oeVars.size() + 1));
	std::iota(v.begin(), v.end(), attractorsIndicies().back() + 1);
	return v;
}

// write second in terms of this
std::vector<std::vector<int>> Game::unprimedMutationVarsIndicesWindowed() const {
	std::vector<std::vector<int>> result;
	int start = treatmentVarIndices().back() + 1;

	for (int i = 0; i < numMutations; i++) {
		std::vector<int> v(koVars.size() + 1);
		std::iota(v.begin(), v.end(), start);
		start = v.back() + 1;
		result.push_back(v);
	}

	return result;
}

std::vector<int> Game::unprimedMutationVarsIndices() const {
	//std::vector<std::vector<int>> result(numMutations);

	//for (int i = 0; i < numMutations; i++) {
	//	std::vector<int> v(koVars.size() + 1);
	//	std::iota(v.begin(), v.end(), treatmentVarIndices().back() + 1);
	//	result.push_back(v);
	//}

	//return result;

	std::vector<int> v(numMutations * bits(koVars.size() + 1));
	std::iota(v.begin(), v.end(), treatmentVarIndices().back() + 1);
	return v;
}

// write second in terms of this. at least test unwindowing this gives the same as the other
std::vector<std::vector<int>> Game::primedMutationVarsIndicesWindowed() const {
	std::vector<std::vector<int>> result;
	int start = unprimedMutationVarsIndices().back() + 1;

	for (int i = 0; i < numMutations; i++) {
		std::vector<int> v(koVars.size() + 1);
		std::iota(v.begin(), v.end(), start);
		start = v.back() + 1;
		result.push_back(v);
	}

	return result;
}


std::vector<int> Game::primedMutationVarsIndices() const {
	//std::vector<std::vector<int>> result(numMutations);

	//for (int i = 0; i < numMutations; i++) {
	//	std::vector<int> v(koVars.size() + 1);
	//	std::iota(v.begin(), v.end(), treatmentVarIndices().back() + 1);
	//	result.push_back(v);
	//}

	//return result;

	std::vector<int> v(numMutations * bits(koVars.size() + 1));
	std::iota(v.begin(), v.end(), unprimedMutationVarsIndices().back() + 1);
	return v;
}

// probably take level as param...
std::vector<int> Game::chosenTreatmentsIndices() const {
	std::vector<int> v(numTreatments * bits(oeVars.size() + 1)); // don't actually need +1 because don't need to represent zero, but easier this way
	std::iota(v.begin(), v.end(), primedMutationVarsIndices().back() + 1);
	return v;
}

std::vector<int> Game::chosenMutationsIndices() const {
	std::vector<int> v(numMutations * bits(koVars.size() + 1)); // don't actually need +1 because don't need to represent zero, but easier this way
	std::iota(v.begin(), v.end(), chosenTreatmentsIndices().back() + 1);
	return v;
}

BDD Game::representNonPrimedMutVars() const {
    BDD bdd = attractors.manager.bddOne();

    for (int i : unprimedMutationVarsIndices()) {
        BDD var = attractors.manager.bddVar(i);
        bdd *= var;
    }

    return bdd;
}

ADD Game::renameMutVarsRemovingPrimes(const ADD& states) const {
    std::vector<int> permute(Cudd_ReadNodeCount(attractors.manager.getManager()));
    std::iota(permute.begin(), permute.end(), 0);

	std::vector<int> primedIndices(primedMutationVarsIndices());
	std::vector<int> unprimedIndices(unprimedMutationVarsIndices());
	
    for (int i = 0; i < primedIndices.size(); i++) {
        permute[primedIndices[i]] = unprimedIndices[i];
    }

    return states.Permute(&permute[0]);
}
// maybe do purely at the level of bits?
BDD Game::chooseRelation(int level) const {
    //BDD bdd = attractors.manager.bddZero();
	BDD bdd = attractors.manager.bddOne();

	const auto unprimedMuts = unprimedMutationVarsIndicesWindowed(); // use const auto more
	const auto primedMuts = unprimedMutationVarsIndicesWindowed();
	BDD choiceMustBeMade = attractors.manager.bddZero();

    for (int var = 0; var < numMutations; var++) {
        for (int val = 0; val < koVars.size(); val++) {
			BDD unprimedMutVal = representMutation(var, val);
            BDD choice = representChosenMutation(level, val);
			choiceMustBeMade += choice;
			BDD primedMutZero = representPrimedMutationNone(var);

            BDD otherPrimedUnchanged = attractors.manager.bddOne();
			for (int var2 = 0; var2 < numMutations; var2++) {
				if (var2 != var) {
					for (int i = 0; i < unprimedMuts[var2].size(); i++) {
						BDD unprimeBit = attractors.manager.bddVar(unprimedMuts[var2][i]);
						BDD primedBit = attractors.manager.bddVar(primedMuts[var2][i]);
						otherPrimedUnchanged *= logicalEquivalence(unprimeBit, primedBit);
					}
				}
				//if (var2 != var) {
				//	int i = treatmentVarIndices().back() + var2 * bits(koVars.size() + 1);
				//	//int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size() + 1) + var2 * bits(koVars.size() + 1);
				//	int end = i + bits(koVars.size() + 1);
				//	int offset = numMutations * bits(koVars.size() + 1); // off by one???????????


				//	/*unprimedMutationVarsIndices
				//	primedMutationVarsIndices*/



				//	for (; i < end; i++) {
				//		BDD unprimeBit = attractors.manager.bddVar(i);
				//		BDD primedBit = attractors.manager.bddVar(i + offset);
				//		otherPrimedUnchanged *= logicalEquivalence(unprimeBit, primedBit);
				//	}
				//}
			}

			//bdd += logicalEquivalence(choice, unprimedMutVal * primedMutZero * otherPrimedUnchanged); // we also need to say that a choice has to be made
			bdd *= logicalEquivalence(unprimedMutVal, choice * primedMutZero * otherPrimedUnchanged); // also should be *=?
		}
    }
    // choice_level = unprimedMut_n  /\ primedMut_n = 0 /\ primedMut_i/=n = unprimedMut_i
    
	bdd *= choiceMustBeMade;

    return bdd;
}

ADD Game::unmutate(int level, const ADD& states) const {
    ADD add = states * chooseRelation(level).Add();
    //add = add.ExistAbstract(representNonPrimedMutVars().Add()); // not sure if this will work. not sure if maxabstract will either.. might have to implement my own exist abstract again here which returns 0 or .. or use clever min/max/negation..
	add = add.MaxAbstract(representNonPrimedMutVars().Add());
    add = renameMutVarsRemovingPrimes(add);
    return add;
}

void Game::forceMutationLexicographicalOrdering(BDD& S) const {
    BDD ordering = attractors.manager.bddOne();

    int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size() + 1);
    int b = bits(koVars.size() + 1); // you actually can use fewer bits that this, as you can represent one fewer choice each time
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
	int b = bits(oeVars.size() + 1);
    int theoreticalMax = (1 << b) - 1;

    for (int val = oeVars.size(); val <= theoreticalMax - 1; val++) { // theoreticalMAx - 1 here because we already use 0 for no treatment
		std::cout << "removing val = " << val << std::endl;
        S *= !representTreatment(val);
    }
}

void Game::removeInvalidMutationBitCombinations(BDD& S) const {
	int b = bits(koVars.size() + 1);
	int theoreticalMax = (1 << b) - 1;

    for (int var = 0; var < numMutations; var++) {
        for (int val = koVars.size(); val <= theoreticalMax - 1; val++) { // theoreticalMAx - 1 here because we already use 0 for no mutation
            S *= !representMutation(var, val);
		}
    }
}

// temp: hard coded for now
BDD Game::nMutations(int n) const {
	if (koVars.size() > 2) {
		std::cout << "nmutations with size(koVars) > 2 not implemented" << std::endl;
		throw std::runtime_error("nmutations with size(koVars) > 2 not implemented");
	}
	else if (n == 0) {
		return representMutationNone(0) * representMutationNone(1);
	}
	else if (n == 1) {
		return (representMutation(0, 0) + representMutation(0, 1)) * representMutationNone(1);
	}
	else if (n == 2) {
		return representMutation(0, 0) * representMutation(1, 1);
	}
	else {
		std::cout << "nmutations > 2 not implemented" << std::endl;
		throw std::runtime_error("nmutations > 2 not implemented");
	}

    //BDD bdd = attractors.manager.bddOne();

    //for (int var = 0; var < n; var++) {
    //    BDD isMutated = !representMutationNone(var);
    //    bdd *= isMutated;
    //}
    //for (std::vector<int>::size_type var = n; var < koVars.size(); var++) {
    //    BDD isNotMutated = representMutationNone(var);
    //    bdd *= isNotMutated;
    //}
    //return bdd;
}

ADD Game::untreat(int level, const ADD& states) const {
   // i think actually for untreat, since we have only one variable, it can be a permute.
   // then no need for an exist of treatment vars
   // this has the effect of remembering the treatment by storing it in remember@level and removing treatment var
   // also try a multiply-and-exist version that should behave identically

    std::vector<int> permute(Cudd_ReadNodeCount(attractors.manager.getManager()));
    std::iota(permute.begin(), permute.end(), 0);

    int i = attractors.numUnprimedBDDVars * 2; // refactor out
    int j = i + bits(oeVars.size() + 1) + numMutations * 2 * bits(koVars.size() + 1) + level * bits(oeVars.size() + 1);

    for (int n = 0; n < bits(oeVars.size() + 1); n++) { // duplication
        permute[n + i] = n + j;
    }

    return states.Permute(&permute[0]);
}


//BDD Attractors::representSyncQNTransitionRelation(const QNTable& qn) const {
//	BDD bdd = manager.bddOne();
//
//	for (int v = 0; v < ranges.size(); v++) {
//		if (ranges[v] > 0) {
//			const auto& iVars = qn.inputVars[v];
//			const auto& iValues = qn.inputValues[v];
//			const auto& oValues = qn.outputValues[v];
//
//			std::vector<BDD> states(ranges[v] + 1, manager.bddZero());
//			for (int i = 0; i < oValues.size(); i++) {
//				states[oValues[i]] += representStateQN(iVars, iValues[i]);
//			}
//			for (int val = 0; val <= ranges[v]; val++) {
//				BDD vPrime = representPrimedVarQN(v, val);
//				bdd *= logicalEquivalence(states[val], vPrime);
//			}
//		}
//	}
//	return bdd;
//}


BDD Game::buildMutantSyncQNTransitionRelation() const {
	// TEMP!!!
	//return attractors.representSyncQNTransitionRelation(attractors.qn);
	/*std::cout << "start of build mutant tr" << std::endl;

	std::cout << "treatmentVarIndices.front()" << treatmentVarIndices().front();
	std::cout << "treatmentVarIndices.back()" << treatmentVarIndices().back();
	std::cout << "unprimedMutationVarsIndices.front()" << unprimedMutationVarsIndices().front();
	std::cout << "unprimedMutationVarsIndices.back()" << unprimedMutationVarsIndices().back();

	*/	

    BDD bdd = attractors.manager.bddOne();

	//std::cout << "buildMutation start Cudd_ReadSize(manager.getManager()): " << Cudd_ReadSize(attractors.manager.getManager()) << std::endl;;

    int k = 0;
    int o = 0;
    
	//std::set<int> oeVarsSet(oeVars.begin(), oeVars.end()); // if this works refacotr
	//std::set<int> koVarsSet(koVars.begin(), koVars.end());

    for (int v = 0; v < attractors.ranges.size(); v++) {
		//std::cout << "buildMutation iterationCudd_ReadSize(manager.getManager()): " << Cudd_ReadSize(attractors.manager.getManager()) << std::endl;;
		if (attractors.ranges[v] > 0) {
			const auto& iVars = attractors.qn.inputVars[v];
			const auto& iValues = attractors.qn.inputValues[v];
			const auto& oValues = attractors.qn.outputValues[v];
			std::vector<BDD> states(attractors.ranges[v] + 1, attractors.manager.bddZero());
			for (int i = 0; i < oValues.size(); i++) {
				states[oValues[i]] += attractors.representStateQN(iVars, iValues[i]);
			}

			BDD targetFunction = attractors.manager.bddOne();
             
			for (std::vector<int>::size_type val = 0; val <= attractors.ranges[v]; val++) {
				BDD vPrime = attractors.representPrimedVarQN(v, val);
				targetFunction *= logicalEquivalence(states[val], vPrime);
			}
    
			// WAIT............... WE ARE ASSUMING SORTED??????????????????????????????????????????????????????
			// assuming koVars and oeVars are disjoint. and sorted. so at some point we need to call sort
			
			// temp!!!!
			//if ((koVarsSet.find(v) != koVarsSet.end())) { //
			if (k < koVars.size() && koVars[k] == v) {
				BDD isMutated = attractors.manager.bddZero();
				
				for (int lvl = 0; lvl < numMutations; lvl++) {
					//isMutated += representMutation(lvl, v);
					isMutated += representMutation(lvl, k);
				}

				// temp!!
			    //BDD isMutated = attractors.manager.bddOne(); // with this it throws an exception..

				//bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 0) * attractors.representUnprimedVarQN(v, 0), targetFunction);
				//bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 0) * attractors.representUnprimedVarQN(v, 0), targetFunction);
				bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 0), targetFunction);
				std::cout << "KO BRANCH  EXECUTED for v =" << v << std::endl;

				//// temp!!
				//std::cout << "v:" << v << std::endl;
				//std::cout << "range(v):" << attractors.ranges[v] << std::endl;
				//bdd *= attractors.representPrimedVarQN(v, 0) * attractors.representUnprimedVarQN(v, 0);
				//bdd *= attractors.representPrimedVarQN(v, 0);
				//bdd *= attractors.representUnprimedVarQN(v, 0); // this is the bit that crashes.. but primed is fine
				//std::cout << "here12" << std::endl;
				//BDD temp = attractors.representUnprimedVarQN(v, 1);
				//std::cout << "here13" << std::endl;
				//bdd *= temp; // temp. also crashes.. wtf

				////bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 0), targetFunction);
				k++;
				// do i need to also set unprimed........... if you don't, when you run backwards you can unmutate spontanously................................
				// if you do.. 
			}
			//else if (oeVarsSet.find(v) != oeVarsSet.end()) { // doesn't help
			else if (o < oeVars.size() && oeVars[o] == v) {
				//BDD isTreated = representTreatment(v);
				BDD isTreated = representTreatment(o);
				int max = attractors.ranges[v];
				//std::cout << "max:" << max << std::endl;
				//bdd *= isTreated.Ite(attractors.representPrimedVarQN(v, max) * attractors.representUnprimedVarQN(v, max), targetFunction);
				bdd *= isTreated.Ite(attractors.representPrimedVarQN(v, max), targetFunction);

				std::cout << "OE BRANCH EXECUTED for v =" << v << std::endl;
				o++;
			}
			else {
				std::cout << "thenormal target function part of tr is built.." << std::endl;
				bdd *= targetFunction;
			}
		}
	}

	/*std::cout << "buildMutation end Cudd_ReadSize(manager.getManager()): " << Cudd_ReadSize(attractors.manager.getManager()) << std::endl;;

	std::cout << "bdd.IsZero" << bdd.IsZero() << std::endl;
	std::cout << "bdd.IsOne" << bdd.IsOne() << std::endl;

	std::cout << "end of build mutant tr" << std::endl;
*/
    return bdd;
}

// OneZeroMaximum is not what we want
ADD binaryMaximum(const ADD& a, const ADD& b) {
	return a.Maximum(b).BddPattern().Add();
}

// we want if a > b then a else 0
ADD addSetDiffMax(const ADD& a, const ADD& b) {
	return binaryMaximum(a, b) * a + binaryMaximum(b, a) * b;
    //return a.OneZeroMaximum(b) * a + b.OneZeroMaximum(a) * b;
    // if a = b = 0 then this gives zero
    // if a > b then a + 0 = a
    // if b > a then 0 + b = b
}

ADD addSetDiffMaxNew(const ADD& a, const ADD& b) {
	// we want..
	return binaryMaximum(a, b) * a + binaryMaximum(b, a) * b;
}

ADD addSetDiffMin(const ADD& a, const ADD& b) {
	return binaryMaximum(-a, -b) * a + binaryMaximum(-b, -a) * b;
    //return (-a).OneZeroMaximum(-b) * a + (-b).OneZeroMaximum(-a) * b;
    // if -a = -b = 0 then 0
    // if -a > -b then a + 0 = a
    // if -b > -a then 0 + b = b
}

ADD Game::renameBDDVarsAddingPrimes(const ADD& add) const {
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
    ADD add = renameBDDVarsAddingPrimes(states); // uses attractors.primeVariables/nonPrime
    add *= mutantTransitionRelation.Add();
    return add.MaxAbstract(attractors.primeVariables.Add());
}

ADD Game::immediateBackMin(const ADD& states) const {
    ADD add = renameBDDVarsAddingPrimes(states); // uses attractors.primeVariables/nonPrime
    add *= mutantTransitionRelation.Add();
    return -((-add).MaxAbstract(attractors.primeVariables.Add()));
}
// hangs.. you could check if reachable == old reachable
//ADD Game::backMax(const ADD& states) const {
//    ADD reachable = attractors.manager.addZero(); // ???
//	//ADD reachable = states;
//    ADD frontier = states;
//
//    while (!frontier.IsZero()) {
//        ADD back = immediateBackMax(frontier);
//		frontier = addSetDiffMax(back, reachable); // this is wrong, we want back - reachable. not this symmetric ooeration
//		reachable = reachable.Maximum(back);
//    }
//    return reachable;
//}


//BDD Attractors::backwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
//	BDD reachable = manager.bddZero();
//	BDD frontier = valuesBdd;
//
//	while (!frontier.IsZero()) {
//		frontier = immediatePredecessorStates(transitionBdd, frontier) * !reachable;
//		reachable += frontier;
//	}
//	return reachable;
//}
//

ADD Game::backMax(const ADD& states) const {
	ADD reachable = attractors.manager.addZero();
	ADD frontier = states;

	while (!frontier.IsZero()) {
		frontier = immediateBackMax(frontier) * (!(reachable.BddPattern())).Add();
		reachable = reachable.Maximum(frontier);
	}
	return reachable;
}

//ADD Game::backMax(const ADD& states) const {
//	ADD reachable = attractors.manager.addZero();
//	ADD frontier = states;
//
//	while (!frontier.IsZero()) {
//		ADD back = immediateBackMax(frontier);
//		reachable = reachable.Maximum(back);
//		frontier = back * (!(reachable.BddPattern())).Add();
//	}
//	return reachable;
//}
//
//ADD Game::backMax(const ADD& states) const {
//	ADD reachable = states;
//	ADD back = attractors.manager.addZero();
//	while (back != reachable) { // ah.. set diff
//	    back = immediateBackMax(reachable);
//		reachable = reachable.Maximum(back);
//	}
//	return reachable;
//}

//// hacky
//ADD Game::backMax(const ADD& states) const {
//	std::cout << "here6.1" << std::endl;
//	ADD max = states.FindMax();
//	std::cout << "here6.2" << std::endl;
//	BDD reachable = states.BddPattern(); // does this help?
//	std::cout << "here6.3" << std::endl;
//	BDD frontier = reachable;
//	std::cout << "here6.4" << std::endl;
//	while (!frontier.IsZero()) {
//		std::cout << "here6.4.1" << std::endl;
//		BDD frontier = attractors.immediatePredecessorStates(mutantTransitionRelation, frontier) * !reachable;
//		std::cout << "here6.5" << std::endl;
//		reachable += frontier;
//		std::cout << "here6.6" << std::endl;
//	}
//	std::cout << "here6.7" << std::endl;
//	return reachable.Add() * max;
//}


//// eventually want to replace with immediateForwardMean
//ADD Game::immediateForwardMaxOnLoop(const ADD& states) const {
//	ADD add = states * mutantTransitionRelation.Add();
//	add = add.MaxAbstract(attractors.nonPrimeVariables.Add());
//	return renameBDDVarsAddingPrimes(add); // uses attractors.primeVariables/nonPrime
//}
//
//// eventually want forwardMean instead
//ADD Game::forwardMaxOnLoop(const ADD& states) const {
//	
//}
////
//
//BDD Attractors::forwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
//	BDD reachable = manager.bddZero();
//	BDD frontier = valuesBdd;
//
//	while (!frontier.IsZero()) {
//		frontier = immediateSuccessorStates(transitionBdd, frontier) * !reachable;
//		reachable += frontier;
//	}
//	return reachable;
//}


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

ADD Game::scoreLoop(const BDD& loop, const ADD& scoreRelation) const {
	ADD a = loop.Add();
	ADD max = (a * scoreRelation).FindMax();
	return max * a;
}

ADD Game::scoreAttractors(bool maximisingPlayer, int numMutations) const {
	//std::cout << "in scoreAttractors. treatmentVar indices:" << std::endl;
	//for (auto i : this->treatmentVarIndices()) std::cout << i << " ";
	//std::cout << std::endl;

	//std::cout << "attractorsIndicies():" << std::endl;
	//for (auto i : this->attractorsIndicies()) std::cout << i << " ";
	//std::cout << std::endl;

	//std::cout << "attractors.ranges:" << std::endl;
	//for (auto i : this->attractors.ranges) std::cout << i << " ";
	//std::cout << std::endl;

   ADD states = attractors.manager.addZero();

   //BDD treatment = maximisingPlayerLast ? !representTreatmentNone() : representTreatmentNone();
   //BDD initial = attractors.manager.bddOne();// temp
   //BDD initial = representTreatmentNone() * nMutations(0);// temp
  // BDD initial = !representTreatmentNone() * nMutations(0);// temp
   //BDD initial = !representTreatmentNone() * nMutations(2);// temp

   //std::cout << "representTreatmentNone():" << std::endl;
   //representTreatmentNone().PrintMinterm();
   //std::cout << "nMutations(0):" << std::endl;
   //nMutations(0).PrintMinterm();
   //std::cout << "nMutations(1):" << std::endl;
   //nMutations(1).PrintMinterm();
   //std::cout << "nMutations(2):" << std::endl;
   //nMutations(2).PrintMinterm();
   //std::cout << "representMutationNone(0):" << std::endl;
   //representMutationNone(0).PrintMinterm();
   //std::cout << "!representMutationNone(0):" << std::endl;
   //(!representMutationNone(0)).PrintMinterm();
   //std::cout << "representMutationNone(1):" << std::endl;
   //representMutationNone(1).PrintMinterm();
   //std::cout << "!representMutationNone(1):" << std::endl;
   //(!representMutationNone(1)).PrintMinterm();
   //std::cout << "representMutationNone(2):" << std::endl;
   //representMutationNone(2).PrintMinterm();
   //std::cout << "!representMutationNone(2):" << std::endl;
   //(!representMutationNone(2)).PrintMinterm();

   /*std::cout << "alternate nMutations(0):" << std::endl;
   (representMutationNone(0) * representMutationNone(1)).PrintMinterm();
   std::cout << "alternate nMutations(1):" << std::endl;
   ((!representMutationNone(0) * representMutationNone(1)) + (representMutationNone(0) * !representMutationNone(1))).PrintMinterm();
   std::cout << "alternate nMutations(2):" << std::endl;
   (!representMutationNone(0) * !representMutationNone(1)).PrintMinterm();

   std::cout << "another alternate nMutations(1):" << std::endl;
   ((!representMutationNone(0) * representMutationNone(1))).PrintMinterm();*/
   

   //need to check remove invalid too... best is to hard code it:
   //std::cout << "another alternate nMutations(1):" << std::endl;
   //(((representMutation(0, 0) + representMutation(0, 1)) * representMutationNone(1))).PrintMinterm();
   //std::cout << "another alternate nMutations(2):" << std::endl;
   //(representMutation(0, 0) * representMutation(1, 1)).PrintMinterm(); // defintely wrong.......
			//														   // go with these^^^^^^^^^^^^^^^^^^^^


   //std::cout << "(representMutation(0, 0)):" << std::endl;
   //(representMutation(0, 0)).PrintMinterm();
   //std::cout << "(representMutation(0, 1)):" << std::endl;
   //(representMutation(0, 1)).PrintMinterm();

   //std::cout << "(representMutation(1, 0)):" << std::endl;
   //(representMutation(1, 0)).PrintMinterm();

   //std::cout << "(representMutation(1, 1)):" << std::endl;
   //(representMutation(1, 1)).PrintMinterm();

   //std::cout << ".." << std::endl;
   //
   //BDD initial = representTreatmentNone() * nMutations(2);// temp
   //BDD initial = representTreatment(0) * nMutations(2);// temp

   BDD initial = attractors.manager.bddOne();

   if (maximisingPlayer) {
	   initial = representSomeTreatment() * nMutations(numMutations);// temp
   }
   else {
	   initial = representTreatmentNone() * nMutations(numMutations);// temp
   }
   
   //initial.PrintMinterm();
   //std::cout << "numMutations:" << numMutations << std::endl;
   //BDD initial = nMutations(numMutations) * treatment;

   // temp. need this
   //removeInvalidTreatmentBitCombinations(initial); // refacotr this out.. can be computed once too
   //std::cout << "initial after remvoing invalid treatments:" << initial.FactoredFormString() << std::endl;
   //initial.PrintMinterm();
   //removeInvalidMutationBitCombinations(initial); // temp
   //forceMutationLexicographicalOrdering(initial);


   // ***************************
   // print out removeInvalidTreatmentBitCombinations and !representTreatment(val);

   //std::cout << "representTreatmentNone()" << representTreatmentNone().FactoredFormString() << std::endl;
   //representTreatmentNone().PrintMinterm();
   //for (int i = 0; i <= 2; i++) {
	  // std::cout << "representTreatment(" << i << "):" << representTreatment(i).FactoredFormString() << std::endl;
	  // representTreatment(i).PrintMinterm();
   //}

   //
   // TODO: variables to keep implementation breaks this. each BDD now represents N attractors.
   // but.. iterative max computation would work
   std::list<BDD> att = attractors.attractors(mutantTransitionRelation, !initial/*, attractors.manager.bddOne()*/);
   //std::list<BDD> att = attractors.attractors(mutantTransitionRelation, !initial, initial);
   //std::list<BDD> att = attractors.attractors(mutantTransitionRelation, attractors.manager.bddZero(), attractors.manager.bddOne());

   std::ofstream file("Attractors.csv");
   for (const BDD& a : att) {
	   // existmax out bdd vars, leaving just mutvars
	   // call findmax
	  // std::cout << "attractor" << std::endl;
	   states += scoreLoop(a, scoreRelation);
	   file << attractors.prettyPrint(a) << std::endl;
   }

   std::ofstream file2("scoredAttractors.csv");
   file2 << attractors.prettyPrint(states.BddPattern()) << std::endl;

   return states;
}



// go over comments in github version
ADD Game::minimax() const {
	std::cout << "\n\n\nin minimax" << std::endl;
    int height = this->height;
    int numTreatments = this->numTreatments;
    int numMutations = this->numMutations;
    bool maximisingPlayer = this->maximisingPlayerLast;
	 
    ADD states = scoreAttractors(maximisingPlayer, numMutations);
	std::cout << "states is zero?" << states.IsZero();


    for (; height > 0; height--) { // do i have an off by one error
        if (maximisingPlayer) {
            numTreatments--;
			std::cout << "numTreatments:" << numTreatments << std::endl;
            //states = backMin(states);
			states = backMax(states); // backmax will work for sync networks
			std::cout << "states is zero?" << states.IsZero();
			states = untreat(numTreatments, states);
			std::cout << "states is zero?" << states.IsZero();

            BDD att = attractors.manager.bddZero();
			BDD initial = nMutations(numMutations) * representTreatmentNone(); // refactor this duplication away
			BDD variablesToAdd = initial; // ?
			removeInvalidTreatmentBitCombinations(initial); // refacotr this out.. can be computed once too
			removeInvalidMutationBitCombinations(initial);
			//forceMutationLexicographicalOrdering(initial); // temp

            for (const BDD& a : attractors.attractors(mutantTransitionRelation, !initial/*, variablesToAdd*/)) {
                att += a;
				std::cout << "attractor" << std::endl;
            }
            states *= att.Add(); // what happens here if we have duplicate states? well, chosen mut is retained and tagged so there are no duplicates

			std::cout << "states is zero?" << states.IsZero();

			// ok.. so if we keep going like this i guess what we end up with is attractors at the top level, plus chosen muts and treatments from that point and the score they lead to. yeah.. so no need for chooseMaxTreatment. Although there is a need to maintain a list of BDDs if you want to retain attractor information..
        }
        else {
            numMutations--;
			std::cout << "numMutations" << numMutations << std::endl;
            states = backMax(states);
			states = unmutate(numMutations, states);

            BDD att = attractors.manager.bddZero();

			BDD treatment = numTreatments > 0 ? !representTreatmentNone() : representTreatmentNone(); // refactor away duplication
			BDD initial = nMutations(numMutations) * treatment; // refactor this duplication away
			BDD variablesToAdd = initial;
			removeInvalidTreatmentBitCombinations(initial); // move all these out to one function
			removeInvalidMutationBitCombinations(initial);
			//forceMutationLexicographicalOrdering(initial);

            for (const BDD& a : attractors.attractors(mutantTransitionRelation, !initial/*, variablesToAdd*/)) {
                att += a;
            }
            states *= att.Add();
        }

        maximisingPlayer = !maximisingPlayer;
    }
    return states; // we can print the final add which is just combinatorial choicevars mapping to score, as a compact csv like i do with attractors. remember to subtract one from the scores
}

BDD Game::representTreatment(int treatment) const {
	treatment++; // 0 represents no treatment, so n is represented by n+1
    BDD bdd = attractors.manager.bddOne();
    int i = attractors.numUnprimedBDDVars * 2; // refactor out

    int b = bits(oeVars.size() + 1); // + 1 so we can represent no mutation too
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

BDD Game::representTreatmentNone() const {
	return representTreatment(-1); // not the most clear implementation
}

BDD Game::representSomeTreatment() const {
	BDD bdd = attractors.manager.bddZero();

	for (int i = 0; i < oeVars.size(); i++) {
		bdd += representTreatment(i);
	}

	return bdd;
}

BDD Game::representMutation(int var, int mutation) const {
	//mutation++;  // 0 represents no mutation, so n is represented by n+1
 //   BDD bdd = attractors.manager.bddOne();
 //   
	//std::vector<int> indices(unprimedMutationVarsIndices());
	//
	//for (int i = 0; i < indices.size(); i++) {
 //       BDD var = attractors.manager.bddVar(indices[i]);
 //       if (!nthBitSet(mutation, i)) {
 //           var = !var;
 //       }
 //       bdd *= var;
 //   }
 //   
 //   return bdd;
	mutation++;  // 0 represents no mutation, so n is represented by n+1
	BDD bdd = attractors.manager.bddOne();

	int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size() + 1) +
		var * bits(koVars.size() + 1);
	int b = bits(koVars.size() + 1); // + 1 so we can represent no mutation too // you actually can use fewer bits that this, as you can represent one fewer choice each time
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

BDD Game::representMutationNone(int var) const {
	return representMutation(var, -1); // not the most clear implementation
}

BDD Game::representPrimedMutation(int var, int mutation) const {
	mutation++;  // 0 represents no mutation, so n is represented by n+1
    BDD bdd = attractors.manager.bddOne();
    
    int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size() + 1) + numMutations * bits(koVars.size() + 1) +
            var * bits(koVars.size() + 1);

    int b = bits(koVars.size() + 1); // + 1 so we can represent no mutation too // you actually can use fewer bits that this, as you can represent one fewer choice each time // you actually can use fewer bits that this, as you can represent one fewer choice each time
	
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

BDD Game::representPrimedMutationNone(int var) const {
	return representPrimedMutation(var, -1); // not the most clear implementation
}

BDD Game::representChosenTreatment(int level, int treatment) const { // in this case var is the val..
	treatment++;  // 0 represents no treatment, so n is represented by n+1. not actually required for choice vars as we don't allow zero, but easier to do this way for symmetry
    BDD bdd = attractors.manager.bddOne();

    int i = attractors.numUnprimedBDDVars * 2 +
            bits(oeVars.size() + 1) + 
            numMutations * 2 * bits(koVars.size() + 1) +
            level * bits(oeVars.size() + 1);
    // * 2 to allow space for primed mutation //countBitsMutVar(var); // different for tre
    
    int b = bits(oeVars.size() + 1); // don't actually need +1 because don't need to represent zero, but easier this way

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
	mutation++;  // 0 represents no mutation, so n is represented by n+1. not actually required for choice vars as we don't allow zero, but easier to do this way for symmetry
	BDD bdd = attractors.manager.bddOne();
    // refactor out
    int i = attractors.numUnprimedBDDVars * 2 +
            bits(oeVars.size()) +
            numMutations * 2 * bits(koVars.size()) + // * 2 to allow space for primed mutation
            numTreatments * bits(oeVars.size()) +
            level * bits(koVars.size());
    int b = bits(koVars.size() + 1); // again, don't need + 1 but easier this way for symmetry with other classes of variables
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
	/*std::cout << "buildScoreRelation start Cudd_ReadSize(manager.getManager()): " << Cudd_ReadSize(attractors.manager.getManager()) << std::endl;;
*/
    ADD score = attractors.manager.addZero();

    for (int val = 0; val <= attractors.ranges[apopVar]; val++) { // what if it is a ko/oe var with a range of 0?
        ADD selector = attractors.representUnprimedVarQN(apopVar, val).Add(); // how efficent is conversion, again. should we just rewrite attractors in terms of 0/1 ADDs?
        score = selector.Ite(attractors.manager.constant(val + 1), score); // maybe IteConstant
        // PRETTY SURE IT WILL HAVE TO BE VAL + 1, ZERO IS FOR UNREACHED STATES
    }

	//std::cout << "buildScoreRelation end Cudd_ReadSize(manager.getManager()): " << Cudd_ReadSize(attractors.manager.getManager()) << std::endl;;

    return score;
}
