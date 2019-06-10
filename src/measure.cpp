/**
 * Copyright (c) 2019 by Marek Wydmuch
 * All rights reserved.
 */

#include <string>
#include <fstream>
#include <iomanip>

#include "model.h"
#include "threads.h"

#include "measure.h"
#include "set_utility.h"

Measure::Measure(Args& args, Model* model) {
    sum = 0;
    count = 0;
}

double Measure::value() {
    return sum / count;
}

MeasureAtK::MeasureAtK(Args& args, Model* model) : Measure(args, model){
    k = args.topK;
}

Recall::Recall(Args& args, Model* model) : Measure(args, model){
    name = "Recall";
}

void Recall::accumulate(Label* labels, const std::vector<Prediction>& prediction){
    double tp = 0;
    int l;
    for (const auto& p : prediction){
        l = -1;
        while(labels[++l] > -1)
            if (p.label == labels[l]){
                ++tp;
                break;
            }
    }
    l = -1;
    while(labels[++l] > -1);
    if(l > 0) {
        sum += tp / l;
        ++count;
    }
}

RecallAtK::RecallAtK(Args& args, Model* model) : MeasureAtK(args, model){
    name = "R@" + std::to_string(k);
}

void RecallAtK::accumulate(Label* labels, const std::vector<Prediction>& prediction){
    double tp = 0;
    int l;
    for (int i = 0; i < k; ++i){
        l = -1;
        while(labels[++l] > -1)
            if (prediction[i].label == labels[l]){
                ++tp;
                break;
            }
    }
    l = -1;
    while(labels[++l] > -1);
    if(l > 0) {
        sum += tp / l;
        ++count;
    }
}

Precision::Precision(Args& args, Model* model) : Measure(args, model){
    name = "Precision";
}

void Precision::accumulate(Label* labels, const std::vector<Prediction>& prediction){
    double tp = 0;
    int l;
    for (const auto& p : prediction){
        l = -1;
        while(labels[++l] > -1)
            if (p.label == labels[l]){
                ++tp;
                break;
            }
    }
    sum += tp / prediction.size();
    ++count;
}

PrecisionAtK::PrecisionAtK(Args& args, Model* model) : MeasureAtK(args, model){
    name = "P@" + std::to_string(k);
}

void PrecisionAtK::accumulate(Label* labels, const std::vector<Prediction>& prediction){
    double tp = 0;
    int l;
    for (int i = 0; i < k; ++i){
        l = -1;
        while(labels[++l] > -1)
            if (prediction[i].label == labels[l]){
                ++tp;
                break;
            }
    }
    sum += tp / k;
    ++count;
    //sum += tp;
    //count += k;
}

Coverage::Coverage(Args& args, Model* model) : Measure(args, model){
    name = "Coverage";
}

void Coverage::accumulate(Label* labels, const std::vector<Prediction>& prediction){
    int l;
    for (const auto& p : prediction){
        l = -1;
        while(labels[++l] > -1)
            if (p.label == labels[l]){
                seen.insert(p.label);
                break;
            }
    }
}

double Coverage::value(){
    return static_cast<double>(seen.size()) / m;
}

CoverageAtK::CoverageAtK(Args& args, Model* model) : MeasureAtK(args, model){
    name = "C@" + std::to_string(k);
}

void CoverageAtK::accumulate(Label* labels, const std::vector<Prediction>& prediction){
    int l;
    for (int i = 0; i < k; ++i){
        l = -1;
        while(labels[++l] > -1)
            if (prediction[i].label == labels[l]){
                seen.insert(prediction[i].label);
                break;
            }
    }
}

double CoverageAtK::value(){
    return static_cast<double>(seen.size()) / m;
}

Accuracy::Accuracy(Args& args, Model* model) : Measure(args, model){
    name = "Acc";
}

void Accuracy::accumulate(Label* labels, const std::vector<Prediction>& prediction){
    if(labels[0] > -1 && prediction.size() && labels[0] == prediction[0].label)
        ++sum;
    ++count;
}


std::vector<std::shared_ptr<Measure>> measuresFactory(Args& args, Model* model){
    std::vector<std::shared_ptr<Measure>> measures;

    std::vector<std::string> measuresNames = split(args.measures, ',');
    for(const auto& m : measuresNames){
        if(m == "p")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<Precision>(args, model)));
        else if(m == "p@k")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<PrecisionAtK>(args, model)));
        else if(m == "r")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<Recall>(args, model)));
        else if(m == "r@k")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<RecallAtK>(args, model)));
        else if(m == "c")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<Coverage>(args, model)));
        else if(m == "c@k")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<CoverageAtK>(args, model)));
        else if(m == "acc")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<Accuracy>(args, model)));
        else if(m == "uP")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<PrecisionUtility>(args, model)));
        else if(m == "uF1")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<F1Utility>(args, model)));
        else if(m == "uAlfa")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<UtilityAlfa>(args, model)));
        else if(m == "uAlfaBeta")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<UtilityAlfaBeta>(args, model)));
        else if(m == "uDeltaGamma")
            measures.push_back(std::static_pointer_cast<Measure>(std::make_shared<UtilityDeltaGamma>(args, model)));
    }

    return measures;
}