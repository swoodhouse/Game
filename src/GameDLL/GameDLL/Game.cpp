// heavy refactoring needed. an on Attractors.cpp too i think

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

//void Game::computeAttractorsExplict(const std::string& outputPath, const std::string& header) const {
//    // what does this need to do
//    // have std::list<BDD> attractors.attractors(transitionBDD, statesToRemove)
//    // so i need to form many transitionBDDs..
//    // statesToRemove can be manager.bddZero() for all initial states, at least in explict mode
//
//
//    // need to set the parameters of this function. what is passed here, what is passed to ctor of Game, what do i call to make transitionRelation
//    // you have attractors.representSyncQNTransitionRelation() but that uses private variables..
//    // if you want to use here they would have to be parameters instead..
//    // whereas for the symbolic..
//    for (...) {
//        BDD transitionRelation = ...;
//        if (transitionRelation.IsZero()) std::cout << "TransitionBDD is zero!" << std::endl;
//
//        std::string muts = "";
//
//        std::list<BDD> atts = attractors.attractors(transitionRelation, attractors.manager.bddZero());
//
//        int i = 0;
//        for (const BDD& attractor : atts) {
//            std::ofstream file(outputPath + muts + "Attractor" + std::to_string(i) + ".csv");
//            file << header << std::endl;
//            file << attractors.prettyPrint(attractor) << std::endl;
//            i++;
//        }
//    }
//}
//
//void Game::computeAttractorsSymbolicAll(const std::string& outputPath, const std::string& header) const {
//    // two ways to do symbolically.. in one go or in height goes
//    // all in statesToRemove
//    if (mutantTransitionRelation.IsZero()) std::cout << "TransitionBDD is zero!" << std::endl;
//
//    BDD statesToRemove = ...;
//
//    std::list<BDD> atts = attractors.attractors(mutantTransitionRelation, statesToRemove);
//
//    int i = 0;
//    for (const BDD& attractor : atts) {
//        std::ofstream file(outputPath + "Attractor" + std::to_string(i) + ".csv");
//        file << header << std::endl;
//        file << attractors.prettyPrint(attractor) << std::endl;
//        i++;
//    }
//
//
//    // score...
//}

// now states to remove.. i should have functions or variables which already take care of that. then rig up the dll entry point
// have these...
// //BDD tempAtMostNMutations(int n) const;

ADD Game::buildScoreRelation(int apopVar, int apopRange) const {
    ADD score = attractors.manager.addZero(); // zero?? -infinity?

    for (int val = 0; val <= apopRange; val++) { // what if it is a ko/oe var with a range of 0?
        ADD selector = attractors.representUnprimedVarQN(apopVar, val).Add(); // how efficent is conversion, again. should we just rewrite attractors in terms of 0/1 ADDs?
        score = selector.Ite(attractors.manager.constant(val + 1), score); // maybe IteConstant
        // PRETTY SURE IT WILL HAVE TO BE VAL + 1, ZERO IS FOR UNREACHED STATES
    }

    return score;
}

// question is can i just call attractors.representUnprimedVarQN.. answer is no because it relies on ranges..
// so.. the number of bits we need for each one is the number needed to represent bits(length(koVars))
// need a Game::countBits
BDD Game::representMutationVar(int var, int val) const {
    BDD bdd = attractors.manager.bddOne();
    
    int i = var * bits(koVars.size()); //countBitsMutVar(var); // different for treatment vars.. need to also count mut vars first
    int b = bits(koVars.size()); // you actually can use fewer bits that this, as you can represent one fewer choice each time
    for (int n = 0; n < b; n++) {
        BDD var = attractors.manager.bddVar(i);
        if (!nthBitSet(val, n)) {
            var = !var;
        }
        bdd *= var;
        i++;
    }
    
    return bdd;
}

