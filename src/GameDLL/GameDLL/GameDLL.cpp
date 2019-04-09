// Copyright (c) Microsoft Research 2017
// License: MIT. See LICENSE

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

// TEMP ///////////////
#include <rapidcheck.h>
#include <random>
#include <chrono>
#include <vector>
#include <algorithm>
#include <functional>

void unmutate2(const Game& game) {
	rc::check("unmutate",
		[&]() {
		const auto v = *rc::gen::container<std::vector<bool>>(game.attractors.numUnprimedBDDVars, rc::gen::arbitrary<bool>()); // temp
		//const auto level = *rc::gen::inRange(1, game.numMutations);//*rc::gen::inRange(0, game.numMutations); // or 0 to mut???
		const auto level = *rc::gen::inRange(0, game.numMutations);
		const auto mutValues = *rc::gen::unique<std::vector<int>>(rc::gen::inRange(0, game.numMutations));
		//const auto mutValues = *rc::gen::container<std::vector<int>>(level, rc::gen::inRange(0, static_cast<int>(game.koVars.size())));
		//const auto mutValues = *rc::gen::container<std::vector<int>>(level, rc::gen::inRange(0, static_cast<int>(game.koVars.size())));
		//const auto mutValues = *rc::gen::container<std::vector<int>>(level + 1, rc::gen::inRange(0, static_cast<int>(game.koVars.size())));

		//const auto mutation = *rc::gen::inRange(0, static_cast<int>(game.koVars.size()));

		BDD states = game.attractors.manager.bddVar(0);
		if (v[0]) states = !states;

		for (int i = 1; i < v.size(); i++) {
			BDD s = game.attractors.manager.bddVar(i);

			if (v[i]) s = !s;

			if (*rc::gen::arbitrary<bool>()) {
				states *= s;
			}
			else {
				states += s;
			}
		}

		const int mutation = mutValues[0];

		BDD otherMutations = game.attractors.manager.bddOne();
		// this loop was commented out
		for (int i = 0; i < level; i++) {
			//int m = *rc::gen::inRange(0, static_cast<int>(game.koVars.size()));
			//int m = *rc::gen::elementOf()
			otherMutations *= game.representMutation(i, m[i]);
		}

		// right?
		//otherMutations = game.nMutations(level - 1);
		
		/*	std::cout << "game.representChosenMutation(level, mutation):" << game.representChosenMutation(level, mutation).FactoredFormString() << std::endl;
		std::cout << "game.representMutation(level, mutation):" << game.representMutation(level, mutation).FactoredFormString() << std::endl;
		std::cout << "game.representMutationNone(level):" << game.representMutationNone(level).FactoredFormString() << std::endl;*/
		// ^ representNone seems to be what we are missing, maybe indexing errors in chooseRelation

		// temp!!
		BDD unmutated = states * otherMutations * game.representChosenMutation(level, mutation) *  game.representMutationNone(level); //game.representMutationNone(level - 1);
		BDD mutated = states * otherMutations * game.representMutation(level, mutation);

		//std::cout << "states:" << states.FactoredFormString() << std::endl;
		//std::cout << "mutated:" << mutated.FactoredFormString() << std::endl;
		//std::cout << "unmutated:" << unmutated.FactoredFormString() << std::endl;
		//std::cout << "transformed:" << game.unmutate(level, mutated.Add()).BddPattern().FactoredFormString() << std::endl; // this is 1. like its removed everything

		//std::cout << "equal bdds?:" << (game.unmutate(level, mutated.Add()).BddPattern() == unmutated);
		//std::cout << "equal adds?:" << (game.unmutate(level, mutated.Add()) == unmutated.Add());

		// actually looks right now. but still failing.. some maybe the terminal node value is different?
		// ah.. could come from exist abstract
		// should i replace with max abstract then?? do i need a version for max and min?
		// you get rid of the mutation but you tag with a new choice var. so should only be only possibility?
		// chooseRelation => probably the bug

		RC_ASSERT(game.unmutate(level, mutated.Add()) == unmutated.Add());
	});
}



////////////////

extern "C" __declspec(dllexport) int minimax(int numVars, int ranges[], int minValues[], int numInputs[], int inputVars[], int numUpdates[],
    int inputValues[], int outputValues[], int numMutations, int numTreatments, int mutationVars[], int treatmentVars[], int apopVar, int depth, bool maximisingPlayerGoesLast) {
//extern "C" __declspec(dllexport) int minimax2(int numVars, int ranges[], int minValues[], int numInputs[], int inputVars[], int numUpdates[],
//	int inputValues[], int outputValues[], int numMutations, int numTreatments, int mutationVars[], int treatmentVars[], int apopVar, int depth, bool maximisingPlayerGoesLast) {
    //std::string outputPath(output, outputLength);
    //std::string header(csvHeader, headerLength);

	
    std::vector<int> rangesV(ranges, ranges + numVars);
	std::vector<int> minValuesV(minValues, minValues + numVars);
	std::vector<int> mutationVarsV(mutationVars, mutationVars + numMutations);
	std::vector<int> treatmentVarsV(treatmentVars, treatmentVars + numTreatments);
    std::vector<std::vector<int>> inputVarsV;
    std::vector<std::vector<int>> outputValuesV;
    std::vector<std::vector<std::vector<int>>> inputValuesV;
	
    int k = 0;
    for (int i = 0; i < numVars; i++) {
        std::vector<int> in;
        for (int j = 0; j < numInputs[i]; j++) {
            in.push_back(inputVars[k]);
            k++;
        }
        inputVarsV.push_back(in);
    }

    k = 0;
    for (int i = 0; i < numVars; i++) {
        std::vector<int> out;
        for (int j = 0; j < numUpdates[i]; j++) {
            out.push_back(outputValues[k]);
            k++;
        }
        outputValuesV.push_back(out);
    }

    k = 0;
    for (int i = 0; i < numVars; i++) {
        std::vector<std::vector<int>> in;
        for (int j = 0; j < numUpdates[i]; j++) {
            std::vector<int> v;
            for (int l = 0; l < numInputs[i]; l++) {
                v.push_back(inputValues[k]);
                k++;
            }
            in.push_back(v);
        }
        inputValuesV.push_back(in);
    }


	QNTable qn = QNTable(std::move(inputVarsV), std::move(inputValuesV), std::move(outputValuesV));

	std::sort(mutationVarsV.begin(), mutationVarsV.end());
	std::sort(treatmentVarsV.begin(), treatmentVarsV.end());

    Game g(std::move(minValuesV), std::move(rangesV), std::move(qn), std::move(mutationVarsV), std::move(treatmentVarsV), apopVar, depth, maximisingPlayerGoesLast);
	std::cout << "game.chosenMutationsIndices().back():" << g.chosenMutationsIndices().back();

	std::cout << "apopVar: " << apopVar << std::endl;


	// TEMP: RUN TEST
	unmutate2(g); // ..........

	/*ADD out = g.minimax();

	std::cout << "out is zero?" << out.IsZero() << std::endl;
	out.PrintMinterm();
*/
    return 0;
}