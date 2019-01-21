// occurs to me addZero is probably different to constant(0.0) and so my whole val + 1 thing may be redundant.

#include <rapidcheck.h>
#include <random>
#include <chrono>
#include <vector>
#include <algorithm>
#include <functional>
#define NOMINMAX
#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

void test1() {
	rc::check("double reversal yields the original value",
		[](const std::vector<int> &l0) {
		auto l1 = l0;
		std::reverse(begin(l1), end(l1));
		std::reverse(begin(l1), end(l1));
		RC_ASSERT(l0 == l1);
	});
}

/*
test that score relation actually works - generate a random state with a designated apop var, verify that state.add() * score maps to state * apopVar.value.
to test disjunctions.. a loop of a random length. add them all. verify states.add() * score maps to states * max(values apopVars)+
*/
void scoreFixpoint(const Game& game) {
	rc::check("scoreFixpoint...",
		[&](const std::vector<std::pair<int, int>> &varsValues, int apopVar) { // rename scoreVar
		std::vector<int> vars(varsValues.size());
		std::vector<int> values(varsValues.size());

		for (auto it = std::make_move_iterator(varsValues.begin()), end = std::make_move_iterator(varsValues.end()); it != end; ++it) {
			vars.push_back(std::move(it->first));
			values.push_back(std::move(it->second));
		}

		RC_PRE(apopVar < vars.size());
		RC_PRE(*std::min_element(vars.begin(), vars.end()) >= 0);
		RC_PRE(*std::max_element(vars.begin(), vars.end()) < game.attractors.ranges.size());

		// each var needs to be within range
		RC_PRE(*std::min_element(values.begin(), values.end()) >= 0);

		for (int i = 0; i < values.size(); i++) {
			RC_PRE(values[0] <= game.attractors.ranges[i]);
		}

		ADD scoreRelation = game.buildScoreRelation(apopVar);

		BDD state = game.attractors.manager.bddOne();
		for (int i = 0; i < vars.size(); i++) {
			state *= game.attractors.representUnprimedVarQN(vars[i], values[i]);
		}

		RC_ASSERT(state.Add() * scoreRelation == (state.Add() * game.attractors.manager.constant(values[apopVar])));
	});
}

// simple test. combining above and this would be more complete
void scoreLoop(const Game& game) {
	rc::check("scoreLoop...",
		[&](const std::vector<int> &values, int apopVar) { // rename scoreVar
		RC_PRE(apopVar > 0);
		RC_PRE(apopVar < game.attractors.ranges.size());
		RC_PRE(*std::min_element(values.begin(), values.end()) >= 0);
		int max = *std::max_element(values.begin(), values.end());
		RC_PRE(max <= game.attractors.ranges[apopVar]);

		BDD loop = game.attractors.manager.bddZero();

		for (int value : values) {
			loop += game.attractors.representUnprimedVarQN(apopVar, value);
		}

		ADD scoreRelation = game.buildScoreRelation(apopVar);
		RC_ASSERT(loop.Add() * scoreRelation == (loop.Add() * game.attractors.manager.constant(max)));
	});

}

/* backMax[Min] - test backMax not immediateBackMax [backMin is similar]. pick two random, unequal, states. call Attractors.backwardReachableStates on each, call those a and b. multiply those two random states by n and m, n < m, and then add. call backMax on that, call the result x. check that x.bdd() = a + b. check that (a-b).Add() * x = (a - b).Add() * n. check that b.Add() * x = b.Add() * n. they can be separate, equal, or intersecting - count how many times each case occurs - a * b = 0. a = b, else. [https://github.com/emil-e/rapidcheck/blob/master/doc/distribution.md]+*/
void backMax(const Game& game) {
	rc::check("backMax...",
		[&](int i, int j) {
		ADD n = game.attractors.manager.constant(std::min(i, j));
		ADD m = game.attractors.manager.constant(std::max(i, j));

		BDD S = game.attractors.manager.bddOne();

		game.removeInvalidTreatmentBitCombinations(S); // refacotr this out.. can be computed once too
		game.removeInvalidMutationBitCombinations(S);
		game.forceMutationLexicographicalOrdering(S);
		game.attractors.removeInvalidBitCombinations(S);

		// unsure if the two other params are required
		BDD state1 = game.attractors.randomState(S, std::unordered_set<int>(), game.attractors.manager.bddOne());
		BDD state2 = game.attractors.randomState(S, std::unordered_set<int>(), game.attractors.manager.bddOne());

		RC_PRE(state1 != state2);

		BDD back1 = game.attractors.backwardReachableStates(game.mutantTransitionRelation, state1);
		BDD back2 = game.attractors.backwardReachableStates(game.mutantTransitionRelation, state2);

		ADD scoredState1 = state1.Add() * game.attractors.manager.constant(n);
		ADD scoredState2 = state2.Add() * game.attractors.manager.constant(m);

		ADD scoredBack = game.backMax(scoredState1 + scoredState2);

		// count whether equal, separate, intersecting
		RC_CLASSIFY(back1 == back2, "Equal");
		RC_CLASSIFY((back1 * !back2).IsZero(), "Separate");
		RC_CLASSIFY(!(back1 * !back2).IsZero(), "Intersecting");

		RC_ASSERT(scoredBack.BddPattern() == back1 + back2); // states reachable are same
		RC_ASSERT((back1 * !back2).Add() * scoredBack == (back1 * !back2).Add() * n); // set difference is scored right
		RC_ASSERT((back2 * !back1).Add() * scoredBack == (back2 * !back1).Add() * m); // set difference is scored right
		RC_ASSERT((back1 * back2).Add() * scoredBack == (back1 * back2).Add() * m); // intersection is max
		RC_ASSERT(back2.Add() * scoredBack == back2.Add() * m); // back of max initial are all max
	});
}

