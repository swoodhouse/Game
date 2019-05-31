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
	//std::vector<int> permute(Cudd_ReadNodeCount(manager.getManager()));
	//std::vector<int> permute(Cudd_ReadSize(manager.getManager()));
	std::vector<int> permute(numBDDVars);
	std::iota(permute.begin(), permute.end(), 0);

	for (int i = 0; i < numUnprimedBDDVars; i++) {
		permute[i + numUnprimedBDDVars] = i;
	}

	return bdd.Permute(&permute[0]);
}

BDD Attractors::renameAddingPrimes(const BDD& bdd) const {
	std::vector<int> permute(numBDDVars);
	//std::vector<int> permute(Cudd_ReadNodeCount(manager.getManager()));
	//std::vector<int> permute(Cudd_ReadSize(manager.getManager()));
	std::iota(permute.begin(), permute.end(), 0);

	for (int i = 0; i < numUnprimedBDDVars; i++) {
		permute[i] = i + numUnprimedBDDVars;
	}

	return bdd.Permute(&permute[0]);
}

//
//BDD Attractors::renameRemovingPrimes(const BDD& bdd) const {
//	std::vector<int> permute(Cudd_ReadNodeCount(manager.getManager()));
//	std::iota(permute.begin(), permute.end(), 0);
//
//	for (int i = 0; i < numUnprimedBDDVars; i++) {
//		permute[i + numUnprimedBDDVars] = i;
//	}
//
//	return bdd.Permute(&permute[0]);
//}
//
//BDD Attractors::renameAddingPrimes(const BDD& bdd) const {
//	std::vector<int> permute(Cudd_ReadNodeCount(manager.getManager()));
//	std::iota(permute.begin(), permute.end(), 0);
//
//	for (int i = 0; i < numUnprimedBDDVars; i++) {
//		permute[i] = i + numUnprimedBDDVars;
//	}
//
//	return bdd.Permute(&permute[0]);
//}
//
//// TODO: switch between variablesToIngore and variablesToKeep implementations, verify they are equivalent and it is just that
//// the later is more efficent
//BDD Attractors::randomState(const BDD& S, const std::unordered_set<int>& variablesToIgnore, const BDD& variablesToKeep) const {
//	std::cout << "here random 1" << std::endl;
//    char *out = new char[Cudd_ReadNodeCount(manager.getManager())];
//	std::cout << "here random 2" << std::endl;
//    S.PickOneCube(out);
//	std::cout << "here random 3" << std::endl;
//	BDD bdd = manager.bddOne();
//	std::cout << "here random 4" << std::endl;
//    for (int i = 0; i < numUnprimedBDDVars; i++) {
//		BDD var = manager.bddVar(i);
//        if (out[i] == 0) {
//			var = !var;
//        }
//		bdd *= var;
//    }
//	std::cout << "here random 5" << std::endl;
//
//	// modification made for Game - we need to be able to represent random choices of mutations, treaments too
//	// now actually think this was a mistake, turned off for now
//	/*for (int i = numUnprimedBDDVars * 2; i < Cudd_ReadNodeCount(manager.getManager()); i++) {
//		if (variablesToIgnore.find(i) != variablesToIgnore.end()) {
//			BDD var = manager.bddVar(i);
//			if (out[i] == 0) {
//				var = !var;
//			}
//			bdd *= var;
//		}
//	}*/
//
//	// i think what we actually want is variables to keep - ......
//
//    delete[] out;
//	std::cout << "here random 6" << std::endl;
//	//return bdd;
//	return bdd * variablesToKeep; //TODO: switch between variablesToIngore and variablesToKeep implementations
//}

// BDD Attractors::randomState(const BDD& S) const {
//     int bddVars = manager.ReadSize(); // this also does ADD vars annoyingly.. do we actually introduce any additional vars in score though? i don't think so.
//     std::cout << "#bddVars:" << bddVars << std::endl;
//     std::cout << "numUnprimedBDDVars" << numUnprimedBDDVars << std::endl;
//     char *out = new char[bddVars];
//     S.PickOneCube(out);

//     BDD values = manager.bddOne();
//     int i = 0;
//     for (; i < numUnprimedBDDVars; i++) {
//         BDD var = manager.bddVar(i);
//         if (out[i] == 0) {
//             var = !var;
//         }
//         values *= var;
//     }
//     i += numUnprimedBDDVars; // then skip over the primed ones.. is it += numUnprimedBDDVars?
//     for (; i < bddVars; i++) { // then loop over anything else that remains.. the hacky bit. but it will work.
//         BDD var = manager.bddVar(i);
//         if (out[i] == 0) {
//             var = !var;
//         }
//         values *= var;
//     }
    
//     delete[] out;
//     return values;
// }

