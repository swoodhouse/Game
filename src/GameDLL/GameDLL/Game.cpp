// heavy refactoring needed. an on Attractors.cpp too i think

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

/*inline*/ BDD logicalImplication(const BDD& a, const BDD& b) {
  return (!a) + b;
}

int Game::calcNumMutations(int height, bool maximisingPlayerGoesLast) {
  if (height <= 1) return 0;
  else if (height == 2) {
    if (maximisingPlayerGoesLast) return 0;
    return 1;
  }
  else if (height == 3) return 1;
  else if (height == 4) {
    if (maximisingPlayerGoesLast) return 1;
    return 2;
  }
  else if (height == 5) return 2;

  throw std::runtime_error("height > 5 not implemented");
}

int Game::calcNumTreatments(int height, bool maximisingPlayerGoesLast) { // wrong
  std::cout << "in calc. height = " << height << std::endl;
  std::cout << "in calc. maximisingLast = " << maximisingPlayerGoesLast<< std::endl;
  if (height <= 1) return 0;
  else if (height == 2) {
    if (maximisingPlayerGoesLast) return 1;
    return 0;
  }
  else if (height == 3) return 1;
  else if (height == 4) {
    if (maximisingPlayerGoesLast) return 2;
    return 1;
  }
  else if (height == 5) return 2;

  throw std::runtime_error("height > 5 not implemented");
}

std::vector<int> Game::attractorsIndicies() const {
  std::vector<int> v(this->numUnprimedBDDVars * 2);
  std::iota(v.begin(), v.end(), 0);
  return v;
}

std::vector<int> Game::treatmentVarIndices() const {
  std::vector<int> v(bits(oeVars.size() + 1));
  std::iota(v.begin(), v.end(), attractorsIndicies().back() + 1);
  return v;
}

std::vector<int> Game::mutationVarsIndices() const {
  int n = numMutations;

  if (numMutations == 0) {
    std::cout << "numMutations == 0, this will trigger a bug" << std::endl; // temp
    n = 1;
  }

  std::vector<int> v(n * bits(koVars.size() + 1));
  std::iota(v.begin(), v.end(), treatmentVarIndices().back() + 1);
  return v;
}

std::vector<int> Game::chosenTreatmentsIndices() const {
  int n = numTreatments;
  if (numTreatments == 0) {
    std::cout << "numTreatments == 0, this will trigger a bug" << std::endl; // temp
    n = 1;
  }
  
  std::vector<int> v(n * bits(oeVars.size() + 1)); // don't actually need +1 because don't need to represent zero, but easier this way
  std::iota(v.begin(), v.end(), mutationVarsIndices().back() + 1);
  return v;
}

