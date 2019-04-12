// heavy refactoring needed. an on Attractors.cpp too i think

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

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

// RENAME.......
std::vector<int> Game::mutationVarsIndices() const {
	std::vector<int> v(numMutations * bits(koVars.size() + 1));
	std::iota(v.begin(), v.end(), treatmentVarIndices().back() + 1);
	return v;
}

// probably take level as param...
std::vector<int> Game::chosenTreatmentsIndices() const {
	std::vector<int> v(numTreatments * bits(oeVars.size() + 1)); // don't actually need +1 because don't need to represent zero, but easier this way
	std::iota(v.begin(), v.end(), mutationVarsIndices().back() + 1);
	return v;
}

std::vector<int> Game::chosenMutationsIndices() const {
	std::vector<int> v(numMutations * bits(koVars.size() + 1)); // don't actually need +1 because don't need to represent zero, but easier this way
	std::iota(v.begin(), v.end(), chosenTreatmentsIndices().back() + 1);
	return v;
}

BDD Game::representNonPrimedMutVars() const {
	BDD bdd = attractors.manager.bddOne();

	for (int i : mutationVarsIndices()) {
		BDD var = attractors.manager.bddVar(i);
		bdd *= var;
	}

	return bdd;
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
		// this won't work for current unmutate implementation
		//return (representMutation(0, 0) + representMutation(0, 1)) * representMutationNone(1);
		return ((representMutation(0, 0) + representMutation(0, 1)) * representMutationNone(1)) +
			((representMutation(1, 0) + representMutation(1, 1)) * representMutationNone(0));
	}
	else if (n == 2) {
		// this won't work for current unmutate implementation
		//return representMutation(0, 0) * representMutation(1, 1);
		return (representMutation(0, 0) * representMutation(1, 1)) +
			(representMutation(0, 1) * representMutation(1, 0));
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
	int j = i + bits(oeVars.size() + 1) + numMutations * bits(koVars.size() + 1) + level * bits(oeVars.size() + 1);

	for (int n = 0; n < bits(oeVars.size() + 1); n++) { // duplication
		permute[n + i] = n + j;
	}

	return states.Permute(&permute[0]);
}

// alternate unmutate, just copying untreat for now. ideally you would use indices() functions.
ADD Game::unmutate(int level, const ADD& states) const {
	std::vector<int> permute(Cudd_ReadNodeCount(attractors.manager.getManager()));
	std::iota(permute.begin(), permute.end(), 0);

	int i = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size() + 1) + level * bits(koVars.size() + 1); // is this correct?
	int j = attractors.numUnprimedBDDVars * 2 + bits(oeVars.size() + 1) + numMutations * bits(koVars.size() + 1) + numTreatments * bits(oeVars.size() + 1) + level * bits(koVars.size() + 1); // is this correct?
																																																  // * 2 will need to be removed when I remove primed muts
																																																  // THIS NEEDS TO CHANGE WHEN WE REMOVE PRIMED// THIS NEEDS TO CHANGE WHEN WE REMOVE PRIMED
	for (int n = 0; n < bits(koVars.size() + 1); n++) { // duplication
		permute[n + i] = n + j;
	}

	return states.Permute(&permute[0]);
}

BDD Game::buildMutantSyncQNTransitionRelation() const {
	BDD bdd = attractors.manager.bddOne();

	int k = 0;
	int o = 0;

	for (int v = 0; v < attractors.ranges.size(); v++) {
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

			// assuming koVars and oeVars are disjoint. and sorted. we call sort in entry point

			if (k < koVars.size() && koVars[k] == v) {
				BDD isMutated = attractors.manager.bddZero();

				for (int lvl = 0; lvl < numMutations; lvl++) {
					isMutated += representMutation(lvl, k);
				}
				int max = attractors.ranges[v];
				bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, max), targetFunction);
				std::cout << "MUTATION BRANCH  EXECUTED for v =" << v << std::endl;
				k++;
			}
			else if (o < oeVars.size() && oeVars[o] == v) { // rename to treat vars - koing not oe-ing
				BDD isTreated = representTreatment(o);
				bdd *= isTreated.Ite(attractors.representPrimedVarQN(v, 0), targetFunction);
				std::cout << "TREATMENT BRANCH EXECUTED for v =" << v << std::endl;
				o++;
			}
			else {
				std::cout << "thenormal target function part of tr is built.." << std::endl;
				bdd *= targetFunction;
			}
		}
	}

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

