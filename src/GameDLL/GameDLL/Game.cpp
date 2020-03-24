// heavy refactoring needed. an on Attractors.cpp too i think

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

inline BDD logicalImplication(const BDD& a, const BDD& b) {
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

int Game::calcNumTreatments(int height, bool maximisingPlayerGoesLast) {
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
      return ((representSomeMutation(0) * representMutationNone(1)) +
              (representSomeMutation(1) * representMutationNone(0)));
    }
    else if (numMutations == 1) {
      return representSomeMutation(0);
    }
    else {
      std::cout << "nmutations > 2 not implemented" << std::endl;
      throw std::runtime_error("nmutations > 2 not implemented");
    }
  }
  else if (n == 2 && numMutations == 2) {
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

  i = 0;
  for (int var = 0; var < attractors.ranges.size(); var++) {
    int b = bits(attractors.ranges[var]);

    if (std::find(oeVars.begin(), oeVars.end(), var) != oeVars.end()) {
      for (int n = 0; n < b; n++) {
        permute[i] = i + attractors.numUnprimedBDDVars;
  	   i++;
      }
    }
    else {
      i += b;
    }
  }

  ADD permuted = states.Permute(&permute[0]);

  // new, bug fix: then multiply by a transition ADD that selectively existentially quantifies
  // then mult by (If representChosenTreatment(j, level) then qnvar(j) = 0 or 1 or 2 or 3 or ... else qnvar(j) = qnvar'(j) for all j in oeVars)
  // is the result the same if i switch to relation2()?
  BDD abstractRelation = treatmentAbstractRelation(level);
  ADD abstracted = permuted * abstractRelation.Add();
  
  // then abstract out the primed vars
  abstracted = abstracted.MaxAbstract(attractors.primeVariables.Add());
  
  return abstracted;
}

BDD Game::treatmentAbstractRelation(int level) const {  
  BDD abstractRelation = attractors.manager.bddOne();
  for (int treatIndex = 0; treatIndex < oeVars.size(); treatIndex++) { // size_type not int 
    int treatVar = oeVars[treatIndex];
    BDD disjunction = attractors.manager.bddOne(); // could this be written as an exist, or does it even need to be anythign at all.. probably can use an implication rather than an Ite
    BDD equality = attractors.manager.bddOne();

    for (int val = 0; val <= attractors.ranges[treatVar]; val++) {
      equality *= logicalEquivalence(attractors.representUnprimedVarQN(treatVar, val),
				     attractors.representPrimedVarQN(treatVar, val));
    }
    
    abstractRelation *= representChosenTreatment(level, treatIndex).Ite(disjunction, equality); // not level + 1 or level - 1?
  }
  
  return abstractRelation;
}

// assuming the above is fixed, is this broken too. this says if we have just unmutated mut i at recent level, don't do anything, otherwise set unprimed = primed
BDD Game::mutationAbstractRelation(int level) const {
  BDD abstractRelation = attractors.manager.bddOne();
  for (int mutIndex = 0; mutIndex < koVars.size(); mutIndex++) { // size_type not int
    int mutVar = koVars[mutIndex];
    BDD disjunction = attractors.manager.bddOne(); // could this be written as an exist, or does it even need to be anythign at all.. probably can use an implication rather than an Ite
    BDD equality = attractors.manager.bddOne();
    for (int val = 0; val <= attractors.ranges[mutVar]; val++) {
      equality *= logicalEquivalence(attractors.representUnprimedVarQN(mutVar, val),
				     attractors.representPrimedVarQN(mutVar, val));
    }

    abstractRelation *= representChosenMutation(level, mutIndex).Ite(disjunction, equality); // not level + 1 or level - 1?
  }
  
  return abstractRelation;
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
  
  //new, bug fix: abstract out the actual QN variable that was mutated, too.
  // there was a bug here i've attempted to fix, need to do similar on untreat
  i = 0;
  for (int var = 0; var < attractors.ranges.size(); var++) {
    int b = bits(attractors.ranges[var]);

    if (std::find(koVars.begin(), koVars.end(), var) != koVars.end()) {
      for (int n = 0; n < b; n++) {
        permute[i] = i + attractors.numUnprimedBDDVars;
	i++;
      }
    }
    else {
      i += b;
    }
  }
 
  ADD permuted = states.Permute(&permute[0]);

  // new, bug fix:
  BDD abstractRelation = mutationAbstractRelation(level);
  ADD abstracted = permuted * abstractRelation.Add();

  // then abstract out the primed vars
  abstracted = abstracted.MaxAbstract(attractors.primeVariables.Add());
  
  return abstracted;
}