std::vector<int> Game::chosenMutationsIndices() const {
  int n = numMutations;
  if (numMutations == 0) {
    std::cout << "numMutations == 0, this will trigger a bug" << std::endl; // temp
    n = 1;
  }

  std::vector<int> v(n * bits(koVars.size() + 1)); // don't actually need +1 because don't need to represent zero, but easier this way
  // TODO: clean this up
  auto temp = chosenTreatmentsIndices();
  auto temp2 = temp.back(); // problem is here.........
  auto temp3 = temp2 + 1;
  std::iota(v.begin(), v.end(), temp3);
  return v;
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

BDD Game::representSomeMutation(int var) const {
  BDD bdd = attractors.manager.bddZero();

  for (std::vector<int>::size_type i = 0; i < koVars.size(); i++) {
    bdd += representMutation(var, i);
  }

  return bdd;
}

BDD Game::nMutations(int n) const {
  if (n == 0) {
    if (numMutations == 2) {
      return representMutationNone(0) * representMutationNone(1);
    }
    else if (numMutations == 1) {
      return representMutationNone(0);
    }
    else {
      std::cout << "nmutations > 2 not implemented" << std::endl;
      throw std::runtime_error("nmutations > 2 not implemented");
    }
  }
  else if (n == 1) {
    if (numMutations == 2) {
      return representSomeMutation(0) * representMutationNone(1); // new - lexicographically ordred
    }
    else if (numMutations == 1) {
      return representSomeMutation(0);
    }
    else {
      std::cout << "nmutations > 2 not implemented" << std::endl;
      throw std::runtime_error("nmutations > 2 not implemented");
    }
  }
  else if (n == 2 && numMutations > 1) {
    return (representSomeMutation(0) * representSomeMutation(1));
  }
  else {
    std::cout << "nmutations > 2 not implemented" << std::endl;
    throw std::runtime_error("nmutations > 2 not implemented");
  }
}

ADD Game::untreat(int level, const ADD& states) const {
  // this has the effect of remembering the treatment by storing it in remember@level and removing treatment var

  std::vector<int> permute(chosenMutationsIndices().back() + 1);
  std::iota(permute.begin(), permute.end(), 0);

  int b = bits(oeVars.size() + 1);
  int i = treatmentVarIndices().front();
  int j = chosenTreatmentsIndices().front() + level * b;

  for (int n = 0; n < b; n++) { // duplication
    permute[n + i] = n + j;
  }

  return states.Permute(&permute[0]);
}

ADD Game::unmutate(int level, const ADD& states) const {
  std::vector<int> permute(chosenMutationsIndices().back() + 1);
  std::iota(permute.begin(), permute.end(), 0);

  int b = bits(koVars.size() + 1);
  int i = mutationVarsIndices().front() + level * b;
  int j = chosenMutationsIndices().front() + level * b;
  for (int n = 0; n < b; n++) { // duplication
    permute[n + i] = n + j;
  }

  return states.Permute(&permute[0]);
}

BDD Game::buildMutantSyncQNTransitionRelation() const {
  BDD tr = attractors.manager.bddOne();

  std::cout << "in buildMutantTR" << std::endl;

  auto components = connectedComponents();
  std::cout << "num connected components: " << components.size() << std::endl;

  // do i want to sort in ascending or descending order? ascending
  std::sort(components.begin(), components.end(),
   	    [](const std::vector<std::vector<int>::size_type>& a,
   	       const std::vector<std::vector<int>::size_type>& b){ return a.size() < b.size(); });

  auto start = std::chrono::steady_clock::now();
 
  int vars_done = 0;
  int comp_num = 0;
  for (auto comp : components) {
    BDD bdd = attractors.manager.bddOne();
    for (auto v : comp) {
      std::cout << "variables done:" << vars_done << std::endl;
      vars_done++;
      std::cout << "node " << v << std::endl;
      if (attractors.ranges[v] > 0) {
        const auto& iVars = attractors.qn.inputVars[v];
        const auto& iValues = attractors.qn.inputValues[v];
        const auto& oValues = attractors.qn.outputValues[v];
        std::vector<BDD> states(attractors.ranges[v] + 1, attractors.manager.bddZero());
        for (std::vector<int>::size_type i = 0; i < oValues.size(); i++) {
	  states[oValues[i]] += attractors.representStateQN(iVars, iValues[i]);
        }

        BDD targetFunction = attractors.manager.bddOne();

        for (int val = 0; val <= attractors.ranges[v]; val++) {
	  BDD vPrime = attractors.representPrimedVarQN(v, val);
	  targetFunction *= logicalEquivalence(states[val], vPrime);
        }
     
        auto koIt = std::find(koVars.begin(), koVars.end(), v);
        
        if (koIt != koVars.end()) { // rename to mutation vars - oe-ing not ko-ing
          int k = std::distance(koVars.begin(), koIt);  
	  BDD isMutated = attractors.manager.bddZero();
				
	  for (int lvl = 0; lvl < numMutations; lvl++) {
	    isMutated += representMutation(lvl, k);
	  }
  				
	  int max = attractors.ranges[v];
	  //bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, max), targetFunction);
	  // temp! hard coding to one to match mathew's benchmark model
	  bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 1), targetFunction);
	}
	else {
          auto oeIt = std::find(oeVars.begin(), oeVars.end(), v);
          if (oeIt != oeVars.end()) { // rename to treat vars - koing not oe-ing
            int o = std::distance(oeVars.begin(), oeIt);        
	    BDD isTreated = representTreatment(o);
	    bdd *= isTreated.Ite(attractors.representPrimedVarQN(v, 0), targetFunction);
          }
	  else {
	    bdd *= targetFunction;
          }
        }
      }
    }
    auto diff = std::chrono::steady_clock::now() - start;
    std::cout << "total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;

    std::cout << "adding connected component " << comp_num << std::endl;
    comp_num++;
    tr *= bdd;

    diff = std::chrono::steady_clock::now() - start;
    std::cout << "total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
  }

  return tr;
}

