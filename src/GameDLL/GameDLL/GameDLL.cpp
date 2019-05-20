// Copyright (c) Microsoft Research 2017
// License: MIT. See LICENSE

#include "stdafx.h"
#include "Attractors.h"
#include "Game.h"

extern "C" __declspec(dllexport) int minimax(int numVars, int ranges[], int minValues[], int numInputs[], int inputVars[], int numUpdates[],
	int inputValues[], int outputValues[], int numMutations, int numTreatments, int mutationVars[], int treatmentVars[], int apopVar, int height)
	{
	/*int headerLength const char *csvHeader,) {*/
    //std::string header(csvHeader, headerLength);
	std::cout << "in dll. numVars:" << numVars << ", numMutations:" << numMutations << ", numTreatments:" << numTreatments << ", apopVar:" << apopVar << ", height: " << height << std::endl;

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
	//QNTable qn = QNTable(inputVarsV, inputValuesV, outputValuesV);

	std::sort(mutationVarsV.begin(), mutationVarsV.end());
	std::sort(treatmentVarsV.begin(), treatmentVarsV.end());

	std::cout << "here" << std::endl;

    Game g(std::move(minValuesV), std::move(rangesV), std::move(qn), std::move(mutationVarsV), std::move(treatmentVarsV), apopVar, height, false);
	//Game g(minValuesV, rangesV, qn, mutationVarsV, treatmentVarsV, apopVar, height, false);
	std::cout << "height:" << height << std::endl;
	std::cout << "g.numMutations:" << g.numMutations << std::endl;
	std::cout << "g.numTreatments:" << g.numTreatments << std::endl;
	std::cout << "game.attractors.ranges.size():" << g.attractors.ranges.size() << std::endl;
	std::cout << "g.attractorsIndicies().back()" << g.attractorsIndicies().back() << std::endl;
	std::cout << "g.treatmentVarIndices().back()" << g.treatmentVarIndices().back() << std::endl;
	std::cout << "g.mutationVarsIndices().back()" << g.mutationVarsIndices().back() << std::endl;
	std::cout << "game.chosenTreatmentsIndices().back():" << g.chosenTreatmentsIndices().back() << std::endl;
	std::cout << "game.chosenMutationsIndices().back():" << g.chosenMutationsIndices().back() << std::endl;

	std::cout << "apopVar: " << apopVar << std::endl;
	

	ADD out = g.minimax();


	std::cout << "\nFinal output of minimax:" << std::endl;
	out.PrintMinterm();


	// both of these blocks cause a crash
	//std::cout << "Cudd_ReadNodeCount:" << Cudd_ReadNodeCount(g.attractors.manager.getManager()) << std::endl;
	//std::cout << "Cudd_ReadSize:" << Cudd_ReadSize(g.attractors.manager.getManager()) << std::endl;

	// the below causes the whole program to crash before even starting anything..
	//std::ofstream csv;
	//csv.open("Minimax.csv", std::ios_base::app);
	//csv << g.prettyPrint(out) << std::endl;
	//std::cout << g.prettyPrint(out) << std::endl;

    return 0;
}