/*
// original
BDD Attractors::randomState(const BDD& S) const {
    char *out = new char[numUnprimedBDDVars * 2];
    S.PickOneCube(out);
    std::vector<bool> values;
    for (int i = 0; i < numUnprimedBDDVars; i++) {
        if (out[i] == 0) {
            values.push_back(false);
        }
        else {
            values.push_back(true);
        }
    }
    delete[] out;
    return representState(values);
}*/
//
//std::vector<int> permute(Cudd_ReadNodeCount(attractors.manager.getManager()));
//std::iota(permute.begin(), permute.end(), 0);
//
//int i = attractors.numUnprimedBDDVars * 2; // refactor out
//int j = i + bits(oeVars.size() + 1) + numMutations * 2 * bits(koVars.size() + 1) + level * bits(oeVars.size() + 1);
//
//for (int n = 0; n < bits(oeVars.size() + 1); n++) { // duplication
//	permute[n + i] = n + j;
//}
//
//return states.Permute(&permute[0]);
//
//BDD Attractors::randomState(const BDD& S) const {
//	//char *out = new char[Cudd_ReadSize(manager.getManager())];
//	// TEMP HACK, THE ABOVE DOESN'T GIVE THE RIGHT NUMBER
//	char *out = new char[49];
//	
//	//std::cout << "Cudd_ReadSize(manager.getManager()): " << Cudd_ReadSize(manager.getManager()) << std::endl;
//	S.PickOneCube(out);
//	std::vector<bool> values;
//	for (int i = 0; i < numUnprimedBDDVars; i++) {
//		if (out[i] == 0) {
//			//std::cout << false << std::endl;
//			values.push_back(false);
//		}
//		else {
//			//std::cout << true << std::endl;
//			values.push_back(true);
//		}
//	}
//	//std::cout << "before delete" << std::endl;
//
//	delete[] out; // temp
//
//	//std::cout << "after delete" << std::endl;
//	BDD temp = representState(values);
//
//	//std::cout << "after temp" << std::endl;
//
//	return temp;
//}
//

BDD Attractors::randomState(const BDD& S) const {
	BDD bdd = manager.bddOne();

	char *out = new char[numBDDVars];
	//char *out = new char[Cudd_ReadNodeCount(manager.getManager())];
	//char *out = new char[Cudd_ReadNodeCount(manager.getManager())];  // does this give the right number.........
	//char *out = new char[Cudd_ReadSize(manager.getManager())]; // Cudd_ReadSize seems like it should actually be correct.. that's not what permute is using though..
															   //char *out = new char[numBddVars]; // 50?
	S.PickOneCube(out);

	for (int i = 0; i < numUnprimedBDDVars; i++) {
		if (out[i] == 0) bdd *= !manager.bddVar(i);
		else bdd *= manager.bddVar(i);
	}

	delete[] out; // temp

	return bdd;
}

//
//// hack, getting to work with mut vars, etc
//BDD Attractors::randomState(const BDD& S/*,const std::vector<int> indicesTokeep, int numBddVars*/) const {
//	//char *out = new char[Cudd_ReadSize(manager.getManager())];
//	// TEMP HACK, THE ABOVE DOESN'T GIVE THE RIGHT NUMBER
//	BDD bdd = manager.bddOne();
//
//	char *out = new char[59]; // 50?
//	//char *out = new char[49]; // 50?
//	//char *out = new char[300]; // 50?
//
//	S.PickOneCube(out);
//
//	for (int i = 0; i < numUnprimedBDDVars; i++) { // change to only keep indices i care about
//		if (out[i] == 0) {
//			bdd *= !manager.bddVar(i);
//		}
//		else {
//			bdd *= manager.bddVar(i);
//		}
//	}
//
//	// hard coded treatment and mut vars for one specific model
//	//for (int i = 32; i <= 37; i++) { // change to only keep indices i care about
//	//	if (out[i] == 0) {
//	//		bdd *= !manager.bddVar(i);
//	//	}
//	//	else {
//	//		bdd *= manager.bddVar(i);
//	//	}
//	//}
//
//	delete[] out; // temp
//
//	return bdd;
//}
//


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

//std::list<BDD> Attractors::attractors(const BDD& transitionBdd, const BDD& statesToRemove, const std::unordered_set<int>& variablesToIgnore,
//	                                  const BDD& variablesToKeep) const {
//    std::list<BDD> attractors;
//    BDD S = manager.bddOne();
//    removeInvalidBitCombinations(S);
//    S *= !statesToRemove;
//
//    while (!S.IsZero()) {
//        BDD s = randomState(S, variablesToIgnore, variablesToKeep);
//
//        for (int i = 0; i < ranges.size(); i++) { // unrolling by ranges.size() may not be the perfect choice of number
//            BDD sP = immediateSuccessorStates(transitionBdd, s);
//            s = randomState(sP, variablesToIgnore, variablesToKeep);
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

// i don't think this is an optimal implementation, will do repeated work - if state a is ruled out under mutations X but not Y, will be run again under X and Y
std::list<BDD> Attractors::attractors(const BDD& transitionBdd, const BDD& statesToRemove, const BDD& statesToKeep /*, const BDD& variablesToAdd*/) const {
	    std::list<BDD> attractors;
	    BDD S = manager.bddOne();
	    removeInvalidBitCombinations(S);
	    S *= !statesToRemove;

	    while (!S.IsZero()) {
			BDD s = randomState(S) * statesToKeep; // threading numBDDVArs through
			//BDD s = randomState(S) * statesToKeep; // new idea
			//BDD s = randomState(S);
			//BDD s = randomState(S); // *variablesToAdd; // variab
	
	        for (int i = 0; i < ranges.size() /*10000*/; i++) { // unrolling by ranges.size() may not be the perfect choice of number
	            BDD sP = immediateSuccessorStates(transitionBdd, s); // variables to add here???
				s = randomState(sP) * statesToKeep; // new idea
																	 //s = randomState(sP) * statesToKeep; // new idea
	        }

	        BDD fr = forwardReachableStates(transitionBdd, s);
	        BDD br = backwardReachableStates(transitionBdd, s);

			//std::cout << "fr:" << std::endl;
			//fr.PrintMinterm();

			//std::cout << "br:" << std::endl;
			//br.PrintMinterm();
	
	        if ((fr * !br).IsZero()) {
				//std::cout << "pushing attractor" << std::endl; // ok.. so we keep hitting this..

	            attractors.push_back(fr);
	        }
			//std::cout << "here5" << std::endl;
	
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