// huge amount of duplication here
BDD Game::representTreatmentVar(int var, int val, int numMutVars) const {
    BDD bdd = attractors.manager.bddOne();

    int i = numMutVars * bits(koVars.size()) + var * bits(oeVars.size()); //countBitsTreatVar(var); // different for treatment vars.. need to also count mut vars first
    int b = bits(oeVars.size()); // you actually can use fewer bits than this, as you can represent one fewer choice each time.. in practice probably not going to help much fi height = 4
    for (int n = 0; n < b; n++) { // this is repeated in lots of places
        BDD var = attractors.manager.bddVar(i);
        if (!nthBitSet(val, n)) {
            var = !var;
        }
        bdd *= var;
        i++;
    }

    return bdd;
}

// stolen from rosetta code
void comb(int N, int K) {
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    // print integers and permute bitmask
    do {
        for (int i = 0; i < N; ++i) // [0..N-1] integers
        {
            if (bitmask[i]) std::cout << " " << i;
        }
        std::cout << std::endl;
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
}

// this is a bit of a nightmare.. needs a table per ko? no.. yes
// we also need to only consider products of a certain length.. e.g. no point building table for 10 mutations + 10 treatments if we are going to height 2... so maybe it is not cartesian product i want..
BDD Game::mutantSyncQNTransitionRelation() const {
    //BDD relation = attractors.manager.bddZero();

    //int i = 0;
    //// you need to do all combinations of kovars and oevars.. cross product of them
    //for (koVars)
    //    oeVars
    //    BDD condition =...representTreatmentVar representMutationVar
    //    relation = condition.ite(attractors.representSyncQNTransitionRelation(qnT[i]), relation) // is this gonna work correctly? you want this combination and only this combination
    //... // this should be simple.. given a seq of qn tables, repreatedly call attractors.syncrelation
    //    // except is isn't because attractors stores them as member vars. all that has to be moved up to Game
    //    // maybe ranges need to line up too between qn tables?

    //return relation;
}

// delete most of commented out code


//BDD Attractors::representSyncQNTransitionRelation() const {
//    BDD bdd = manager.bddOne();
//
//    for (int v = 0; v < ranges.size(); v++) {
//        if (ranges[v] > 0) {
//            const auto& iVars = qn.inputVars[v];
//            const auto& iValues = qn.inputValues[v];
//            const auto& oValues = qn.outputValues[v];
//
//            std::vector<BDD> states(ranges[v] + 1, manager.bddZero());
//            for (int i = 0; i < oValues.size(); i++) {
//                states[oValues[i]] += representStateQN(iVars, iValues[i]);
//            }
//            for (int val = 0; val <= ranges[v]; val++) {
//                BDD vPrime = representPrimedVarQN(v, val);
//                bdd *= logicalEquivalence(states[val], vPrime);
//            }
//        }
//    }
//    return bdd;
//}


// why do i have it taking a qntable and minvalues, ranges, etc.
//struct QNTable {
//    std::vector<std::vector<int>> inputVars;
//    std::vector<std::vector<std::vector<int>>> inputValues;
//    std::vector<std::vector<int>> outputValues;
//
//    QNTable(std::vector<std::vector<int>>&& inputVarsV, std::vector<std::vector<std::vector<int>>>&& inputValuesV, std::vector<std::vector<int>>&& outputValuesV) :
//        inputVars(std::move(inputVarsV)), inputValues(std::move(inputValuesV)), outputValues(std::move(outputValuesV)) {}
//};



//// HERE.
////// only do for sync for now
////// do i need ranges on ko and oevars
////BDD mutantSyncQNTransitionRelation(const std::vector<int>& minValues, const std::vector<int>& ranges, const Cudd& manager, const QNTable& qn, const std::vector<int>& koVars, const std::vector<int>& oeVars) {
////    BDD bdd = manager.bddOne();
////
////    for (int v = 0; v < ranges.size(); v++) { // actually presents an issue. currently if the range is zero we just ignore. can't if oe.
////        BDD f = manager.bddOne();
////        if (ranges[v] > 0) {
////            const auto& iVars = qn.inputVars[v];
////            const auto& iValues = qn.inputValues[v];
////            const auto& oValues = qn.outputValues[v];
////
////            std::vector<BDD> states(ranges[v] + 1, manager.bddZero());
////            for (int i = 0; i < oValues.size(); i++) {
////                states[oValues[i]] += representStateQN(iVars, iValues[i]);
////            }
////            for (int val = 0; val <= ranges[v]; val++) { // what if it is a ko/oe var with a range of 0?
////                BDD vPrime = representPrimedVarQN(v, val);
////                f *= logicalEquivalence(states[val], vPrime);
////            }
////        }
////        if (...) { // if in ko vars.
////                   // not sure which ite function to use..
////            BDD isMutated = representUnprimedVarQN(v, ...val);
////            BDD ko = representPrimedVarQN(v, ...val); // minval? range? 0?
////            f = isMutated.Ite(ko, f) // if ko1 = g | ko2 = g | ... kon = g then vprime = n else f. or do i enforce lexiographical ordering/inequality of muts
////        }
////        \	else if (...) {     // kind of unclear what to do if a var is both mutated and over expressed. also, mutations can go both ways. ko dominating here..
////            BDD isOe = representUnprimedVarQN(v, ...val); // | | | maybe refactor out to isOe()
////            BDD oe = representPrimedVarQN(v, ...val); // minval? range? 0?
////            f = isOe.Ite(oe, f) // if ko1 = g | ko2 = g | ... kon = g then vprime = n else f. or do i enforce lexiographical ordering/inequalit
////                ..
////        }
////
////        bdd *= f;
////    }
////
////    return bdd;
////    //   // primed = unprimed.. or actually, if we are clever enough to skip over these in rename and exists then we never need a primed ko var
////    // no, you really do need these.
////}



BDD Game::invalidMutationBitCombinations(int numMutations) const {
    BDD S = attractors.manager.bddOne();

    for (int var = 0; var < numMutations; var++) {
        int b = bits(koVars.size());
        int theoreticalMax = (1 << b) - 1;

        for (int val = koVars.size() + 1; val <= theoreticalMax; val++) {
            S *= !representMutationVar(var, val);
        }
    }

    return S;
}

BDD Game::invalidTreatmentBitCombinations(int numMutations, int numTreatments) const {
    BDD S = attractors.manager.bddOne();

    for (int var = 0; var < numTreatments; var++) {
        int b = bits(koVars.size());
        int theoreticalMax = (1 << b) - 1;

        for (int val = koVars.size() + 1; val <= theoreticalMax; val++) {
            S *= !representTreatmentVar(var, val, numMutations);
        }
    }

    return S;
}
//
//BDD Attractors::representPrimeVariables() const {
//    BDD bdd = manager.bddOne();
//    for (int i = numUnprimedBDDVars; i < numUnprimedBDDVars * 2; i++) {
//        BDD var = manager.bddVar(i);
//        bdd *= var;
//    }
//    return bdd;
//}
//
//inline int Game::countBitsMutVar(int var) const { // so this counts bits up to var right. definitely do differently to how i am doing below..
//    auto lambda = [](int a, int b) { return a + bits(b); }; // insert size call here and replace ranges with koVars // is koVars the right one?
//    return attractors.numUnprimedBDDVars * 2 + std::accumulate(ranges.begin(), ranges.begin() + var, 0, lambda);
//    // you can actually dec by one each time as you can represent one fewer mut
//}

///*BDD*/ADD Game::representTreatmentVar(int var, int val) const {
//    return ADD();
//}
//
//
//BDD Attractors::representPrimedVarQN(int var, int val) const {
//    BDD bdd = manager.bddOne();
//    int i = numUnprimedBDDVars + countBits(var);
//
//    int b = bits(ranges[var]);
//    for (int n = 0; n < b; n++) {
//        BDD var = manager.bddVar(i);
//        if (!nthBitSet(val, n)) {
//            var = !var;
//        }
//        bdd *= var;
//        i++;
//    }
//
//    return bdd;
//}

BDD Game::nMutations(int n) const {
    BDD bdd = attractors.manager.bddOne();

    for (int var = 0; var < n; var++) {
        BDD isMutated = !representMutationVar(var, 0);
        bdd *= isMutated;
    }
    for (int var = n; var < koVars.size(); var++) {
        BDD isNotMutated = representMutationVar(var, 0);
        bdd *= isNotMutated;
    }
    return bdd;
}

// refactor out the duplication
BDD Game::nTreatments(int n, int numMutVars) const {
    BDD bdd = attractors.manager.bddOne();

    for (int var = 0; var < n; var++) {
        BDD isMutated = !representTreatmentVar(var, 0, numMutVars);
        bdd *= isMutated;
    }
    for (int var = n; var < oeVars.size(); var++) {
        BDD isNotMutated = representTreatmentVar(var, 0, numMutVars);
        bdd *= isNotMutated;
    }
    return bdd;
}


void Game::computeAttractorsSymbolicN(const std::string& outputPath, const std::string& header, bool maximisingPlayer) const {
    if (mutantTransitionRelation.IsZero()) std::cout << "TransitionBDD is zero!" << std::endl;

    //BDD invalidStates = invalidMutationBitCombinations(); // can't call here, you need to repeatedly call.. for now.

    int numMutations = 0;
    int numTreatments = 0;

    for (int i = 0; i < height; i++) { // off by one?
        BDD initialStates = nMutations(numMutations) * nTreatments(numTreatments, numMutations); // you want to iteratively build this really........
        BDD invalidStates = invalidMutationBitCombinations(numMutations) + invalidTreatmentBitCombinations(numTreatments);
        // at the moment this is independent for each level of the tree? does that work in general?
        
        std::list<BDD> atts = attractors.attractors(mutantTransitionRelation, invalidStates * !initialStates);

        int j = 0;
        for (const BDD& attractor : atts) {
            std::ofstream file(outputPath + "Attractor" + std::to_string(i) + "_" + std::to_string(j) + ".csv");
            file << header << std::endl;
            file << attractors.prettyPrint(attractor) << std::endl;
            j++;
            // score, printminterm/dag
        }

        if (maximisingPlayer) {
            numTreatments++;
        }
        else {
            numMutations++;
        }
        maximisingPlayer = !maximisingPlayer;
    }
}


////// instead od doing #height attractor computations you could do one and collect, but that seems messier
////// ok. so todo for version 1: the initial scoring of attractors. buildUnmutateRelation and nMutations. backMin.
//// as a first test check the attractors are the same as the explict version
//void Game::testAttractors(const std::vector<int>& minValues, const std::vector<int>& ranges, const QNTable& qn, const std::vector<int>& koVars, const std::vector<int>& oeVars, const std::string& outputFile, const std::string& header) {
//    const Cudd manager;
//    manager.AutodynEnable(CUDD_REORDER_GROUP_SIFT); // who knows what best choice now is
//    BDD tr = mutantSyncQNTransitionRelation(minValues, ranges, manager, qn, koVars, oeVars);
//    BDD initialStates = atMostNMutations(koVars) * atMostNOEs(oeVars);
//    BDD statesToRemove = !initialStates;
//    std::list<BDD> syncLoops = attractors(tr, statesToRemove);
//
//    int i = 0;
//    for (const BDD& attractor : syncLoops) {
//        attractor = attractor.ExistAbstract(koVars * oeVars);
//        std::ofstream file(outputFile + "Attractor" + std::to_string(i) + ".csv");
//        file << header << std::endl;
//        file << prettyPrint(attractor) << std::endl;
//        i++;
//    }
//}




















//ADD/*std::list<ADD>*/ Game::minimax(int height, bool maximisingPlayerLast) const { // height and numMutations and numTreatments obviously related..
//                                                                 // refactor, messy
//    int temp = height / 2;
//    int numMutations = height % 2 != 0 && !maximisingPlayerLast ? temp + 1 : temp;
//    int numTreatments = height % 2 != 0 && maximisingPlayerLast ? temp + 1 : temp;
//
//    bool maximisingPlayer = maximisingPlayerLast;
//    //std::list<ADD> path;
//    ADD states = attractors.manager.addZero(); // 1 or 0?
//
//    for (const ADD& a : attractors.attractors(mutantTransitionRelation, !(nMutations(numMutations) * nTreatments(numTreatments)))) {
//        // this is the bit nir was saying i should do for iteration with the set too. 
//        // if i don't know a bound on how many times to iterate this for mean, i can check after the fact all the attractors and check n was large enough
//        ADD max = (a * scoreRelation).FindMax();
//        ADD scoredLoop = max * a;
//        states += scoredLoop;
//    }
//
//    /*DdNode *
//        Cudd_addExistAbstract(
//            DdManager * manager,
//            DdNode * f,
//            DdNode * cube
//        )
//        Abstracts all the variables in cube from f by summing over all possible values taken by the variables.Returns the abstracted ADD.
//        
//        DdNode * 
//Cudd_addOrAbstract(
//  DdManager * manager, 
//  DdNode * f, 
//  DdNode * cube 
//)
//Abstracts all the variables in cube from the 0-1 ADD f by taking the disjunction over all possible values taken by the variables. Returns the abstracted ADD if successful; NULL otherwise.
//Side Effects None
//See Also Cudd_addUnivAbstract Cudd_addExistAbstract
//
//        DdNode * 
//Cudd_addUnivAbstract(
//  DdManager * manager, 
//  DdNode * f, 
//  DdNode * cube 
//)
//Abstracts all the variables in cube from f by taking the product over all possible values taken by the variable. Returns the abstracted ADD if successful; NULL otherwise.
//Side Effects None
//See Also Cudd_addExistAbstract Cudd_bddUnivAbstract Cudd_addOrAbstract
//        */
//
//    // i'm not thinking the below commented out version, with just back() and with chooseMax or chooseMin is the correct option. the problem is max and min abstract.
//    // may have to do min as max-x or otherway around too. and you need std::list path to remember what exactly the choices you made were.
//
//    // at the moment have stacking treatments, treating them the same
//    // what happens if there are equally good choices
//    for (; height > 0; height--) { // do i have an off by one error
//        if (maximisingPlayer) {
//            numTreatments--;
//            states = back(states);
//            states *= untreatRelation; // maybe treat as an ADD to begin with..
//            ADD att = attractors.manager.addZero();
//            for (const ADD& a : attractors.attractors(mutantTransitionRelation, !(nMutations(numMutations) * nTreatments(numTreatments)))) {
//                att += a;
//            }
//            states *= att;
//            //path.push_front(states);
//            //states = chooseMaxTreatment(states, height); // height - 1??
//        }
//        else {
//            numMutations--;
//            states = back(states);
//            states *= untreatRelation; // maybe treat as an ADD to begin with..
//            ADD att = attractors.manager.addZero();
//            for (const ADD& a : attractors.attractors(mutantTransitionRelation, !(nMutations(numMutations) * nTreatments(numTreatments)))) {
//                att += a;
//            }
//            states *= att;
//            //path.push_front(states);
//            //states = chooseMinMutation(states, height); // height - 1??
//        }
//
//        maximisingPlayer = !maximisingPlayer;
//    }
//    return states;
//    // is it return path.FindMax.. maybe exist on everything apart from muts and treatments made..
//    //return path; // will have the attractors at top level, each with an attached score, and each with the sequence of mutations you should make
//                 // if i also want to remember the attractors themselves.. or the apop values.. i should store them too somehow..
//                 // also, you really should do a final min or max over the attractors at the final level
//}















//
//
//
//
//
////
////
////
/////* so..
////in case where every sub-model stabilises
////
////bottom
////mut1=a, mut2=b, mut3=c, ..state.., score = n
////
////then you do unmutate
////mut1=a/b, mut2=b/c, mut3=empty, ..states.., mutStored1=a/b/c, score = n
////
////then you do back
////mut1=a/b, mut2=b/c, mut3=empty, ....states...., mutStored1=a/b/c, score = n
////
////then you do choose
////
////one level up
////mut1=a, mut2=b, mut3=empty, ..states.., score = 1
////
////back * one level up
////mut1=a, mut2=b, mut3=empty, ..states.., mutStored1=c, score = n | ... | ...
////
////ALTERNATE............
////
////so i just thought of something. yes. unmutate is easy, set mutN as empty and choose N-1 of the old to be mut 1..N-1 lexigraphically. and set mutStored1=old mutN.
////back() also needs to handle mutStored, and choose the best.. is that right?. eg. exist out everything but mutStored. then do FindMax, then multiply back? so my thinking before was right, except it needs to go into back().. well, does it? you only have to do it once, not every backwards step.
////so what to call this? chooseMax? basically if the same state can reach mutliple scores.
////
////yes this is absolutely true. so you do back. and for the same mut1..n states you can have a different mutStored and score.
////so after calling back you need to call choose. THEN you can mult.
////x
////
////the lexiographical is going to hurt you. unmutate needs to take into account.
////*/
////
//
//
//
//
//
//
//
//
//
//
//
///*
//to be done: minimax [change to do iteratively?], back, intersect[uses unmutateRelation, don't recompute it]
//done: atMostNMutations, scoreAdd [multiply 1-0 add by this], unmutateRelation, mutantSyncQNTransitionRelation, nMutations[lexiographical ordering not done.. do along with invalidBitCombinations]
//to finalise:  mutantSyncQNTransitionRelation,
//// lexiographical. ite? be careful with invalidbitcombinations
//// also print minterms to check correct along the way
//// probably also need to be careful with the background.. if its zero probably don't ever take the min but negate and then take max
//// may be a better way than explict, i.e matrix multiplication
//// well.. transition relation needs koVar' = koVar
//
//// idea: Exist seperately multiply bundle back. you have multiple parent attractors to intersect with..........
//// seems like we are doign redundant work by piggybacking on Attractors
//*/
//
//#include "stdafx.h"
//#include "Attractors.h"
//
//// on collapsing.. you can either pick random if a 1/0 add or findmax (i think) if scored. only the mean where you have to do successor.
//// although implement both ways and verify the same
//// actually, you can't do it the above way if you want to collapse and do in parallel
//
////you can bundle before the max calculation because each state only has one possible successor. all elements in the loop are connected anyway
//ADD collapseLoop(ADD loop) {
//    // pick a state.
//    // x = max(x, immediateSucc(x))...
//}
//
//ADD collapseAttractors(...) {
//    ADD states = manager.addOne; // rename
//    for (BDD attractor : runSync(...)) {
//        ADD x = attractor.Add() * score;
//        states *= collapseLoop(x); // have this always do the max. and if you want to take the mean then pass -x
//    }
//    return states;
//}
//
//
//
//
//

//
//
//
//
//// /arithmetic/ or even algebraic symbolic computation
//// Symbolic Model Checking of Probabilistic Processes using MTBDDs and the Kronecker Representation
//// "we allow nondeterminism as well as probability"
//// "we generate the matrix in full, then perform BDD reachability analysis to obtain the actual state space"
//// "Concurrent probabilistic systems generalise ordinary Markov chains in that they allow a nondeterministic choice between possibly several probability distributions in a given state."
//// "Our tool performs model checking for PBTL, with and without fairness. The MTBDD for a system of modules is automatically generated from its textual
//// description using the translation described above.Forward reachability analysis is then performed to filter out the unreachable states."
//// "To model check qualitative properties (typically with probability 1), reachability analysis through fixed point computation suffices.Quantitative properties, however, require numerical computation."
//// "matrix-bymatrix multiplication algorithms supplied with the CUDD package"
////The only precaution which must
////be taken is to ensure that the relative order of the MTBDD variables is correct.
////If the MTBDDs f and g represent the matrices F and G respectively
////and all the variables in f precede all those of g in the overall variable orderi
////then Apply(*, f, g) gives the MTBDD for the matrix F x G which depends
////on the variables of both.Because we have ensured that our Boolean variables
////are grouped and ordered by module, the Kronecker expression can be computed
////easily with Apply(*).
//// (1 / number attractors) * ...?
////  addition, multiplication, division, minimum, maximum,
//// ADD MatrixMultiply(const ADD& B, std::vector<ADD> z) const;
////     ADD TimesPlus(const ADD& B, std::vector<ADD> z) const;
////     ADD Triangle(const ADD& g, std::vector<ADD> z) const;
////     ADD Eval(int * inputs) const;
//
//
//
//// only works for sync models. actually.. should work for async?
//
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
//
//

//
//
//
//
//ADD Game::backMin(const ADD& states) const {
//    // repeatedly do immediatePre then max seems correct. as long as transition relation has koVar' = koVar
//    // do have the problem that multiple states with different scores could reach..
//    //  if you get multiple scores for that state how do i tell it to take the max or min of those? I SORT OF THINK THIS IS THE REMAINING ISSUE AND ALSO HOW TO STORE PATHS
//}
//


//Game.cpp
//
//#include "stdafx.h"
//#include "Attractors.h"
//#include "Game.h"
//
//// issue with manager needing to be shared between Attractors and Game. ref? value?
//
//// may need to have a +1 shift, as zero represents unreachable?
////// will invalid bit combinations cause a problem? shouldn't because the state you are multiplying with can't have them?
////// doing just with apop for now

//
///*BDD*/ADD Game::representPrimedMutationVarZero(int var) const {
//    return ADD();
//}
//
////BDD Game::representChosenMutation(int level) const { // in this case var is the val..
////    return BDD();
////}
////
////BDD Game::representChosenTreatment(int level) const { // in this case var is the val..
////    return BDD();
////}
//
//// temp.. needs another variable
///*BDD*/ADD Game::representChosenMutation(int level) const { // in this case var is the val..
//    return ADD();
//}
//
////BDD Game::representChosenMutation(int level, int mutation) const { // in this case var is the val..
////    return BDD();
////}
//
///*BDD*/ADD Game::representChosenTreatment(int level, int treatment) const { // in this case var is the val..
//    return ADD();
//}
//

//
///*BDD*/ADD Game::buildMutantTransitionRelation() const {
//    return ADD();
//}
//
////
//
///*BDD*/ADD Game::buildUnmutateRelation() const {
//    auto add = attractors.manager.addZero();
//
//    for (int var = 0; var < koVars.size(); var++) {
//        auto isMutated = !representMutationVar(var, 0); // lexigraphical could seemingly make this more efficent.. ite..
//        auto unmutate = representPrimedMutationVarZero(var);
//        auto rememberMutation = representChosenMutation(var); // need to ITE over height
//        auto transition = isMutated * unmutate * rememberMutation;
//        add += transition;
//    }
//
//    return add;
//}
//
//
////BDD Attractors::immediatePredecessorStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
////    BDD bdd = renameAddingPrimes(valuesBdd);
////    bdd *= transitionBdd;
////    return bdd.ExistAbstract(primeVariables);
////}
////BDD Attractors::backwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
////    BDD reachable = manager.bddZero();
////    BDD frontier = valuesBdd;
////
////    while (!frontier.IsZero()) {
////        frontier = immediatePredecessorStates(transitionBdd, frontier) * !reachable;
////        reachable += frontier;
////    }
////    return reachable;
////}
////
////ADD Game::backMin(const ADD& states) const {
////    return ADD();
////}
////
////ADD Game::backMax(const ADD& states) const {
////    return ADD();
////}
//
//ADD Game::back(const ADD& states) const {
//    return attractors.backwardReachableStates(mutantTransitionRelation, states);
//}
//
////ADD Game::chooseMaxTreatment(const ADD& states, int level) const {
////    return states.ExistAbstract(representChosenTreatment(level).Add()); // really want MaxAbstract!
////    // maybe treat as ADD to begin with
////}
////
////ADD Game::chooseMinMutation(const ADD& states, int level) const {
////    return states.ExistAbstract(representChosenMutation(level).Add()); // really want MinAbstract!
////    // maybe treat as ADD to begin with
////}
//
//// we only allow nTreatments = 2 at all stages. and there may be a stage in between unmutate and untreat
//
//
//
//
//// what would be good to put into here
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
//
////// maybe max within an add is threshold or something
////// ADD Threshold(const ADD& g) const; // f if f>=g; 0 if f<g.
////// ADD OneZeroMaximum(const ADD& g) const; // Returns 1 if f > g and 0 otherwise.
////// ADD FindMax() const; // Finds the maximum discriminant of f. ??
//////    ADD FindMin() const;
////
////// you need to do removeInvalid on the mutations var when you do it on normal vars
////// after calling this you have to do an exist out the unprimed and a rename as usual
////// non deterministically choose a mut var which <> 0. don't recalculate but store it
////// need to introduce the special mutation vars too.. which are (numUnprimedBDDVars * 2) + ...
//// or do i account for that directly in numUnprimedBDDVars?
