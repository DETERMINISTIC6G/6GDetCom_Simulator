//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "TsnTranslator.h"

#include "../../distribution/contract/IRandomNumberProvider.h"

#include "../../distribution/histogram/HistogramContainer.h"


#include <regex>


namespace d6g {
//using namespace inet;

Define_Module(TsnTranslator);

void TsnTranslator::initialize()
{
    //cSimpleModule::initialize(stage);
    distributionChangeEvent = new cMessage("distribution-changed");
}

/*void TsnTranslator::handleMessage(cMessage *msg)
{
    cSimpleModule::handleMessage(msg);

}*/


void TsnTranslator::handleParameterChange(const char *name) {
    if (!strcmp(name, "delayDownlink") || !strcmp(name, "delayUplink") ) {
        cMsgPar *details = new cMsgPar("details");
        details->setStringValue(name);
        emit(DynamicScenarioObserver::distributionChangeSignal,
                distributionChangeEvent, details);
        delete details;
    }
}


cValueArray* TsnTranslator::getDistribution(const char *delayStr, int numberOfSamples) {

    auto delay = par(delayStr).str();
    cDynamicExpression *dynExpr = new cDynamicExpression();
    dynExpr->parse(delay.c_str());

    cValueArray *jsonBins = new cValueArray();
    Histogram *h = nullptr;


    if (dynExpr->isAConstant()) {
        h = new Histogram();
        double constDelay = dynExpr->evaluate(this).doubleValueInUnit("ms");
        if (constDelay == 0) {
            delete h;
            delete dynExpr;
            return jsonBins;
        }
        cXMLElement *histogramEntity = h->createHistogramEntity( { constDelay, constDelay }, { 1, 0 }, 1);
        h->parseHistogramConfig(histogramEntity);
        h->convertHistogramToJSONBins(jsonBins);
        delete histogramEntity;
        delete h;
    } else {

        cStringTokenizer tokenizer(delay.c_str(), " \t\n\r\f(),*\"");
        const char *token = tokenizer.nextToken();

        if (!strcmp(token, "rngProvider")) {

            auto container = tokenizer.nextToken();
            auto key = tokenizer.nextToken();

            cModule *module = getModuleByPath(container);
            HistogramContainer *sourceModule =
                    dynamic_cast<HistogramContainer*>(module);

            h = sourceModule->getHistogram(key);
            h->convertHistogramToJSONBins(jsonBins);
        } else {
            double num_bins = round(log2(numberOfSamples) + 1); // Sturges-Formel
            std::vector<double> samples(numberOfSamples);
            for (int i = 0; i < numberOfSamples; ++i) {
                samples[i] = dynExpr->evaluate(this).doubleValueInUnit("ms");
            }
            double min_val = *std::min_element(samples.begin(), samples.end());
            double max_val = *std::max_element(samples.begin(), samples.end());

            double bin_width = (max_val - min_val) / num_bins;

            std::vector<double> bin_edges;
            for (int i = 0; i <= num_bins; ++i) {
                bin_edges.push_back(min_val + i * bin_width);
            }
            std::vector<int> frequencies;
            frequencies.resize(num_bins + 1, 0);
            for (double value : samples) {
                int bin_index = std::min(
                        static_cast<int>((value - min_val) / bin_width),
                        static_cast<int>(num_bins - 1));
                frequencies[bin_index]++;
            }

            h = new Histogram();
            cXMLElement *histogramEntity = h->createHistogramEntity(bin_edges,
                    frequencies, num_bins);
            h->parseHistogramConfig(histogramEntity);
            h->convertHistogramToJSONBins(jsonBins);

            delete histogramEntity;
            delete h;
        }
    }
    delete dynExpr;
    return jsonBins;
}



TsnTranslator::~TsnTranslator() {
    delete distributionChangeEvent;
}





} /* namespace d6g */