void backMin(const Game& game) {
	rc::check("backMin...",
		[&](int i, int j) {
		ADD n = game.attractors.manager.constant(std::min(i, j));
		ADD m = game.attractors.manager.constant(std::max(i, j));

		BDD S = game.attractors.manager.bddOne();

		game.removeInvalidTreatmentBitCombinations(S); // refacotr this out.. can be computed once too
		game.removeInvalidMutationBitCombinations(S);
		game.forceMutationLexicographicalOrdering(S);
		game.attractors.removeInvalidBitCombinations(S);

		// unsure if the two other params are required
		BDD state1 = game.attractors.randomState(S, std::unordered_set<int>(), game.attractors.manager.bddOne());
		BDD state2 = game.attractors.randomState(S, std::unordered_set<int>(), game.attractors.manager.bddOne());

		RC_PRE(state1 != state2);

		BDD back1 = game.attractors.backwardReachableStates(game.mutantTransitionRelation, state1);
		BDD back2 = game.attractors.backwardReachableStates(game.mutantTransitionRelation, state2);

		ADD scoredState1 = state1.Add() * game.attractors.manager.constant(n);
		ADD scoredState2 = state2.Add() * game.attractors.manager.constant(m);

		ADD scoredBack = game.backMin(scoredState1 + scoredState2);

		// count whether equal, separate, intersecting
		RC_CLASSIFY(back1 == back2, "Equal");
		RC_CLASSIFY((back1 * !back2).IsZero(), "Separate");
		RC_CLASSIFY(!(back1 * !back2).IsZero(), "Intersecting");

		RC_ASSERT(scoredBack.BddPattern() == back1 + back2); // states reachable are same
		RC_ASSERT((back1 * !back2).Add() * scoredBack == (back1 * !back2).Add() * n); // set difference is scored right
		RC_ASSERT((back2 * !back1).Add() * scoredBack == (back2 * !back1).Add() * m); // set difference is scored right
		RC_ASSERT((back1 * back2).Add() * scoredBack == (back1 * back2).Add() * n); // intersection is min
		RC_ASSERT(back1.Add() * scoredBack == back1.Add() * n); // back of min initial are all min
	});
}


/*
+untreat and unmutate work - ...generate a random BDD. Tag it with N treatments. Verify that untreat(bdd) == what? the bdd you began with before you tagged it with treatment1, or treatment2, ....  + a tag saying that was indeed the one that was chosen. do i pick a random number of treatments or can i assume this works inductively*/
void untreat(const Game& game) {
	rc::check("untreat",
		[&](std::vector<bool> v, int level, int treatment) {
		RC_PRE(v.size() > 0);
		RC_PRE(level < game.numTreatments);
		RC_PRE(treatment < game.oeVars.size());

		std::random_device rd;     // only used once to initialise (seed) engine
		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
		std::uniform_int_distribution<int> uni(0, 1); // guaranteed unbiased

		if (v.size() > game.attractors.numUnprimedBDDVars) {
			v.resize(game.attractors.numUnprimedBDDVars);
		}

		BDD states = game.attractors.manager.bddVar(0);
		for (int i = 1; i < v.size(); i++) {
			if (uni(rng) == 0) { // report
				states *= game.attractors.manager.bddVar(i);
			}
			else {
				states += game.attractors.manager.bddVar(i);
			}
		}

		BDD untreated = states * game.representChosenTreatment(level, treatment);
		BDD treated = states * game.representTreatment(treatment);

		RC_ASSERT(game.untreat(level, treated.Add()) == untreated.Add());
	});
}

