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

// this is really unprimed qn vars
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


// another alternative. no longer random
// THIS DOESN'T WORK
// BDD Attractors::randomState(const BDD& S) const {
//   BDD rnd = S.LargestCube();//  S.PickOneMinterm(vars);
  
//   BDD notUnprimedQnVars = manager.bddOne();
//   for (int i = numUnprimedBDDVars; i < numBDDVars; i++) {
//     notUnprimedQnVars *= manager.bddVar(i);
//   }
//   return rnd.ExistAbstract(notUnprimedQnVars);
// }


BDD Attractors::randomState(const BDD& S) const {
  std::vector<BDD> vars;
  for (int i = 0; i < numBDDVars; i++) {
    BDD v = manager.bddVar(i);
    vars.push_back(v);
  }
  //  std::cout << "in randomState, a" << std::endl;
  BDD rnd = S.PickOneMinterm(vars);
  //std::cout << "in randomState, b" << std::endl;
  
  BDD notUnprimedQnVars = manager.bddOne();
  for (int i = numUnprimedBDDVars; i < numBDDVars; i++) {
    notUnprimedQnVars *= manager.bddVar(i);
  }
  return rnd.ExistAbstract(notUnprimedQnVars);
}

//     BDD
// BDD::PickOneMinterm(
//   std::vector<BDD> vars) const
// {


// BDD Attractors::randomState(const BDD& S) const {
//   BDD bdd = manager.bddOne();


//   // try this hack...........
//   std::cout << "in randomState, 1" << std::endl;
//   //char *out = new char[numBDDVars * 100]; // i think maybe switch to c allocation..
//   //...

//   //std::vector<char> out(numBDDVars);

//   //  char *out = (char *) malloc(numBDDVars * sizeof(char)); // try calloc too
//   char *out = (char *) calloc(numBDDVars, sizeof(char)); // try calloc too

//   // hmm maybe i can use pickoneminterm.. is a cube = to a minterm
//     // or std::vector<char> out(numBDDVars);
//     // pickonecube(&out[0]);
//   // return bdd.Permute(&permute[0]);
//     // not sure if the above can work. or use malloc. or.. use a static array
//   std::cout << "in randomState, 2" << std::endl;
//   //char *out = new char[numBDDVars];
//   S.PickOneCube(out); // this exact line is crashing

//   // temp..................
//   //S.PickOneCube(&out[0]); // this exact line is crashing
  
//   std::cout << "in randomState, 3" << std::endl;

//   for (int i = 0; i < numUnprimedBDDVars; i++) {
//     if (out[i] == 0) bdd *= !manager.bddVar(i);
//     else bdd *= manager.bddVar(i);
//   }
//   std::cout << "in randomState, 4" << std::endl;
  
//   //delete[] out; // temp
//   free(out);

//   std::cout << "in randomState, 5" << std::endl;
//   return bdd;
// }

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