std::vector<std::vector<std::vector<int>::size_type>> Game::connectedComponents() const {
  boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> graph;
  for (std::vector<std::vector<int>>::size_type i = 0; i < attractors.qn.inputVars.size(); i++) {
    for (int j : attractors.qn.inputVars[i]) boost::add_edge(j, i, graph); // flipping direction of these edges should make no difference
  }
    
  std::vector<int> component(boost::num_vertices(graph));
  int num = boost::strong_components(graph, &component[0]);

  std::vector<std::vector<std::vector<int>::size_type>> ret(num);

  for (std::vector<int>::size_type i = 0; i < component.size(); i++) {
    std::cout << "Vertex " << i
	      << " is in component " << component[i] << std::endl;
    ret[component[i]].push_back(i); // need to make sure the order of the graph indices matches my indices
  }
  return ret;
}

ADD Game::renameBDDVarsAddingPrimes(const ADD& add) const {
  int *permute = new int[chosenMutationsIndices().back() + 1];
  int i = 0;
  for (; i < attractors.numUnprimedBDDVars; i++) {
    permute[i] = i + attractors.numUnprimedBDDVars;
    permute[i + attractors.numUnprimedBDDVars] = i + attractors.numUnprimedBDDVars;
  }

  for (; i < chosenMutationsIndices().back() + 1; i++) {
    permute[i] = i;
  }

  ADD r = add.Permute(permute);
  delete[] permute;
  return r;
}

ADD Game::renameBDDVarsRemovingPrimes(const ADD& add) const {
  //std::vector<int> permute(attractors.numBDDVars); // this was wrong
  std::vector<int> permute(chosenMutationsIndices().back() + 1);
  std::iota(permute.begin(), permute.end(), 0);

  for (int i = 0; i < attractors.numUnprimedBDDVars; i++) {
    permute[i + attractors.numUnprimedBDDVars] = i;
  }

  return add.Permute(&permute[0]);
}

// want also immediateForwardMean. Not for sync networks, there is only one possible successor (and predecessor if you are in a loop)
ADD Game::immediateForwardMax(const ADD& states) const {
  ADD add = mutantTransitionRelation.Add() * states;
  add = add.MaxAbstract(attractors.nonPrimeVariables.Add());
  return renameBDDVarsRemovingPrimes(add);
}

