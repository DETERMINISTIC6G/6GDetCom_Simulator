// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "HistogramContainer.h"
#include "inet/common/XMLUtils.h"
#include "fstream"


namespace d6g {

using namespace inet::xmlutils;

Define_Module(HistogramContainer);


void HistogramContainer::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        histograms.clear();

        auto histogramsMap = check_and_cast<cValueMap*>(
                par("histograms").objectValue());
        auto histogramsFields = histogramsMap->getFields();

        for (auto &histogramsField : histogramsFields) {
            auto streamName = histogramsField.first;
            auto xmlFile = histogramsField.second.stringValue();
            histograms[streamName] = loadHistogramFromFile(xmlFile);
            EV << histograms[streamName] << std::endl;
        }
    }
}

Histogram *HistogramContainer::loadHistogramFromFile(const char *fileName) {
    std::ifstream infile(fileName);
    if (!infile) {
        throw cRuntimeError("File '%s' not found, pwd is %s", fileName, getcwd(NULL, 0));
    }

    cXMLElement *xmlData = getEnvir()->getXMLDocument(fileName);

    if (!xmlData || strcmp(xmlData->getTagName(), "histogram") != 0) {
        throw cRuntimeError("Invalid XML data for histogram");
    }

    // Create a new Histogram instance
    auto *histogram = new Histogram();

    // Parse the histogram configuration
    histogram->parseHistogramConfig(xmlData);

    infile.close();

    return histogram;
}

cValue HistogramContainer::getRand(const std::string &key) {
    auto histogram = getHistogram(key);
    return histogram->getRand();
}

Histogram *HistogramContainer::getHistogram(const std::string &key) const {
    auto it = histograms.find(key);
    if (it == histograms.end()) {
        throw cRuntimeError("Histogram '%s' not found", key.c_str());
    }
    return it->second;
}


void HistogramContainer::processCommand(const cXMLElement &node) {
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-histogram")) {
        auto key = xmlutils::getMandatoryAttribute(node, "key");
        auto pathToHistogram = xmlutils::getMandatoryAttribute(node,
                "histogram");

        histograms[key] = loadHistogramFromFile(pathToHistogram);

    } else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

void HistogramContainer::handleParameterChange(const char *parname) {
    if (!strcmp(parname, "histograms")) {
        EV << (histograms.rbegin())->first << std::endl;
    }

}


HistogramContainer::~HistogramContainer() {
    for (auto &histogram: histograms) {
        delete histogram.second;
    }
}



} /* namespace d6g */