// same as above.. except add mutations up to level.
void unmutate(const Game& game) {
	rc::check("unmutate",
		[&](std::vector<bool> v, int level, int mutation) {
		RC_PRE(v.size() > 0);
		RC_PRE(level < game.numMutations);
		RC_PRE(mutation < game.koVars.size());

		std::random_device rd;     // only used once to initialise (seed) engine
		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
		std::uniform_int_distribution<int> uni(0, 1); // guaranteed unbiased

		if (v.size() > game.attractors.numUnprimedBDDVars) {
			v.resize(game.attractors.numUnprimedBDDVars);
		}

		BDD states = game.attractors.manager.bddVar(0);
		for (int i = 1; i < v.size(); i++) {
			if (uni(rng) == 0) { // report
				states *= game.attractors.manager.bddVar(i);
			}
			else {
				states += game.attractors.manager.bddVar(i);
			}
		}

		std::uniform_int_distribution<int> uni2(0, game.koVars.size() - 1);

		BDD otherMutations = game.attractors.manager.bddOne();
		for (int i = 0; i < level; i++) {
			int m = uni2(rng);
			otherMutations *= game.representMutation(level, m);
		}

		BDD unmutated = states * otherMutations * game.representChosenMutation(level, mutation);
		BDD mutated = states * otherMutations * game.representMutation(level, mutation);

		RC_ASSERT(game.unmutate(level, mutated.Add()) == unmutated.Add());
	});
}

// generate N random pairs of vals. call representMutation(var, val). if -1 call representMutationNone(var). flip a coin to add or multiply to state[skipping this at the moment]. in parallel build a state2 doing representPrimedMutation(var, val), if -1 then representPrimedMutationNone(var). then, check state1 == state2
void renameMutVarsRemovingPrimes(const Game& game) {
	rc::check("rename...",
		[&](const std::vector<float> &l) {
		RC_PRE(l.size() > 0);
		// report length
		BDD state1 = game.attractors.manager.bddOne();
		BDD state2 = game.attractors.manager.bddOne();

		std::random_device rd;     // only used once to initialise (seed) engine
		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
		std::uniform_int_distribution<int> uni(-1, game.numMutations - 1); // guaranteed unbiased

		for (int i = 0; i < l.size(); i++) {
			int val = uni(rng); // report this value
			if (val == -1) { // report
				state1 *= game.representMutationNone(i);
				state2 *= game.representPrimedMutationNone(i);
			}
			else {
				state1 *= game.representMutation(i, val);
				state2 *= game.representPrimedMutation(i, val);
			}
		}

		RC_ASSERT(state1 == state2);
	});
}

// findmax works as expected - make n constant 0/N ADDs. ITE together. FindMax should give you back whichever the largest constant is
void findMax(const Game& game) {
	rc::check("findMax....",
		[&](const std::vector<float> &l) {
		RC_PRE(l.size() > 0);
		float max = *std::max_element(std::begin(l), std::end(l));
		RC_PRE(max > 0);

		ADD add = game.attractors.manager.addZero();
		// instead of 0 to n, shuffle? for (i in indices)
		for (int i = 0; i < l.size(); i++) {
			ADD a = game.attractors.manager.addVar(i);
			add = a.Ite(game.attractors.manager.constant(l[i]), add);
		}
		ADD maxAdd = game.attractors.manager.constant(max);

		//add.PrintMinterm();
		//maxAdd.PrintMinterm();
		RC_ASSERT(add.FindMax() == maxAdd);
	});
}