BDD Game::buildMutantSyncQNTransitionRelation(bool back) const {
  BDD tr = attractors.manager.bddOne();

  std::cout << "in buildMutantTR" << std::endl;

  auto components = connectedComponents();
  std::cout << "num connected components: " << components.size() << std::endl;

  // do i want to sort in ascending or descending order? ascending
  std::sort(components.begin(), components.end(),
   	    [](const std::vector<std::vector<int>::size_type>& a,
   	       const std::vector<std::vector<int>::size_type>& b){ return a.size() < b.size(); });

  //auto start = std::chrono::steady_clock::now();
 
  int vars_done = 0;
  int comp_num = 0;
  for (auto comp : components) {
    std::cout << "here3" << std::endl;
    BDD bdd = attractors.manager.bddOne();
    for (auto v : comp) {
      std::cout << "here4" << std::endl;
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

	  std::cout << "k is:" << k << std::endl;
	  std::cout << "v is:" << v << :: std::endl;
	  
	  BDD isMutated = attractors.manager.bddZero();
				
	  for (int lvl = 0; lvl < numMutations; lvl++) {
	    isMutated += representMutation(lvl, k);
	  }
  				
	  int max = attractors.ranges[v];

	  // adding this goes back to a case where we only allow m1 and m2 equal
	  // backMax isn't worrking.. it's still allowing things to spontenously unmutate.. is it because we don't set the ranges for them?
	  // intersect with S?
	  if (back) {
	    //std::cout << "isMutated:" << std::endl;
	    //isMutated.PrintMinterm();
	    //bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 1) * attractors.representUnprimedVarQN(v, 1),
	    //			 targetFunction);
	    // temp
	    //bdd *= isMutated.Ite(attractors.representUnprimedVarQN(v, 1), targetFunction);
	    bdd *= isMutated.Ite(attractors.representUnprimedVarQN(v, max), targetFunction);
	  }
	  else {
	    bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, max), targetFunction);
	    // temp! hard coding to one to match mathew's benchmark model
	    //bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 1), targetFunction);

	    // new. still hard coding to 1. ideally want to build two trs - a forward one and a back one, have a param and an if statement here
	    //bdd *= isMutated.Ite(attractors.representPrimedVarQN(v, 1) * attractors.representUnprimedVarQN(v, 1),
	    //                     targetFunction);
	  }
	  
	}
	else {
          auto oeIt = std::find(oeVars.begin(), oeVars.end(), v);
          if (oeIt != oeVars.end()) { // rename to treat vars - koing not oe-ing
            int o = std::distance(oeVars.begin(), oeIt);        
	    BDD isTreated = representTreatment(o);

	    std::cout << "o is:" << o << std::endl;
	    std::cout << "v is:" << v << :: std::endl;
	    
	    if (back) {
	      //bdd *= isTreated.Ite(attractors.representPrimedVarQN(v, 0) * attractors.representUnprimedVarQN(v, 0),
	      //		   targetFunction);
	      // temp
	      bdd *= isTreated.Ite(attractors.representUnprimedVarQN(v, 0), targetFunction);
	    }
	    else {
	      bdd *= isTreated.Ite(attractors.representPrimedVarQN(v, 0), targetFunction);
	    }
	    // new. ideally want to build two trs - a forward one and a back one, have a param and an if statement here [back is below, fwd is above] // this leads to a memory exception
	    //bdd *= isTreated.Ite(attractors.representPrimedVarQN(v, 0) * attractors.representUnprimedVarQN(v, 0),
	    //			 targetFunction);
          }
	  else {
	    bdd *= targetFunction;
          }
        }
      }
    }
    //auto diff = std::chrono::steady_clock::now() - start;
    //std::cout << "total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;

    std::cout << "adding connected component " << comp_num << std::endl;
    comp_num++;
    tr *= bdd;
    std::cout << "here1" << std::endl;
    
    //diff = std::chrono::steady_clock::now() - start;
    //std::cout << "total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
  }

  std::cout << "here at end" << std::endl;
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
  ADD add = mutantTransitionRelationAtt.Add() * states;
  add = add.MaxAbstract(attractors.nonPrimeVariables.Add());
  return renameBDDVarsRemovingPrimes(add);
}