ADD Game::immediateBackMax(const ADD& states) const {
  ADD add = renameBDDVarsAddingPrimes(states);
  add *= mutantTransitionRelation.Add();
  return add.MaxAbstract(attractors.primeVariables.Add());
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

// a Mean version of this should be easy - CUDD has no mean, but you can either implement that or do scored = (scored + fr) / constant(2)
ADD Game::scoreLoop(const BDD& loop, const ADD& scoreRelation) const {
  ADD scored = loop.Add() * scoreRelation;

  for (std::vector<int>::size_type i = 0; i < attractors.ranges.size(); i++) { // ranges.size is temp.. need to make work for all loop sizes
    ADD fr = immediateForwardMax(scored);
    scored = scored.Maximum(fr);
    // scored = (scored + fr).Divide(attractors.manager.constant(2)); // mean version
  }

  return scored;
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
    for (std::vector<int>::size_type v = 0; v < attractors.ranges.size(); v++) {
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
    // get the score value
    std::string rest = std::to_string(std::stoi(line.substr(i)) - 1); // subtract 1 back off add value (0 is nothing, so 1 is score of 0)
    output.push_back(rest); // trim this

    out += std::accumulate(std::next(output.begin()), output.end(), output.front(), lambda) + "\n";
  }

  return out;
}

ADD Game::scoreAttractors(bool applyTreatments, int numMutations) const {
  ADD states = attractors.manager.addZero();
  BDD treatment = applyTreatments ? representSomeTreatment() : representTreatmentNone();
	
  BDD mutsAndTreats = treatment * nMutations(numMutations);

  // std::cout << "mutationsBDD:" << std::endl;
  // nMutations(numMutations).PrintMinterm();
  
  BDD statesToRemove = !mutsAndTreats;
  std::list<BDD> loops = attractors.attractors(mutantTransitionRelation, statesToRemove, mutsAndTreats);
  std::cout << "loops.len:" << loops.size() << std::endl; // 64..?

  // temp............................ print these out to debug
  // int j = 0;
  // for (auto loop : loops) {
  //   std::cout << "loop " << j << ":" << std::endl;
  //   j++;
  //   loop.PrintMinterm();
  // }
  
  int i = 0;
  for (const BDD& a : loops) {
    ADD scored = scoreLoop(a, scoreRelation);
    states = states.Maximum(scored);
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

// this isn't really doing minimax, it's computing the game tree
ADD Game::minimax() const {
  std::cout << "\n\n\nin minimax" << std::endl;
  int height = this->height;
  int numTreatments = this->numTreatments;
  int numMutations = this->numMutations;
  bool maximisingPlayer = this->maximisingPlayerLast;

  maximisingPlayer = false; // temp..............

  std::cout << "maximisingPlayer:" << maximisingPlayer << std::endl;
  std::cout << "height:" << height << std::endl;
  std::cout << "treatment?" << false << std::endl;
  std::cout << "numTreatments: " << numTreatments << std::endl;
  std::cout << "numMutations: " << numMutations << std::endl;

  std::cout << "calling scoreAttractors..." << std::endl;
  ADD states = scoreAttractors(false, numMutations);

  // temp, debugging
  std::cout << "states muts/treats/chosen vars at very beginning:" << std::endl;
  states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();
  
  // temp, debugging
  std::ofstream csv;
  csv.open("Minimax_level_" + std::to_string(height) + ".csv", std::ios_base::app);
  csv << std::endl << prettyPrint(states) << std::endl;

  height--;
  maximisingPlayer = true; // temp..............
	
  for (; height > 0; height--) { // do i have an off by one error
    std::cout << "height:" << height << std::endl;
    if (maximisingPlayer) {
      // add t
      std::cout << "treatment?" << maximisingPlayer << std::endl;
      std::cout << "numTreatments" << numTreatments << std::endl;
      std::cout << "numMutations" << numMutations << std::endl;
      std::cout << "calling backMax..." << std::endl;

      // temp, testing
      //BDD oldStates = states.BddPattern();
      
      states = backMax(states); // should this be backMin if we support async networks?

      // temp, debugging
      std::cout << "states muts/treats/chosen vars after backMax:" << std::endl;
      states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

      
      std::cout << "calling scoreAttractors..." << std::endl;
      BDD att = scoreAttractors(maximisingPlayer, numMutations).BddPattern(); // to score then unscore is not ideal

      // temp, debugging
      std::ofstream csv2;
      csv2.open("Minimax_level_" + std::to_string(height) + "_a.csv", std::ios_base::app);
      csv2 << std::endl << prettyPrint(att.Add()) << std::endl;


      states = states.MaxAbstract(representTreatmentVariables().Add()) * att.Add(); // removing the treatment = 0 forcing variables

      // temp, debugging
      std::cout << "states muts/treats/chosen vars after MaxAbstract out treatments and intersecting att:" << std::endl;
      states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

      
      // temp, testing
      // BDD testStates = att.ExistAbstract(representTreatmentVariables()) * representTreatmentNone();
      // testStates = attractors.forwardReachableStates(mutantTransitionRelation, testStates);
      // // this seems to always be false on benchmark
      // // and actually, most the time on the simple-benchark
      // std::cout << ">>> are forward test states contained in original states? " << (testStates * !oldStates).IsZero() << std::endl;
      // std::cout << "(testStates * !oldStates):" << std::endl;
      // (testStates * !oldStates).PrintMinterm();
      // std::cout << "oldStates:" << std::endl;
      // oldStates.PrintMinterm();
      // std::cout << "testStates:" << std::endl;
      // testStates.PrintMinterm();
      // std::cout << "(oldStates * !testStates):" << std::endl;
      // (oldStates * !testStates).PrintMinterm();

	
      // they should be.. because we have computed attractors from all possible states

		
      // then remove m... representNonPrimedMutVars() can be removed...........			
      numMutations--;
      std::cout << "treatment?" << maximisingPlayer << std::endl;
      std::cout << "numTreatments" << numTreatments << std::endl;
      std::cout << "numMutations" << numMutations << std::endl;
      std::cout << "calling backMax..." << std::endl;

      states = backMax(states); // should this be backMin if we support async networks?

      // temp, debugging
      std::cout << "states muts/treats/chosen vars after backMax:" << std::endl;
      states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

      
      // temp, testing
      // BDD testBackStates = attractors.backwardReachableStates(mutantTransitionRelation, states.BddPattern());
      // std::cout << "does backMax = backward?" << (testBackStates == states.BddPattern()) << std::endl;


      // std::cout << "states before unmutate:" << std::endl;
      // states.PrintMinterm();
      
      states = unmutate(numMutations, states);


      // temp, debugging
      std::cout << "states muts/treats/chosen vars after unmutate:" << std::endl;
      states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

      
      // std::cout << "states after unmutate:" << std::endl;
      // states.PrintMinterm();
      
      std::cout << "calling scoreAttractors..." << std::endl;
      att = scoreAttractors(maximisingPlayer, numMutations).BddPattern(); // to score then unscore is not ideal
      
      // temp, debugging
      std::ofstream csv3;
      csv3.open("Minimax_level_" + std::to_string(height) + "_b.csv", std::ios_base::app);
      csv3 << std::endl << prettyPrint(att.Add()) << std::endl;
      
      states *= att.Add();

        // temp, debugging
      std::cout << "states muts/treats/chosen vars after intersection with att:" << std::endl;
      states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();
    }

    else {
      // remove t
      numTreatments--;
      std::cout << "treatment?" << maximisingPlayer << std::endl;
      std::cout << "numTreatments" << numTreatments << std::endl;
      std::cout << "numMutations" << numMutations << std::endl;
      std::cout << "calling backMax..." << std::endl;

      // temp, testing
      // BDD oldStates = states.BddPattern();

      states = backMax(states); // should this be backMin if we support async networks?

      // temp, debugging
      std::cout << "states muts/treats/chosen vars after backMax:" << std::endl;
      states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

      
      states = untreat(numTreatments, states);

      // temp, debugging
      std::cout << "states muts/treats/chosen vars after untreat:" << std::endl;
      states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();


      std::cout << "calling scoreAttractors..." << std::endl;
      BDD att = scoreAttractors(maximisingPlayer, numMutations).BddPattern(); // to score then unscore is not ideal

      // temp, debugging
      std::ofstream csv2;
      csv2.open("Minimax_level_" + std::to_string(height) + ".csv", std::ios_base::app);
      csv2 << std::endl << prettyPrint(att.Add()) << std::endl;

      
      // temp, testing. treat the attractor and run forwards
      // BDD testStates = att.ExistAbstract(representTreatmentVariables()) * representSomeTreatment();
      // testStates = attractors.forwardReachableStates(mutantTransitionRelation, testStates);
      // // this seems to always be false on benchmark
      // // and actually, most the time on the simple-benchark
      // std::cout << ">>> are forward test states contained in original states? " << (testStates * !oldStates).IsZero() << std::endl;
      // std::cout << "(testStates * !oldStates):" << std::endl;
      // (testStates * !oldStates).PrintMinterm();
      // std::cout << "oldStates:" << std::endl;
      // oldStates.PrintMinterm();
      // std::cout << "testStates:" << std::endl;
      // testStates.PrintMinterm();
      // std::cout << "(oldStates * !testStates):" << std::endl;
      // (oldStates * !testStates).PrintMinterm();
      
      // they should be.. because we have computed attractors from all possible states

      states *= att.Add(); // if they are disappearing somewhere here could it be that some combos lead to a zero bdd attractor..

      // temp, debugging
      std::cout << "states muts/treats/chosen vars after intersection with att:" << std::endl;
      states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

    }

    maximisingPlayer = !maximisingPlayer;
  }

  return states;
}

BDD Game::representTreatment(int treatment) const {
  treatment++; // 0 represents no treatment, so n is represented by n+1
  BDD bdd = attractors.manager.bddOne();
  int i = treatmentVarIndices().front();

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

  for (std::vector<int>::size_type i = 0; i < oeVars.size(); i++) {
    bdd += representTreatment(i);
  }

  return bdd;
}

BDD Game::representMutation(int var, int mutation) const {
  mutation++;  // 0 represents no mutation, so n is represented by n+1
  BDD bdd = attractors.manager.bddOne();

  int b = bits(koVars.size() + 1); // + 1 so we can represent no mutation too // you actually can use fewer bits that this, as you can represent one fewer choice each time
  int i = mutationVarsIndices().front() + var * b;
	
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
  int b = bits(oeVars.size() + 1); // don't actually need +1 because don't need to represent zero, but easier this way
  int i = chosenTreatmentsIndices().front() + level * b;	

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
  int b = bits(koVars.size() + 1); // again, don't need + 1 but easier this way for symmetry with other classes of variables
  int i = chosenMutationsIndices().front() + level * b;
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