ADD Game::backMax(const ADD& states) const {
	ADD reachable = attractors.manager.addZero();
	ADD frontier = states;

	while (!frontier.IsZero()) {
		frontier = immediateBackMax(frontier) * (!(reachable.BddPattern())).Add();
		reachable = reachable.Maximum(frontier);
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

ADD Game::scoreLoop(const BDD& loop, const ADD& scoreRelation) const {
	ADD a = loop.Add();
	ADD max = (a * scoreRelation).FindMax();
	return max * a;
}

std::string Game::prettyPrint(const ADD& states) const {
	// ideally would not use a temp file
	FILE *old = attractors.manager.ReadStdout();
	FILE *fp = fopen("temp_game.txt", "w");
	attractors.manager.SetStdout(fp);
	states.PrintMinterm();
	attractors.manager.SetStdout(old);
	fclose(fp);

	std::string out;
	std::ifstream infile("temp_game.txt");
	std::string line;
	auto lambda = [](const std::string& a, const std::string& b) { return a + "," + b; };
	while (std::getline(infile, line)) {
		std::list<std::string> output;
		int i = 0;
		for (int v = 0; v < attractors.ranges.size(); v++) {
			int b = bits(attractors.ranges[v]);
			output.push_back(fromBinary(line.substr(i, b), attractors.minValues[v]));
			i += b;
		}

		i = chosenTreatmentsIndices().front();
		for (int v = 0; v < numTreatments; v++) {
			int b = bits(oeVars.size() + 1);
			auto val = fromBinary(line.substr(i, b), 0); //-1; // TEMP! we have to subtract one in current encoding
			output.push_back(val);
			i += b;
		}

		i = chosenMutationsIndices().front();
		for (int v = 0; v < numMutations; v++) {
			int b = bits(koVars.size() + 1);
			auto val = fromBinary(line.substr(i, b), 0); // -1; // TEMP! we have to subtract one in current encoding
			output.push_back(val);
			i += b;
		}

		out += std::accumulate(std::next(output.begin()), output.end(), output.front(), lambda) + "\n";
	}

	return out;
}

ADD Game::scoreAttractors(bool applyTreatments, int numMutations) const {
	ADD states = attractors.manager.addZero();
	BDD treatment = applyTreatments ? representSomeTreatment() : representTreatmentNone();
	BDD mutsAndTreats = treatment * nMutations(numMutations);
	// temp. need this
	//removeInvalidTreatmentBitCombinations(mutsAndTreats); // refacotr this out.. can be computed once too
	//removeInvalidMutationBitCombinations(mutsAndTreats); // temp
	//forceMutationLexicographicalOrdering(initial); // temp
	std::string header = "";// temp

	BDD statesToRemove = !mutsAndTreats;

	//std::cout << "hereA" << std::endl;
	std::list<BDD> loops = attractors.attractors(mutantTransitionRelation, statesToRemove);
	//std::cout << "hereB" << std::endl;

	int i = 0;
	for (const BDD& a : loops) { // loop here and writing to same file is not right.
		ADD scored = scoreLoop(a, scoreRelation);
		//std::ofstream file("LoopAttractor" + std::to_string(i) + ".csv");
		//file << header << std::endl;
		//file << prettyPrint(scored) << std::endl; // TODO: this must have an indexing bug, it throws an exception
		//file << attractors.prettyPrint(states.BddPattern()) << std::endl; // temp
		states += scored;
		i++;
	}

	return states;
}

BDD Game::representTreatmentVariables() const {
	BDD bdd = attractors.manager.bddOne();
	for (auto i : treatmentVarIndices()) {
		BDD var = attractors.manager.bddVar(i);
		bdd *= var;
	}
	return bdd;
}

ADD Game::minimax() const {
	std::cout << "\n\n\nin minimax" << std::endl;
	int height = this->height;
	int numTreatments = this->numTreatments;
	int numMutations = this->numMutations;
	bool maximisingPlayer = this->maximisingPlayerLast;

	maximisingPlayer = false; // temp

	std::cout << "height:" << height << std::endl;
	std::cout << "treatment?" << false << std::endl;
	std::cout << "numMutations" << numMutations << std::endl;
	ADD states = scoreAttractors(false, numMutations);
	height--;
	
	for (; height > 0; height--) { // do i have an off by one error
		std::cout << "height:" << height << std::endl;
		if (maximisingPlayer) {
			numTreatments--; // HERE OR BEFORE?
			std::cout << "treatment?" << false << std::endl;
			std::cout << "numMutations" << numMutations << std::endl;
			//states = backMin(states);
			states = backMax(states); // backmax will work for sync networks
			states = untreat(numTreatments, states);			
			BDD att = scoreAttractors(false, numMutations).BddPattern(); //THIS MAY HAVE BEEN A BUG // to score then unscore is not ideal
			states *= att.Add();
		}
		else {
			numMutations--; // here or after unmutate? here
			std::cout << "treatment?" << true << std::endl;
			std::cout << "numMutations" << numMutations << std::endl;
			states = backMax(states);
			states = unmutate(numMutations, states);
			BDD att = scoreAttractors(true, numMutations).BddPattern(); // THIS MAY HAVE BEEN A BUG // to score then unscore is not ideal
			states = states.MaxAbstract(representTreatmentVariables().Add()) * att.Add(); // score again here?? run backrward and intersect with attractors in between adding treatment and removing mut?
		}

		maximisingPlayer = !maximisingPlayer;
	}

	//return states.MaxAbstract(attractors.representNonPrimeVariables().Add()); // should be min anyway but this is very wrong
	//return backMax(states); // should be min anyway but this is very wrong

	return states;
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

BDD Game::representChosenTreatment(int level, int treatment) const { // in this case var is the val..
	treatment++;  // 0 represents no treatment, so n is represented by n+1. not actually required for choice vars as we don't allow zero, but easier to do this way for symmetry
	BDD bdd = attractors.manager.bddOne();

	int i = attractors.numUnprimedBDDVars * 2 +
		bits(oeVars.size() + 1) +
		numMutations * bits(koVars.size() + 1) +
		level * bits(oeVars.size() + 1);

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
		numMutations * bits(koVars.size()) +
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
	ADD score = attractors.manager.addZero();

	for (int val = 0; val <= attractors.ranges[apopVar]; val++) { // what if it is a ko/oe var with a range of 0?
		ADD selector = attractors.representUnprimedVarQN(apopVar, val).Add(); // how efficent is conversion, again. should we just rewrite attractors in terms of 0/1 ADDs?
		score = selector.Ite(attractors.manager.constant(val + 1), score); // maybe IteConstant
	}

	return score;
}
