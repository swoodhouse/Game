#pragma once

struct Game {
  /*const*/ int height;
  /*const*/ bool maximisingPlayerLast;
  /*const*/ int numMutations;
  /*const*/ int numTreatments;
  /*const*/ std::vector<int> koVars;
  /*const*/ std::vector<int> oeVars;
  int numUnprimedBDDVars;
  Attractors attractors;
 
  /*const*/ BDD mutantTransitionRelation;
  /*const*/ ADD scoreRelation;
	
  static int calcNumMutations(int height, bool maximisingPlayerGoesLast);
  static int calcNumTreatments(int height, bool maximisingPlayerGoesLast);

  std::vector<int> attractorsIndicies() const;
  std::vector<int> treatmentVarIndices() const;
  std::vector<int> mutationVarsIndices() const;
  std::vector<int> chosenTreatmentsIndices() const;
  std::vector<int> chosenMutationsIndices() const;

  ADD buildScoreRelation(int apopVar) const;
  ADD renameBDDVarsAddingPrimes(const ADD& add) const;
  ADD renameBDDVarsRemovingPrimes(const ADD& add) const;
  ADD immediateBackMax(const ADD& states) const;
  ADD backMax(const ADD& states) const;
  ADD scoreLoop(const BDD& loop, const ADD& scoreRelation) const;
  ADD scoreLoopNew(const BDD& loop, const ADD& scoreRelation) const;
  ADD scoreAttractors(bool maximisingPlayer, int numMutations) const;
  BDD representTreatmentVariables() const;
  std::string prettyPrint(const ADD & states) const;
  BDD buildMutantSyncQNTransitionRelation() const;
  BDD representTreatment(int val) const;
  BDD representTreatmentNone() const;
  BDD representSomeTreatment() const;
  BDD representSomeMutation(int var) const;
  BDD representMutation(int var, int val) const;
  BDD representMutationNone(int var) const;
  BDD representChosenTreatment(int level, int treatment) const;
  BDD representChosenMutation(int level, int mutation) const;
  BDD nMutations(int n) const;
  ADD untreat(int level, const ADD& states) const;
  void removeInvalidTreatmentBitCombinations(BDD& S) const;
  void removeInvalidMutationBitCombinations(BDD& S) const;
  BDD representNonPrimedMutVars() const;
  ADD unmutate(int level, const ADD& states) const;

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
    mutantTransitionRelation = buildMutantSyncQNTransitionRelation();
    scoreRelation = buildScoreRelation(apopVar);
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

  ADD minimax() const;
};