// maxabstract  works as expected - generate a random ADD.. looking like what?.. exist out a var .. simpliest version of this is make a 0/1 ADD, then multiply by a single var that maps to some random positive number if it is 0 and a different one if it is 1 - Ite. Then existmax it out and verify that what it get equal to state * constantADD(std::max(n1, n2)). more complex version is to exist out a whole cube. more complex still is have the 0/1 be a disjunction too+
// is ite enough here?????????? probably.. but the test is more complex. you need to get the child branch that when you call findmax you get the largest value... parents are unaltered.  
// maxabstract on a 0/1 add should be same as exist abstract
void maxAbstract(const Game& game) {
	rc::check("maxAbstract....",
		[&](const std::vector<std::pair<bool, bool>> &v) {
		std::random_device rd;     // only used once to initialise (seed) engine
		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
		std::uniform_real_distribution<double> unf(0.0, 10.0); // guaranteed unbiased
		std::uniform_int_distribution<int> uni(0, v.size() - 1); // guaranteed unbiased
		int var = uni(rng); // var is what you max abstract // report this value

							// need to build two adds one with var ite, one with it removed and the resulting add taking the rhs branch

		BDD bdd = game.attractors.manager.bddVar(0);
		if (!v[0].first) {
			bdd = !bdd;
		}

		for (int i = 1; i < v.size(); i++) {
			if (i == var) {
				break;
			}

			bool value = v[i].first;
			bool operation = v[i].second;
			BDD b = game.attractors.manager.bddVar(i);

			if (!value) {
				b = !b;
			}

			if (operation) {
				bdd *= b;
			}
			else {
				bdd += b;
			}

			i++;
		}

		double random1 = unf(rng);
		double random2 = unf(rng);
		ADD left = bdd.Add() * game.attractors.manager.constant(random1);
		ADD right = bdd.Add() * game.attractors.manager.constant(random2);
		ADD add = game.attractors.manager.addVar(var).Ite(left, right);
		ADD result = add.MaxAbstract(game.attractors.manager.addVar(var));
		RC_ASSERT(result == bdd.Add() * game.attractors.manager.constant(std::max(random1, random2)));

		// if you do an OR with some random stuff on either side you should get that back too.. is that right?
		// particularly on the min side you need to test
		ADD extraLeft = game.attractors.manager.addVar(v.size());
		ADD extraRight = game.attractors.manager.addVar(v.size() + 1);
		ADD extra = game.attractors.manager.addVar(var).Ite(left + extraLeft, right + extraRight);
		ADD extraResult = extra.MaxAbstract(game.attractors.manager.addVar(var));
		RC_ASSERT(extraResult == extraLeft + extraRight + bdd.Add() * game.attractors.manager.constant(std::max(random1, random2)));
	});
}

void maximum(const Game& game) {
	rc::check("check maximum on 0/1 ADDs is same as +",
		[&](const std::vector<bool> &v0, const std::vector<bool> &v1) { // how to ensure same length? // also.. report statisitcs
																	   // make l0 be largest of v0 v1, and l1 be smallest, then truncate l0 to same length as l1
		
		RC_PRE(v0.size() > 0 && v1.size() > 0); // needed?
		std::vector<bool> l0(v0.size() >= v1.size() ? v0 : v1);
		std::vector<bool> l1(v0.size() >= v1.size() ? v1 : v0);
		l0.resize(l1.size());
		// do i even need to do this?
		// report sizes?

		BDD a = game.attractors.representState(l0);
		BDD b = game.attractors.representState(l1);
		ADD x = a.Add();
		ADD y = b.Add();

		//RC_ASSERT((x + y) == x.Maximum(y) == y.Maximum(x));

		//std::cout << "x+y:" << std::endl;
		//(x + y).PrintMinterm();
		//std::cout << "x.Maximum(y):" << std::endl;
		//x.Maximum(y).PrintMinterm();

		RC_ASSERT((a + b).Add() == x.Maximum(y));
		RC_ASSERT(x.Maximum(y) == y.Maximum(x));
		RC_ASSERT((x + y).BddPattern() == (a + b));
	});

	rc::check("check that maximum(random bdd * n, same random bdd * m) = random bdd * m where m >= n",
		[&](const std::vector<bool> &l, int n, int m) {
		RC_PRE(l.size() > 0);
		ADD a = game.attractors.manager.constant(std::min(n, m));
		ADD b = game.attractors.manager.constant(std::max(n, m));
		ADD x = game.attractors.representState(l).Add();

		RC_ASSERT((x * a).Maximum(x * b) == (x * b));
	});
}

void bddPattern(const Game& game) {
	rc::check("bddPattern..",
		[&](const std::vector<bool> &l, int n) { // floats?
		RC_PRE(l.size() > 0);
		RC_PRE(n > 0);
		BDD b = game.attractors.representState(l);
		ADD v = game.attractors.manager.constant(n);

		RC_ASSERT(b == (b.Add() * v).BddPattern());
		RC_ASSERT(b.Add() == (b.Add() * v).BddPattern().Add());
	});
}