ADD Game::immediateBackMax(const ADD& states) const {
  ADD add = renameBDDVarsAddingPrimes(states);
  add *= mutantTransitionRelationBack.Add();
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

    // new: get the mut and treat vars too
    // need to add to the header too in the F# code..
    i = treatmentVarIndices().front();
    int b = bits(oeVars.size() + 1);
    auto val = fromBinary(line.substr(i, b), 0);
    output.push_back(val);

    i = mutationVarsIndices().front();
    for (int v = 0; v < numMutations; v++) { // is nMutations here correct??
      int b = bits(koVars.size() + 1);
      auto val = fromBinary(line.substr(i, b), 0);
      output.push_back(val);
      i += b;
    }

    i = chosenTreatmentsIndices().front();
    for (int v = 0; v < numTreatments; v++) {
      int b = bits(oeVars.size() + 1);
      auto val = fromBinary(line.substr(i, b), 0);
      output.push_back(val);
      i += b;
    }

    i = chosenMutationsIndices().front();
    for (int v = 0; v < numMutations; v++) {
      int b = bits(koVars.size() + 1);
      auto val = fromBinary(line.substr(i, b), 0);
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

// ADD Game::scoreAttractors(bool applyTreatments, int numMutations) const {
//   ADD states = attractors.manager.addZero();
//   BDD treatment = applyTreatments ? representSomeTreatment() : representTreatmentNone();
	
//   BDD mutsAndTreats = treatment * nMutations(numMutations);
  
//   BDD statesToRemove = !mutsAndTreats;

//   std::list<BDD> loops = attractors.attractors(mutantTransitionRelationAtt, statesToRemove);
  
//   std::cout << "loops.len:" << loops.size() << std::endl; // 64..?
  
//   for (const BDD& a : loops) {
//     ADD scored = scoreLoop(a, scoreRelation);
//     states = states.Maximum(scored);
//   }
  
//   return states;
// }


ADD Game::scoreAttractors(bool applyTreatments, int numMutations) const {
  // new, fixpoints optimisation //////
  // std::cout << "Finding fixpoints..." << std::endl;
  // BDD allFixpoints = attractors.fixpoints(mutantTransitionRelationAtt);// this can just be computed once, not every call

  /////////////////////////////////////

  ADD states = attractors.manager.addZero();
  BDD treatment = applyTreatments ? representSomeTreatment() : representTreatmentNone();

  BDD mutsAndTreats = treatment * nMutations(numMutations);

  // new, fixpoints optimisation
  BDD fixpoints = allFixpoints * mutsAndTreats; // fixpoints at this level only
  //////////////////////////////////////

  //BDD statesToRemove = !mutsAndTreats;
  // new, fixpoints optimisation
  BDD statesToRemove = !mutsAndTreats + fixpoints + attractors.backwardReachableStates(mutantTransitionRelationAtt, fixpoints);
  ////////////////////////////////


  std::list<BDD> loops = attractors.attractors(mutantTransitionRelationAtt, statesToRemove);
  // new, fixpoints optimisation
  loops.push_back(fixpoints);
  ///////////
  std::cout << "loops.len:" << loops.size() << std::endl;
 
  for (const BDD& a : loops) {
    ADD scored = scoreLoop(a, scoreRelation);
    states = states.Maximum(scored);
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


  // temp... experimenting with reordering......... 31347.1 ms / 26926.3 ms
  // without changing, ... with removal of printing minterms
  //attractors.manager.AutodynDisable();  // seems to slow down by ~x2?
  // could try other orderings here or manually call below
  //attractors.manager.AutodynEnable(CUDD_REORDER_SIFT); // 5128.17 ms with removal of printing minterms.  7263.61 ms now.
  //attractors.manager.AutodynEnable(CUDD_REORDER_WINDOW2); // 5469.04 ms with removal of printing minterms. 4753.13 ms now
  //attractors.manager.AutodynEnable(CUDD_REORDER_WINDOW4); //  4144.42 ms
  attractors.manager.AutodynEnable(CUDD_REORDER_WINDOW4_CONV); //  4098.88 ms
  //attractors.manager.AutodynEnable(CUDD_REORDER_ANNEALING); //  _SLOW_
  //attractors.manager.AutodynEnable(CUDD_REORDER_GENETIC); // _SLOW_
  //attractors.manager.AutodynEnable(CUDD_REORDER_WINDOW3_CONV);
  //CUDD_REORDER_WINDOW3_CONV
  //CUDD_REORDER_SYMM_SIFT
  // genetic
  /////////////////////////////////////////////////
  
  
  auto start = std::chrono::steady_clock::now();
  
  std::cout << "\n\n\nin minimax" << std::endl;
  int height = this->height;
  int numTreatments = this->numTreatments;
  int numMutations = this->numMutations;
  bool maximisingPlayer = this->maximisingPlayerLast;

  maximisingPlayer = true;

  std::cout << "maximisingPlayer:" << maximisingPlayer << std::endl;
  std::cout << "height:" << height << std::endl;
  std::cout << "treatment?" << maximisingPlayer << std::endl;
  std::cout << "numTreatments: " << numTreatments << std::endl;
  std::cout << "numMutations: " << numMutations << std::endl;

  std::cout << "calling scoreAttractors..." << std::endl;
  ADD states = scoreAttractors(maximisingPlayer, numMutations);
  auto diff = std::chrono::steady_clock::now() - start;
  std::cout << "scoreAttractors done. total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
  std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;
	
  std::cout << "[[at beginning; " << numMutations << " mutations, 1 treat]]" << std::endl;
  
  // temp, debugging
  // std::ofstream csv;
  // csv.open("Minimax_level_" + std::to_string(height) + "_att.csv");
  // csv << prettyPrint(states) << std::endl;

  height--;
  maximisingPlayer = true; // temp..............

  //BDD temp_oldAtts = states.BddPattern();
	
  for (; height > 0; height--) { // do i have an off by one error
    std::cout << "height:" << height << std::endl;
    if (maximisingPlayer) {
      // temp, testing
      // temp, debugging
      // std::cout << "states muts/treats/chosen vars after backMax:" << std::endl;
      // states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

      
      // new, bringing in line with Matthew's model
      if (height < this->height - 1) {
	// add t
	std::cout << "a branch" << std::endl;
	std::cout << "treatment?" << maximisingPlayer << std::endl;
	std::cout << "numTreatments" << numTreatments << std::endl;
	std::cout << "numMutations" << numMutations << std::endl;
	std::cout << "calling backMax..." << std::endl;

	// temp
	//	BDD temp_oldStates = states.BddPattern();
      
	states = backMax(states); // should this be backMin if we support async networks?
        diff = std::chrono::steady_clock::now() - start;
	std::cout << "backMax done. total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
	std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;
 
  
	//testBackReachesAll(numMutations, false, states.BddPattern()); // this one failing?
	//std::cout << "[[back params (from previous): " << numMutations << " mutations, 0 treats]]" << std::endl;

	// temp
	// std::cout << "old states in forward states? " <<
	//   (temp_oldStates * attractors.forwardReachableStates(mutantTransitionRelationAtt, states)) << std::endl;
	// temp_oldStates in forward states. attractors from states = temp_oldStates

	// std::ofstream csv3;
       	// csv3.open("Minimax_level_" + std::to_string(height) + "_a_back.csv");
       	// csv3 << prettyPrint(states) << std::endl;
	
	std::cout << "calling scoreAttractors..." << std::endl;
	BDD att = scoreAttractors(true, numMutations).BddPattern(); // to score then unscore is not ideal
	diff = std::chrono::steady_clock::now() - start;
	std::cout << "scoreAttractors done. total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
	std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;

	std::cout << "[[maximising branch (a): " << numMutations << " mutations, 1 treat]]" << std::endl;
	
	// wait only do this on the mut introducing ones
	// testReachability(att, temp_oldAtts); // temp
	//temp_oldAtts = att;
	
	
	// temp, debugging
	// std::ofstream csv2;
	// csv2.open("Minimax_level_" + std::to_string(height) + "_a_att.csv");
	// csv2 << prettyPrint(att.Add()) << std::endl;

 
	states = states.MaxAbstract(representTreatmentVariables().Add()); //is this a problem... allows value 3...?
      	// std::ofstream csv4;
      	// csv4.open("Minimax_level_" + std::to_string(height) + "_a_untreat.csv");
      	// csv4 << prettyPrint(states) << std::endl;

	std::cout << "intersecting with attractors" << std::endl;
	states *= att.Add(); // removing the treatment = 0 forcing variables
	std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;
	
	// std::ofstream csvI;
	// csvI.open("Minimax_level_" + std::to_string(height) + "_a_intersect.csv");
	// csvI << prettyPrint(states) << std::endl;

      }
     
		
      // then remove m... representNonPrimedMutVars() can be removed...........			
      numMutations--;
      std::cout << "b branch" << std::endl;
      std::cout << "treatment?" << maximisingPlayer << std::endl;
      std::cout << "numTreatments" << numTreatments << std::endl;
      std::cout << "numMutations" << numMutations << std::endl;
      std::cout << "calling backMax..." << std::endl;

      states = backMax(states); // should this be backMin if we support async networks?
      diff = std::chrono::steady_clock::now() - start;
      std::cout << "backMax done. total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
      std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;
 
      // std::ofstream csv3;
      // csv3.open("Minimax_level_" + std::to_string(height) + "_b_back.csv");
      // csv3 << prettyPrint(states) << std::endl;

      //ADD beforeUnmutate_temp = states;

      //do i need an if statement here? no.. it should be irrelavant. if you run from an attractor you should hit everything
      // this has to be conditional on whether the above if statement was triggered
      //testBackReachesAll(numMutations+1, true, states.BddPattern()); // this one failing?
      //std::cout << "[[back params (from previous): " << numMutations + 1 << " mutations, 1 treat]]" << std::endl;

      // std::ofstream csv_backtest;
      // csv_backtest.open("backtest_h" + std::to_string(height) + ".csv");
      // csv_backtest << prettyPrint(states) << std::endl;

      
      //testBackReachesAll(numMutations+1, height < this->height - 1, states.BddPattern());
      //std::cout << "[[back params (from previous): " << numMutations + 1 << " mutations, " << (height < this->height - 1) << " treats]]" << std::endl;
     
      states = unmutate(numMutations, states);

	// temp
  // std::ofstream csv4;
  // csv4.open("Minimax_level_" + std::to_string(height) + "_b_unmutate.csv");
  // csv4 << prettyPrint(states) << std::endl;

  //     // temp, debugging
  //     std::cout << "states muts/treats/chosen vars after unmutate:" << std::endl;
  //     states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

      //testMutationTransfer(numMutations, beforeUnmutate_temp, states);


      
      std::cout << "calling scoreAttractors..." << std::endl;
      BDD att = scoreAttractors(maximisingPlayer, numMutations).BddPattern(); // to score then unscore is not ideal
      diff = std::chrono::steady_clock::now() - start;
      std::cout << "scoreAttractors done. total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
      std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;
      std::cout << "[[maximising branch (b): " << numMutations << " mutations, 1 treat]]" << std::endl;      
      // only do this on unmutate
      //testReachability(att, temp_oldAtts, numMutations); // temp
      //temp_oldAtts = att;
	
      // temp, debugging
      // std::ofstream csv5;
      // csv5.open("Minimax_level_" + std::to_string(height) + "_b_att.csv");
      // csv5 << prettyPrint(att.Add()) << std::endl;
      
      std::cout << "intersecting with attractors" << std::endl;
      states *= att.Add();
      std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;
      
      // temp, debugging
      // std::ofstream csvI;
      // csvI.open("Minimax_level_" + std::to_string(height) + "_b_intersect.csv");
      // csvI << prettyPrint(states) << std::endl;

      
      // std::cout << "att muts/treats/chosen vars:" << std::endl;
      // att.ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

        // temp, debugging
      // std::cout << "states muts/treats/chosen vars after intersection with att:" << std::endl;
      // states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();
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
      diff = std::chrono::steady_clock::now() - start;
      std::cout << "backMax done. total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
      std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;
	
      //testBackReachesAll(numMutations, true, states.BddPattern()); // this one passing now too?
      //std::cout << "[[back params (from previous): " << numMutations << " mutations, 1 treat]]" << std::endl;

  // std::ofstream csv3;
  // csv3.open("Minimax_level_" + std::to_string(height) + "_back.csv");
  // csv3 << prettyPrint(states) << std::endl;
      
  //     // temp, debugging
  //     std::cout << "states muts/treats/chosen vars after backMax:" << std::endl;
  //     states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

      // std::cout << "states before untreat:" << std::endl;
      // states.PrintMinterm();

      //ADD beforeUntreat_temp = states;
      
      states = untreat(numTreatments, states);

      //testTreatmentTransfer(numTreatments, beforeUntreat_temp, states);
      
  // std::ofstream csv4;
  // csv4.open("Minimax_level_" + std::to_string(height) + "_untreat.csv");
  // csv4 << prettyPrint(states) << std::endl;
      // std::cout << "states after untreat:" << std::endl;
      // states.PrintMinterm();
      
      
      // temp, debugging
      // std::cout << "states muts/treats/chosen vars after untreat:" << std::endl;
      // states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();


      std::cout << "calling scoreAttractors..." << std::endl;
      BDD att = scoreAttractors(maximisingPlayer, numMutations).BddPattern(); // to score then unscore is not ideal
      diff = std::chrono::steady_clock::now() - start;
      std::cout << "scoreAttractors done. total time so far: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
      std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;
  
      std::cout << "[[minimising branch: " << numMutations << " mutations, 0 treat]]" << std::endl;      
      // only do this on unmutate
      //testReachability(att, temp_oldAtts); // temp
      //temp_oldAtts = att;

      
      // temp, debugging
      // std::ofstream csv2;
      // csv2.open("Minimax_level_" + std::to_string(height) + "_att.csv");
      // csv2 << prettyPrint(att.Add()) << std::endl;

      std::cout << "intersecting with attractors" << std::endl;
      states *= att.Add(); // if they are disappearing somewhere here could it be that some combos lead to a zero bdd attractor..
      std::cout << "number of BDD variables: " << states.SupportSize() << std::endl;

      // // temp, debugging
      // std::ofstream csvI;
      // csvI.open("Minimax_level_" + std::to_string(height) + "_intersect.csv");
      // csvI << prettyPrint(states) << std::endl;
      
      // // temp, debugging
      // std::cout << "states muts/treats/chosen vars after intersection with att:" << std::endl;
      // states.BddPattern().ExistAbstract(attractors.nonPrimeVariables).PrintMinterm();

    }

    maximisingPlayer = !maximisingPlayer;
  }

  std::cout << "numMutations: " << numMutations << std::endl;
  std::cout << "maximisingPlayer: " << maximisingPlayer << std::endl;
  
  //std::cout << "final testBackReaches all:" << std::endl;
  //testBackReachesAll(numMutations, false, backMax(states).BddPattern());


  std::cout << "[[back params (from previous): " << numMutations << " mutations, 0 treats]]" << std::endl;
 
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

///////////////////
// temp
BDD Game::representChosenVariables() const {
  BDD bdd = attractors.manager.bddOne();

  for (auto i : chosenTreatmentsIndices()) {
    BDD var = attractors.manager.bddVar(i);
    bdd *= var;
  }

  for (auto i : chosenMutationsIndices()) {
    BDD var = attractors.manager.bddVar(i);
    bdd *= var;
  }

  return bdd;
}

void Game::testBackReachesAll(int numMutations, bool treated, const BDD& back) const {
  BDD abstractedBack = back.ExistAbstract(representChosenVariables());

  BDD test = treated ? attractors.manager.bddOne() : representTreatmentNone();

  if (treated) {
    BDD bdd = attractors.manager.bddZero();
    for (int t = 0; t < oeVars.size(); t++) {
      int var = oeVars[t];
      bdd += representTreatment(t) * attractors.representUnprimedVarQN(var, 0);
    }
    test *= bdd;
  }
 
  for (int i = 1; i <= numMutations; i++) { // <= or <????
    BDD bdd = attractors.manager.bddZero();
    for (int m = 0; m < koVars.size(); m++) {
      int var = koVars[m];
      // temp! hard coding to one to match matthew's benchmark model
      bdd += representMutation(i - 1, m) * attractors.representUnprimedVarQN(var, 1);
    }
    test *= bdd;
  }

  for (int i = numMutations; i < this->numMutations; i++) {
    test *= representMutationNone(i);
  }

  std::cout << "does back reach everything?:" << (test == abstractedBack) << std::endl;
}
