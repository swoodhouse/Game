// Copyright (c) Microsoft Research 2017
// License: MIT. See LICENSE

#include "stdafx.h"
#include "Attractors.h"

inline BDD logicalEquivalence(const BDD& a, const BDD& b) {
    return !(a ^ b);
}

std::string printRange(const std::list<int>& values) {
    std::string s("[");
    s += std::to_string(values.front());
    std::for_each(std::next(values.begin()), values.end(), [&s](int b) { s += ";"; s += std::to_string(b); });
    s += "]";
    return s;
}

std::vector<int> parseRange(const std::string& range) {
    if (range.at(0) != '[') return std::vector<int>(1, std::stoi(range));

    std::string copy = range.substr(1, range.size() - 2);
    std::vector<int> result;
    std::istringstream iss(copy);
    std::string s;
    while (std::getline(iss, s, ';')) result.push_back(std::stoi(s));

    return result;
}

std::string fromBinary(const std::string& bits, int offset) {
    int i = 0;
    auto lambda = [&i](int a) { return a + std::pow(2.0f, i); };
    std::list<int> values{ offset };

    for (auto it = std::begin(bits); it < std::end(bits); ++it) {
        if (*it == '-') {
            std::list<int> copy(values);
            std::transform(copy.begin(), copy.end(), copy.begin(), lambda);
            values.splice(values.end(), copy);
        }
        else if (*it == '1') {
            std::transform(values.begin(), values.end(), values.begin(), lambda);
        }
        i++;
    }
    return values.size() > 1 ? printRange(values) : std::to_string(values.front());
}

BDD Attractors::representState(const std::vector<bool>& values) const {
    BDD bdd = manager.bddOne();
    for (int i = 0; i < values.size(); i++) {
        BDD var = manager.bddVar(i);
        if (!values[i]) {
            var = !var;
        }
        bdd *= var;
    }
    return bdd;
}

BDD Attractors::representNonPrimeVariables() const {
    return representState(std::vector<bool>(numUnprimedBDDVars, true));
}

BDD Attractors::representPrimeVariables() const {
    BDD bdd = manager.bddOne();
    for (int i = numUnprimedBDDVars; i < numUnprimedBDDVars * 2; i++) {
        BDD var = manager.bddVar(i);
        bdd *= var;
    }
    return bdd;
}

inline int Attractors::countBits(int end) const {
    auto lambda = [](int a, int b) { return a + bits(b); };
    return std::accumulate(ranges.begin(), ranges.begin() + end, 0, lambda);
}

BDD Attractors::representUnprimedVarQN(int var, int val) const {
    BDD bdd = manager.bddOne();
    int i = countBits(var);

    int b = bits(ranges[var]);
    for (int n = 0; n < b; n++) {
        BDD var = manager.bddVar(i);
        if (!nthBitSet(val, n)) {
            var = !var;
        }
        bdd *= var;
        i++;
    }

    return bdd;
}

BDD Attractors::representPrimedVarQN(int var, int val) const {
    BDD bdd = manager.bddOne();
    int i = numUnprimedBDDVars + countBits(var);

    int b = bits(ranges[var]);
    for (int n = 0; n < b; n++) {
        BDD var = manager.bddVar(i);
        if (!nthBitSet(val, n)) {
            var = !var;
        }
        bdd *= var;
        i++;
    }

    return bdd;
}

BDD Attractors::representStateQN(const std::vector<int>& vars, const std::vector<int>& values) const {
    BDD bdd = manager.bddOne();
    for (size_t i = 0; i < vars.size(); i++) {
        int var = vars[i];
        int val = values[i];
        bdd *= representUnprimedVarQN(var, val);
    }
    return bdd;
}

BDD Attractors::representSyncQNTransitionRelation(const QNTable& qn) const {
    BDD bdd = manager.bddOne();

    for (int v = 0; v < ranges.size(); v++) {
        if (ranges[v] > 0) {
            const auto& iVars = qn.inputVars[v];
            const auto& iValues = qn.inputValues[v];
            const auto& oValues = qn.outputValues[v];

            std::vector<BDD> states(ranges[v] + 1, manager.bddZero());
            for (int i = 0; i < oValues.size(); i++) {
                states[oValues[i]] += representStateQN(iVars, iValues[i]);
            }
            for (int val = 0; val <= ranges[v]; val++) {
                BDD vPrime = representPrimedVarQN(v, val);
                bdd *= logicalEquivalence(states[val], vPrime);
            }
        }
    }
    return bdd;
}

