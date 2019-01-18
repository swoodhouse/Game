// Copyright (c) Microsoft Research 2017
// License: MIT. See LICENSE

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

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

    Game g(std::move(minValuesV), std::move(rangesV), std::move(qn), std::move(mutationVarsV), std::move(treatmentVarsV), apopVar, depth, maximisingPlayerGoesLast);
	ADD out = g.minimax();

	std::cout << "done." << out.IsOne() << out.IsZero();

    return 0;
}