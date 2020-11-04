/**
 * Copyright (c) 2020 by Marek Wydmuch
 * All rights reserved.
 */

#pragma once

#include <unordered_set>

#include "base.h"
#include "model.h"
#include "types.h"


// Probabilistic Labels Graph
class PLG : public Model {
public:
    PLG();
    ~PLG() override;

    void train(SRMatrix<Label>& labels, SRMatrix<Feature>& features, Args& args, std::string output) override;
    void predict(std::vector<Prediction>& prediction, Feature* features, Args& args) override;
    double predictForLabel(Label label, Feature* features, Args& args) override;

    void load(Args& args, std::string infile) override;
    inline int nodeForLabel(int label, int hash) {
        return hashes[hash].hash(label) % layerSize;
    }

protected:
    std::vector<Base*> bases;

    int layerSize;
    std::vector<UniversalHash> hashes; // of size R
    std::vector<std::vector<int>> baseToLabels;
};