BDD Attractors::renameRemovingPrimes(const BDD& bdd) const {
    int *permute = new int[numUnprimedBDDVars * 2];
    for (int i = 0; i < numUnprimedBDDVars; i++) {
        permute[i] = i;
        permute[i + numUnprimedBDDVars] = i;
    }
    BDD r = bdd.Permute(permute);
    delete[] permute;
    return r;
}

BDD Attractors::renameAddingPrimes(const BDD& bdd) const {
    int *permute = new int[numUnprimedBDDVars * 2];
    for (int i = 0; i < numUnprimedBDDVars; i++) {
        permute[i] = i + numUnprimedBDDVars;
        permute[i + numUnprimedBDDVars] = i + numUnprimedBDDVars;
    }

    BDD r = bdd.Permute(permute);
    delete[] permute;
    return r;
}

//BDD Attractors::randomState(const BDD& S) const {
//    char *out = new char[numUnprimedBDDVars * 2];
//    S.PickOneCube(out);
//    std::vector<bool> values;
//    for (int i = 0; i < numUnprimedBDDVars; i++) {
//        if (out[i] == 0) {
//            values.push_back(false);
//        }
//        else {
//            values.push_back(true);
//        }
//    }
//    delete[] out;
//    return representState(values);
//}


//BDD Attractors::representState(const std::vector<bool>& values) const {
//    BDD bdd = manager.bddOne();
//    for (int i = 0; i < values.size(); i++) {
//        BDD var = manager.bddVar(i);
//        if (!values[i]) {
//            var = !var;
//        }
//        bdd *= var;
//    }
//    return bdd;
//}
BDD Attractors::randomState(const BDD& S) const {
    int bddVars = manager.ReadSize(); // this also does ADD vars annoyingly.. do we actually introduce any additional vars in score though? i don't think so.
    std::cout << "#bddVars:" << bddVars << std::endl;
    std::cout << "numUnprimedBDDVars" << numUnprimedBDDVars << std::endl;
    char *out = new char[bddVars];
    S.PickOneCube(out);

    BDD values = manager.bddOne();
    int i = 0;
    for (; i < numUnprimedBDDVars; i++) {
        BDD var = manager.bddVar(i);
        if (out[i] == 0) {
            var = !var;
        }
        values *= var;
    }
    i += numUnprimedBDDVars; // then skip over the primed ones.. is it += numUnprimedBDDVars?
    for (; i < bddVars; i++) { // then loop over anything else that remains.. the hacky bit. but it will work.
        BDD var = manager.bddVar(i);
        if (out[i] == 0) {
            var = !var;
        }
        values *= var;
    }
    
    delete[] out;
    return values;
}

void Attractors::removeInvalidBitCombinations(BDD& S) const {
    for (int var = 0; var < ranges.size(); var++) {
        if (ranges[var] > 0) {
            int b = bits(ranges[var]);
            int theoreticalMax = (1 << b) - 1;

            for (int val = ranges[var] + 1; val <= theoreticalMax; val++) {
                S *= !representUnprimedVarQN(var, val);
            }
        }
    }
}

BDD Attractors::immediateSuccessorStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
    BDD bdd = transitionBdd * valuesBdd;
    bdd = bdd.ExistAbstract(nonPrimeVariables);
    return renameRemovingPrimes(bdd);
}

BDD Attractors::forwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
    BDD reachable = manager.bddZero();
    BDD frontier = valuesBdd;

    while (!frontier.IsZero()) {
        frontier = immediateSuccessorStates(transitionBdd, frontier) * !reachable;
        reachable += frontier;
    }
    return reachable;
}

BDD Attractors::immediatePredecessorStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
    BDD bdd = renameAddingPrimes(valuesBdd);
    bdd *= transitionBdd;
    return bdd.ExistAbstract(primeVariables);
}

BDD Attractors::backwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
    BDD reachable = manager.bddZero();
    BDD frontier = valuesBdd;

    while (!frontier.IsZero()) {
        frontier = immediatePredecessorStates(transitionBdd, frontier) * !reachable;
        reachable += frontier;
    }
    return reachable;
}

