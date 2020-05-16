#pragma once

// temp: removed all const from functions

struct Game {
  /*const*/ int height;
  /*const*/ bool maximisingPlayerLast;
  /*const*/ int numMutations;
  /*const*/ int numTreatments;
  /*const*/ std::vector<int> koVars;
  /*const*/ std::vector<int> oeVars;
  int numUnprimedBDDVars;
  Attractors attractors;
 
  /*const*/ BDD mutantTransitionRelationAtt;
  /*const*/ BDD mutantTransitionRelationBack;
  /*const*/ ADD scoreRelation;
  /*const*/ BDD allFixpoints;
  
  static int calcNumMutations(int height, bool maximisingPlayerGoesLast);
  static int calcNumTreatments(int height, bool maximisingPlayerGoesLast);

  std::vector<int> attractorsIndicies();
  std::vector<int> treatmentVarIndices();
  std::vector<int> mutationVarsIndices();
  std::vector<int> chosenTreatmentsIndices();
  std::vector<int> chosenMutationsIndices();
  std::vector<std::vector<std::vector<int>::size_type>> connectedComponents();
  ADD buildScoreRelation(int apopVar);
  ADD renameBDDVarsAddingPrimes(const ADD& add);
  ADD renameBDDVarsRemovingPrimes(const ADD& add);
  ADD immediateForwardMax(const ADD& states);
  ADD immediateBackMax(const ADD& states);
  ADD backMax(const ADD& states);
  ADD scoreLoop(const BDD& loop, const ADD& scoreRelation);
  ADD scoreAttractors(bool maximisingPlayer, int numMutations);
  BDD representTreatmentVariables();
  std::string prettyPrint(const ADD & states);
  BDD buildMutantSyncQNTransitionRelation(bool back);
  BDD representTreatment(int val);
  BDD representTreatmentNone();
  BDD representSomeTreatment();
  BDD representSomeMutation(int var);
  BDD representMutation(int var, int val);
  BDD representMutationNone(int var);
  BDD representChosenTreatment(int level, int treatment);
  BDD representChosenMutation(int level, int mutation);
  BDD nMutations(int n);
  ADD untreat(int level, const ADD& states);
  ADD unmutate(int level, const ADD& states);

  BDD treatmentAbstractRelation(int level);
  BDD mutationAbstractRelation(int level);

  BDD representChosenVariables();
  void testBackReachesAll(int numMutations, bool treated, const BDD& back);
  
  //std::vector<int> computeInitialLevels();
  void setBDDLevels();
  void setBDDLevels2();
  
  Game(const std::vector<int>& minVals, const std::vector<int>& rangesV, const QNTable& qn, const std::vector<int>& koVarsV, const std::vector<int>& oeVarsV, int apopVar, int depth,
       bool maximisingPlayerGoesLast)
  {
    std::cout << "in Game ctor" << std::endl;
    height = depth;
    maximisingPlayerLast = maximisingPlayerGoesLast;
    numMutations = calcNumMutations(depth, maximisingPlayerGoesLast);
    numTreatments = calcNumTreatments(depth, maximisingPlayerGoesLast);
    koVars = koVarsV;
    oeVars = oeVarsV;
    
    auto lambda = [](int a, int b) { return a + bits(b); };
    this->numUnprimedBDDVars = std::accumulate(rangesV.begin(), rangesV.begin() + rangesV.size(), 0, lambda); // same as rangesV.end()?
    int temp = chosenMutationsIndices().back() + 1;

    attractors = Attractors(minVals, rangesV, qn, temp);

    //setBDDLevels();
    setBDDLevels2();
    
    // try turning off here then back on..
    //attractors.manager.AutodynEnable(CUDD_REORDER_GROUP_SIFT_CONV); // play with different choices again

    mutantTransitionRelationAtt = buildMutantSyncQNTransitionRelation(false);
    mutantTransitionRelationBack = buildMutantSyncQNTransitionRelation(true);
    scoreRelation = buildScoreRelation(apopVar);

    std::cout << "Finding fixpoints..." << std::endl;
    allFixpoints = attractors.fixpoints(mutantTransitionRelationAtt);// this can just be computed once, not every call
  };

  
  // this was buggy, so be careful if you go back to move ctors
  /* Game(std::vector<int>&& minVals, std::vector<int>&& rangesV, QNTable&& qn, std::vector<int>&& koVarsV, std::vector<int>&& oeVarsV, int apopVar, int depth, */
	/* 	bool maximisingPlayerGoesLast) : */
	/* 	height(depth), */
	/* 	maximisingPlayerLast(maximisingPlayerGoesLast), */
	/* 	numMutations(calcNumMutations(depth, maximisingPlayerGoesLast)), // test these two lines.. */
	/* 	numTreatments(calcNumTreatments(depth, maximisingPlayerGoesLast)), */
	/* 	koVars(std::move(koVarsV)), oeVars(std::move(oeVarsV)), */
	/*         attractors(std::move(minVals), std::move(rangesV), std::move(qn), chosenMutationsIndices().back() + 1), // this could be the problem. apparently not */
	/* 	mutantTransitionRelation(buildMutantSyncQNTransitionRelation()), */
	/* 	scoreRelation(buildScoreRelation(apopVar)) */
	/* { */
	/* 	std::cout << "in Game ctor" << std::endl; */
	/* }; // done */

  Game(const Game&) = delete;
  Game& operator=(const Game&) = delete;

  ADD minimax();
};
