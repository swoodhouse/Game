#pragma once

struct Game {
	Attractors attractors;
	const int numMutations;
	const int numTreatments;
	const int height;
	const bool maximisingPlayerLast;
	const std::vector<int> koVars;
	const std::vector<int> oeVars;

	const ADD scoreRelation;
	const BDD mutantTransitionRelation;
	//const BDD unmutateRelation; // needed? // removing this is breaking.. think the num bdd vars will change
	
	static int calcNumMutations(int height, bool maximisingPlayerGoesLast);
	static int calcNumTreatments(int height, bool maximisingPlayerGoesLast);

	std::vector<int> attractorsIndicies() const;
	std::vector<int> treatmentVarIndices() const;
	std::vector<int> mutationVarsIndices() const;
	std::vector<int> chosenTreatmentsIndices() const;
	std::vector<int> chosenMutationsIndices() const;

	ADD scoreFixpoints(const BDD & fix) const;
	BDD fixpoints(const BDD & mutsAndTreats) const;

	ADD buildScoreRelation(int apopVar) const; // done
	ADD renameBDDVarsAddingPrimes(const ADD& add) const; // done
	ADD immediateBackMax(const ADD& states) const; // done
	ADD immediateBackMin(const ADD& states) const; // done
	ADD backMin(const ADD& states) const; // done
	ADD backMax(const ADD& states) const; // done
	ADD scoreLoop(const BDD& loop, const ADD& scoreRelation) const;
	ADD scoreAttractors(bool maximisingPlayer, int numMutations) const; // done.. except taking max not mean
	BDD representTreatmentVariables() const;
	std::string prettyPrint(const ADD & states) const;
	BDD buildMutantSyncQNTransitionRelation() const; // done.. except no.. i need zero to mean no mutation i think!!!
	BDD representTreatment(int val) const; // done.. including zero mut and bits
	BDD representTreatmentNone() const; //	done
	BDD representSomeTreatment() const;
	BDD representMutation(int var, int val) const; // done.. including zero mut and bits
	BDD representMutationNone(int var) const; // done
	BDD representChosenTreatment(int level, int treatment) const; // done
	BDD representChosenMutation(int level, int mutation) const; // done
	BDD nMutations(int n) const; // done
	ADD untreat(int level, const ADD& states) const; // done
	void removeInvalidTreatmentBitCombinations(BDD& S) const; // done
	void removeInvalidMutationBitCombinations(BDD& S) const; // done
	BDD representNonPrimedMutVars() const; // done
	ADD unmutate(int level, const ADD& states) const; // done

	//Game(const std::vector<int>& minVals, const std::vector<int>& rangesV, const QNTable& qn, const std::vector<int>& koVarsV, const std::vector<int>& oeVarsV, int apopVar, int depth,
	//	bool maximisingPlayerGoesLast) :
	//	koVars(koVarsV), oeVars(oeVarsV), attractors(minVals, rangesV, qn),
	//	mutantTransitionRelation(buildMutantSyncQNTransitionRelation()),
	//	scoreRelation(buildScoreRelation(apopVar)),
	//	height(depth),
	//	maximisingPlayerLast(maximisingPlayerGoesLast),
	//	numMutations(calcNumMutations(depth, maximisingPlayerGoesLast)), // test these two lines..
	//	numTreatments(calcNumTreatments(depth, maximisingPlayerGoesLast))
	//{
	//}; // done


	// turn optimisations back on


	Game(std::vector<int>&& minVals, std::vector<int>&& rangesV, QNTable&& qn, std::vector<int>&& koVarsV, std::vector<int>&& oeVarsV, int apopVar, int depth,
		bool maximisingPlayerGoesLast) :
		height(depth),
		maximisingPlayerLast(maximisingPlayerGoesLast),
		numMutations(calcNumMutations(depth, maximisingPlayerGoesLast)), // test these two lines..
		numTreatments(calcNumTreatments(depth, maximisingPlayerGoesLast)),
		koVars(std::move(koVarsV)), oeVars(std::move(oeVarsV)),
		attractors(std::move(minVals), std::move(rangesV), std::move(qn), chosenMutationsIndices().back() + 1),
		mutantTransitionRelation(buildMutantSyncQNTransitionRelation()),
		scoreRelation(buildScoreRelation(apopVar))
	{
		std::cout << "in Game ctor" << std::endl;
	}; // done

	ADD minimax() const; // done
};