std::list<BDD> Attractors::attractors(const BDD& transitionBdd, const BDD& statesToRemove) const {
    std::list<BDD> attractors;
    BDD S = manager.bddOne();
    removeInvalidBitCombinations(S);
    S *= !statesToRemove;

    while (!S.IsZero()) {
        BDD s = randomState(S);

        for (int i = 0; i < ranges.size(); i++) { // unrolling by ranges.size() may not be the perfect choice of number
            BDD sP = immediateSuccessorStates(transitionBdd, s);
            s = randomState(sP);
        }

        BDD fr = forwardReachableStates(transitionBdd, s);
        BDD br = backwardReachableStates(transitionBdd, s);

        if ((fr * !br).IsZero()) {
            attractors.push_back(fr);
        }

        S *= !(s + br);
    }
    return attractors;
}

std::string Attractors::prettyPrint(const BDD& attractor) const {
    // ideally would not use a temp file
    FILE *old = manager.ReadStdout();
    FILE *fp = fopen("temp.txt", "w");
    manager.SetStdout(fp);
    attractor.PrintMinterm();
    manager.SetStdout(old);
    fclose(fp);

    std::string out;
    std::ifstream infile("temp.txt");
    std::string line;
    auto lambda = [](const std::string& a, const std::string& b) { return a + "," + b; };
    while (std::getline(infile, line)) {
        std::list<std::string> output;
        int i = 0;
        for (int v = 0; v < ranges.size(); v++) {
            if (ranges[v] == 0) {
                //output.push_back(std::to_string(minValues[v]));
                output.push_back(std::to_string(ranges[v])); // remember i've changed this when debugging
            }
            else {
                int b = bits(ranges[v]);
                // output.push_back(fromBinary(line.substr(i, b), minValues[v]));
                output.push_back(fromBinary(line.substr(i, b), ranges[v])); // remember i've changed this when debugging
                i += b;
            }
        }

        out += std::accumulate(std::next(output.begin()), output.end(), output.front(), lambda) + "\n";
    }

    return out;
}