std::list<BDD> Attractors::attractors(const BDD& transitionBddFwd, const BDD& transitionBddBwd, const BDD& statesToRemove) const {
  std::list<BDD> attractors;
  BDD S = manager.bddOne();
  removeInvalidBitCombinations(S);
  S *= !statesToRemove;

  while (!S.IsZero()) {
    BDD s = randomState(S) * S; // pick a random state in the QN variables only, reattach all possible configurations of mutations

    //std::cout << "a random state:" << std::endl;
    //s.PrintMinterm(); // temp
    
    
    // crashing in here then
    // for (std::vector<int>::size_type i = 0; i < ranges.size()/* * 1000*/; i++) { // unrolling by ranges.size() may not be the perfect choice of number, especially for Game

    //   BDD sP = immediateSuccessorStates(transitionBddFwd, s);

    //   //so it's crashing here............. which is weird.. because that should have zero dependence on bwd bdd
    //   // it could be that the array is the wrong size now..
    //   //s = randomState(sP) * S; // Nir thinks there may be a problem.. maybe here I should expand back out to N states
    //   // I think I agree.. the first randomstate above is correct, but then it goes in other directions
    //   // this should be
    //   // actually this second random is redundant for synch qns. in asynch would have to pick a single state, per mutation
    //   s = sP; // actually maybe calling randomstate was not incorrect, but it is not optimal in our sync case
    // }

    // here we unroll until we hit an attractor.. this is wrong though........
    // in this implementation i really do need a fwd and a bwd attractor, because 
    BDD reached = manager.bddZero();
    while (!((s * !reached).IsZero())) { // while s - reached = 0
      reached += s;
      s = immediateSuccessorStates(transitionBddFwd, s);
    }

    
    BDD fr = forwardReachableStates(transitionBddFwd, s);
    BDD br = backwardReachableStates(transitionBddBwd, s); // if i use the bwd bdd here i get repeated attractors. is it because back(false) is wierd?

    // this still needed? this is to remove the special variables -
    // this is to be careful about intersecting states from different mutation configurations
    // BDD variablesToKeepNonZero = fr.ExistAbstract(nonPrimeVariables) * br.ExistAbstract(nonPrimeVariables); // this is chosen and mutation that are not zero in both fr and br - those that still have some states remaining
    // BDD frIntersected = fr * variablesToKeepNonZero;
    // BDD brIntersected = br * variablesToKeepNonZero;

    // TODO: document this code
    // seems like we don't need brIntersected. frIntersected * !br would work
    //if ((frIntersected * !brIntersected).IsZero()) { // fr * !br == 0 // seems like this isn't needed??
    if ((fr * !br).IsZero()) {
      // temp, a set would be better. remove this, should never hit
      //if (std::find(attractors.begin(), attractors.end(), frIntersected) == attractors.end()) {
      if (std::find(attractors.begin(), attractors.end(), fr) == attractors.end()) {
	attractors.push_back(fr); // is this right? // doesn't this contain additional unwanted states?
	//attractors.push_back(frIntersected); // is this right? // doesn't this contain additional unwanted states?
      }
      else {
	std::cout << "repeated attractor" << std::endl;

	// std::cout << "frIntersected:" << std::endl;
	// frIntersected.PrintMinterm();

	// std::cout << "fr:" << std::endl;
	// fr.PrintMinterm();

	// std::cout << "brIntersected:" << std::endl;
	// brIntersected.PrintMinterm();

	// std::cout << "br:" << std::endl;
	// br.PrintMinterm();
      }
    }
    else {
      std::cout << "NOT ATTRACTOR" << std::endl;
    }

    //check here that S != old_S and s not equal old_s
    
    S *= !(s + br);
  }
  return attractors;
}

// statesToKeep no longer used
// std::list<BDD> Attractors::attractors(const BDD& transitionBdd, const BDD& statesToRemove, const BDD& statesToKeep) const {
//   std::list<BDD> attractors;
//   BDD S = manager.bddOne();
//   removeInvalidBitCombinations(S);
//   S *= !statesToRemove;

//   int i = 0;
//   while (!S.IsZero()) {
//     //std::cout << "iteration " << i << std::endl;
//     i++;
//     BDD s = randomState(S) * S;

//     // TEMP!
//     for (std::vector<int>::size_type i = 0; i < ranges.size()/* * 1000*/; i++) { // unrolling by ranges.size() may not be the perfect choice of number, especially for Game
//       BDD sP = immediateSuccessorStates(transitionBdd, s);
//       s = randomState(sP) * S;
//     }

//     BDD fr = forwardReachableStates(transitionBdd, s);
//     BDD br = backwardReachableStates(transitionBdd, s);

//     BDD variablesToKeepNonZero = fr.ExistAbstract(nonPrimeVariables) * br.ExistAbstract(nonPrimeVariables);
//     BDD frIntersected = fr * variablesToKeepNonZero;
//     BDD brIntersected = br * variablesToKeepNonZero;

//     // seems like we don't need brIntersected. frIntersected * !br would work
//     if ((frIntersected * !brIntersected).IsZero()) {
//       // temp, a set would be better. remove this, should never hit
//       if (std::find(attractors.begin(), attractors.end(), frIntersected) == attractors.end()) {
// 	attractors.push_back(frIntersected); // is this right?
//       }
//       else {
// 	std::cout << "repeated attractor" << std::endl;
//       }
//     }

//     S *= !(s + br);
//   }
//   return attractors;
// }

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