void oneZeroMaximum(const Game& game) {
	rc::check("test that maximum(x * n, y * m).bdd() = onezeromaximum(x * n, y * m).bdd()",
		[&](const std::vector<bool> &l0, const std::vector<bool> &l1, int n, int m) { // floats?
		RC_PRE(l0.size() > 0);
		RC_PRE(l1.size() > 0);
		RC_PRE(n > 0);
		RC_PRE(m > 0);

		ADD a = game.attractors.manager.constant(n);
		ADD b = game.attractors.manager.constant(m);
		ADD x = game.attractors.representState(l0).Add();
		ADD y = game.attractors.representState(l1).Add();


		// so i think the problem was we were allowing 0 scores. this could be a problem in general? one zero max really what we want? actually, probably is
		// ([false], [false], 1, 1) is false too..
		// which means that x.max(x) != x or x.onezeromax(x) != x
		RC_ASSERT(x.Maximum(x) == x);
		RC_ASSERT(x.OneZeroMaximum(x) == x);
		// could we repplace onezeromax with this instead?
		//if ((x * a).Maximum(y * b).BddPattern() == (x * a).OneZeroMaximum(y * b).BddPattern()) {
		//	return true;
		//}
		//(x * a).Maximum(y * b).PrintMinterm();
		
		
		//(x * a).PrintMinterm(); // zero
		//std::cout << (x * a).IsZero(); // true
		//std::cout << ((y * b) == y); // true
		//zero.maximum(y) != zero.onexeromaximum
		//return false;
		//RC_ASSERT((x * a).Maximum(y * b).BddPattern() == (x * a).OneZeroMaximum(y * b).BddPattern());
	});
}


extern "C" __declspec(dllexport) int minimax(int numVars, int ranges[], int minValues[], int numInputs[], int inputVars[], int numUpdates[],
    int inputValues[], int outputValues[], int numMutations, int numTreatments, int mutationVars[], int treatmentVars[], int apopVar, int depth, bool maximisingPlayerGoesLast) {
//extern "C" __declspec(dllexport) int test(int numVars, int ranges[], int minValues[], int numInputs[], int inputVars[], int numUpdates[], int inputValues[], int outputValues[], int numMutations, int numTreatments, int mutationVars[], int treatmentVars[], int apopVar) {

	std::vector<int> rangesV(ranges, ranges + numVars);
	std::vector<int> minValuesV(minValues, minValues + numVars);
	std::vector<int> mutationVarsV(mutationVars, mutationVars + numMutations);
	std::vector<int> treatmentVarsV(treatmentVars, treatmentVars + numTreatments);
	std::vector<std::vector<int>> inputVarsV;
	std::vector<std::vector<int>> outputValuesV;
	std::vector<std::vector<std::vector<int>>> inputValuesV;

	int k = 0;
	for (int i = 0; i < numVars; i++) {
		std::vector<int> in;
		for (int j = 0; j < numInputs[i]; j++) {
			in.push_back(inputVars[k]);
			k++;
		}
		inputVarsV.push_back(in);
	}

	k = 0;
	for (int i = 0; i < numVars; i++) {
		std::vector<int> out;
		for (int j = 0; j < numUpdates[i]; j++) {
			out.push_back(outputValues[k]);
			k++;
		}
		outputValuesV.push_back(out);
	}

	k = 0;
	for (int i = 0; i < numVars; i++) {
		std::vector<std::vector<int>> in;
		for (int j = 0; j < numUpdates[i]; j++) {
			std::vector<int> v;
			for (int l = 0; l < numInputs[i]; l++) {
				v.push_back(inputValues[k]);
				k++;
			}
			in.push_back(v);
		}
		inputValuesV.push_back(in);
	}
	

	QNTable qn = QNTable(std::move(inputVarsV), std::move(inputValuesV), std::move(outputValuesV));

	test1();
	Game game(std::move(minValuesV), std::move(rangesV), std::move(qn), std::move(mutationVarsV), std::move(treatmentVarsV), apopVar, depth, maximisingPlayerGoesLast);

	//maximum(game); // passes
	//oneZeroMaximum(game); // fails: test works, reveals that oneZeroMaximum doesn't work how I think it does - so replace it
	//bddPattern(game); // passes
	//findMax(game); // passes. but we don't even seem to be using?

	//renameMutVarsRemovingPrimes(game); // crashes

	//backMax(game); // hanging.. and using a lot of memory
	//backMin(game);
	//untreat(game); // exception
	//unmutate(game); // // exception
	//scoreFixpoint(game); // fails
	//scoreLoop(game); // crashes
	return 0;
}
// Report statistics, change these so they fail too. vectors need to be truncated to numVars

// Test m abstract at level of ites with different abstracted vars
// you need to ensure the random ints are positive and small in some of these tests

// Also, hunt down the zero/one bdd. where does it come from?