//inline ADD logicalEquivalence(const ADD& a, const ADD& b) {
//    //return !(a ^ b);
//    return ~(a.Xor(b));
//}
//
//
//ADD Attractors::representState(const std::vector<bool>& values) const {
//    ADD add = manager.addOne();
//    for (int i = 0; i < values.size(); i++) {
//        ADD var = manager.addVar(i);
//        if (!values[i]) {
//            //var = !var;
//            var = ~var;
//        }
//        //bdd *= var;
//        add &= var;
//    }
//    return add;
//}
//
//ADD Attractors::representNonPrimeVariables() const {
//    return representState(std::vector<bool>(numUnprimedBDDVars, true));
//}
//
//ADD Attractors::representPrimeVariables() const {
//    ADD add = manager.addOne();
//    for (int i = numUnprimedBDDVars; i < numUnprimedBDDVars * 2; i++) {
//        ADD var = manager.addVar(i);
//        //bdd *= var;
//        add &= var;
//    }
//    return add;
//}
//
//
//ADD Attractors::representUnprimedVarQN(int var, int val) const {
//    ADD add = manager.addOne();
//    int i = countBits(var);
//
//    int b = bits(ranges[var]);
//    for (int n = 0; n < b; n++) {
//        ADD var = manager.addVar(i);
//        if (!nthBitSet(val, n)) {
//            //var = !var;
//            var = ~var;
//        }
//        add &= var;
//        i++;
//    }
//
//    return add;
//}
//
//ADD Attractors::representPrimedVarQN(int var, int val) const {
//    ADD add = manager.addOne();
//    int i = numUnprimedBDDVars + countBits(var);
//
//    int b = bits(ranges[var]);
//    for (int n = 0; n < b; n++) {
//        ADD var = manager.addVar(i);
//        if (!nthBitSet(val, n)) {
//            //var = !var;
//            var = ~var;
//        }
//        add &= var;
//        //add *= var;
//        i++;
//    }
//
//    return add;
//}
//
//ADD Attractors::representStateQN(const std::vector<int>& vars, const std::vector<int>& values) const {
//    ADD add = manager.addOne();
//    for (size_t i = 0; i < vars.size(); i++) {
//        int var = vars[i];
//        int val = values[i];
//        add &= representUnprimedVarQN(var, val);
//    }
//    return add;
//}
//
//ADD Attractors::representSyncQNTransitionRelation() const {
//    ADD add = manager.addOne();
//    int v = 0;
//
//    for (int v = 0; v < ranges.size(); v++) {
//        if (ranges[v] > 0) {
//            const auto& iVars = qn.inputVars[v];
//            const auto& iValues = qn.inputValues[v];
//            const auto& oValues = qn.outputValues[v];
//
//            std::vector<ADD> states(ranges[v] + 1, manager.addZero());
//            for (int i = 0; i < oValues.size(); i++) {
//                states[oValues[i]] += representStateQN(iVars, iValues[i]);
//            }
//            for (int val = 0; val <= ranges[v]; val++) {
//                ADD vPrime = representPrimedVarQN(v, val);
//                add *= logicalEquivalence(states[val], vPrime);
//            }
//        }
//    }
//    return add;
//}
//
//ADD Attractors::renameRemovingPrimes(const ADD& add) const {
//    int *permute = new int[numUnprimedBDDVars * 2];
//    for (int i = 0; i < numUnprimedBDDVars; i++) {
//        permute[i] = i;
//        permute[i + numUnprimedBDDVars] = i;
//    }
//    ADD r = add.Permute(permute);
//    delete[] permute;
//    return r;
//}
//
//ADD Attractors::renameAddingPrimes(const ADD& add) const {
//    int *permute = new int[numUnprimedBDDVars * 2];
//    for (int i = 0; i < numUnprimedBDDVars; i++) {
//        permute[i] = i + numUnprimedBDDVars;
//        permute[i + numUnprimedBDDVars] = i + numUnprimedBDDVars;
//    }
//
//    ADD r = add.Permute(permute);
//    delete[] permute;
//    return r;
//}
//
//ADD Attractors::randomState(const ADD& S) const {
//    BDD Sbdd = S.BddPattern(); // 0/1 ADD to BDD
//
//    char *out = new char[numUnprimedBDDVars * 2];
//    Sbdd.PickOneCube(out);
//
//    std::vector<bool> values;
//    for (int i = 0; i < numUnprimedBDDVars; i++) {
//        if (out[i] == 0) {
//            values.push_back(false);
//        }
//        else {
//            values.push_back(true);
//        }
//    }
//    delete[] out;
//    return representState(values);
//}
//
//void Attractors::removeInvalidBitCombinations(ADD& S) const {
//    for (int var = 0; var < ranges.size(); var++) {
//        if (ranges[var] > 0) {
//            int b = bits(ranges[var]);
//            int theoreticalMax = (1 << b) - 1;
//
//            for (int val = ranges[var] + 1; val <= theoreticalMax; val++) {
//                //S *= !representUnprimedVarQN(var, val);
//                S *= ~representUnprimedVarQN(var, val);
//            }
//        }
//    }
//}
//
//ADD Attractors::immediateSuccessorStates(const ADD& transitionBdd, const ADD& valuesBdd) const {
//    ADD add = transitionBdd * valuesBdd;
//    bdd = bdd.ExistAbstract(nonPrimeVariables); // may have a problem here
//    return renameRemovingPrimes(bdd); 
//}
//
//BDD Attractors::forwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
//    BDD reachable = manager.bddZero();
//    BDD frontier = valuesBdd;
//
//    while (!frontier.IsZero()) {
//        frontier = immediateSuccessorStates(transitionBdd, frontier) * !reachable;
//        reachable += frontier;
//    }
//    return reachable;
//}
//
//BDD Attractors::immediatePredecessorStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
//    BDD bdd = renameAddingPrimes(valuesBdd);
//    bdd *= transitionBdd;
//    return bdd.ExistAbstract(primeVariables); // may have a problem here
//}
//
//BDD Attractors::backwardReachableStates(const BDD& transitionBdd, const BDD& valuesBdd) const {
//    BDD reachable = manager.bddZero();
//    BDD frontier = valuesBdd;
//
//    while (!frontier.IsZero()) {
//        frontier = immediatePredecessorStates(transitionBdd, frontier) * !reachable;
//        reachable += frontier;
//    }
//    return reachable;
//}
//
//std::list<BDD> Attractors::attractors(const BDD& transitionBdd, const BDD& statesToRemove) const {
//    std::list<BDD> attractors;
//    BDD S = manager.bddOne();
//    removeInvalidBitCombinations(S);
//    S *= !statesToRemove;
//
//    while (!S.IsZero()) {
//        BDD s = randomState(S);
//
//        for (int i = 0; i < ranges.size(); i++) { // unrolling by ranges.size() may not be the perfect choice of number
//            BDD sP = immediateSuccessorStates(transitionBdd, s);
//            s = randomState(sP);
//        }
//
//        BDD fr = forwardReachableStates(transitionBdd, s);
//        BDD br = backwardReachableStates(transitionBdd, s);
//
//        if ((fr * !br).IsZero()) {
//            attractors.push_back(fr);
//        }
//
//        S *= !(s + br);
//    }
//    return attractors;
//}