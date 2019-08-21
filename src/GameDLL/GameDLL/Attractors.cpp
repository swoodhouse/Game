// Copyright (c) Microsoft Research 2017
// License: MIT. See LICENSE

#include "stdafx.h"
#include "Attractors.h"

BDD logicalEquivalence(const BDD& a, const BDD& b) {
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
	std::cout << "hereZ" << std::endl;
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

int Attractors::countBits(int end) const {
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
	std::vector<int> permute(numBDDVars);
	std::iota(permute.begin(), permute.end(), 0);

	for (int i = 0; i < numUnprimedBDDVars; i++) {
		permute[i + numUnprimedBDDVars] = i;
	}

	return bdd.Permute(&permute[0]);
}

BDD Attractors::renameAddingPrimes(const BDD& bdd) const {
	std::vector<int> permute(numBDDVars);
	std::iota(permute.begin(), permute.end(), 0);

	for (int i = 0; i < numUnprimedBDDVars; i++) {
		permute[i] = i + numUnprimedBDDVars;
	}

	return bdd.Permute(&permute[0]);
}

BDD Attractors::randomState(const BDD& S) const {
	BDD bdd = manager.bddOne();

	char *out = new char[numBDDVars];
	S.PickOneCube(out);

	for (int i = 0; i < numUnprimedBDDVars; i++) {
		if (out[i] == 0) bdd *= !manager.bddVar(i);
		else bdd *= manager.bddVar(i);
	}

	delete[] out; // temp

	return bdd;
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
    // std::cout << "valuesBdd:" << std::endl;
    // valuesBdd.PrintMinterm();
    BDD bdd = renameAddingPrimes(valuesBdd);


    // std::cout << "after renameAddingPrimes" << std::endl;
    // bdd.PrintMinterm();
    
    bdd *= transitionBdd;

    // std::cout << "after * transitionBdd" << std::endl;
    // bdd.PrintMinterm();

    // std::cout << "after exist" << std::endl;
    // bdd.ExistAbstract(primeVariables).PrintMinterm();
    
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

// i don't think this is an optimal implementation, will do repeated work - if state a is ruled out under mutations X but not Y, will be run again under X and Y
std::list<BDD> Attractors::attractors(const BDD& transitionBdd, const BDD& statesToRemove, const BDD& statesToKeep /*, const BDD& variablesToAdd*/) const {
	    std::list<BDD> attractors;
	    BDD S = manager.bddOne();
	    removeInvalidBitCombinations(S);
	    S *= !statesToRemove;
	    //S *= statesToKeep;

	    // std::cout << "S, starting:" << std::endl;
	    // S.PrintMinterm();
	    
	    while (!S.IsZero()) {
	      //std::cout << "calling randomState" << std::endl;
			BDD s = randomState(S) * statesToKeep; // threading numBDDVArs through
			if (s.IsZero()) std::cout << "random makes zero" << std::endl;
			//BDD s = randomState(S) * statesToKeep; // new idea
			//BDD s = randomState(S);
			//BDD s = randomState(S); // *variablesToAdd; // variab
	
	        for (int i = 0; i < ranges.size() /*10000*/; i++) { // unrolling by ranges.size() may not be the perfect choice of number, especially for Game
	            BDD sP = immediateSuccessorStates(transitionBdd, s); // variables to add here???
		    s = randomState(sP) * statesToKeep; // new idea
		    if (s.IsZero()) std::cout << "random2 makes zero" << std::endl;																	 //s = randomState(sP) * statesToKeep; // new idea
	        }

		//		std::cout << "s.iszero:" << s.IsZero() << std::endl;
		
	        BDD fr = forwardReachableStates(transitionBdd, s);
	        BDD br = backwardReachableStates(transitionBdd, s);


		// std::cout << "fr" << std::endl;
		// fr.PrintMinterm();
		// std::cout << "br" << std::endl;
		// br.PrintMinterm();
		// std::cout << "statesToKeep:" << std::endl;
		// statesToKeep.PrintMinterm();
		// std::cout << "s:" << std::endl;
		// s.PrintMinterm();
		
	        if ((fr * !br).IsZero()) {
		  //std::cout << "pushing attractor" << std::endl; // ok.. so we keep hitting this..

	            attractors.push_back(fr);
	        }
		// else {
		//   std::cout << "(fr * !br):" << std::endl;
		//   (fr * !br).PrintMinterm();
		// }
			//std::cout << "here5" << std::endl;

		// std::cout << "S:" << std::endl;
		// S.PrintMinterm();
		
		// std::cout << "S * !(s + br):" << std::endl;
		// (S * !(s + br)).PrintMinterm();

		// std::cout << "s:" << std::endl;
		// (s).PrintMinterm();
		
		// std::cout << "!(s + br):" << std::endl;
		// (!(s + br)).PrintMinterm();

		
	        S *= !(s + br);
		// if (S.IsZero()) {
		//   std::cout << "S is zero" << std::endl;
		  
		// }
		//		std::cout << "S.iszero:" << S.IsZero() << std::endl;
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
                output.push_back(std::to_string(minValues[v]));
            }
            else {
                int b = bits(ranges[v]);
                output.push_back(fromBinary(line.substr(i, b), minValues[v]));
                i += b;
            }
        }

        out += std::accumulate(std::next(output.begin()), output.end(), output.front(), lambda) + "\n";
    }

    return out;
}
