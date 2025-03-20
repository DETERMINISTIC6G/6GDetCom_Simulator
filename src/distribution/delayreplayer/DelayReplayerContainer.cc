// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DelayReplayerContainer.h"

#include <omnetpp.h>
#include "fstream"

namespace d6g {

Define_Module(DelayReplayerContainer);

void DelayReplayerContainer::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        delayreplayers.clear();

        auto delayreplayersMap = check_and_cast<cValueMap *>(par("delayreplayers").objectValue());
        auto delayreplayersFields = delayreplayersMap->getFields();

        for(auto &delayreplayersField: delayreplayersFields){
            auto streamName = delayreplayersField.first;
            auto csvFile = delayreplayersField.second.stringValue();
            delayreplayers[streamName] = loadDelayReplayerFromFile(csvFile);
        }
    }
}

DelayReplayer* DelayReplayerContainer::loadDelayReplayerFromFile(const char* fileName){
    auto* delayreplayer = new DelayReplayer();
    delayreplayer->readCSV(fileName);

    return delayreplayer;
}

DelayReplayer *DelayReplayerContainer::getDelayreplayer(const std::string& key) const {
    auto it = delayreplayers.find(key);
    if (it == delayreplayers.end()) {
        throw cRuntimeError("DelayReplayer '%s' not found", key.c_str());
    }
    return it->second;
}

cValue DelayReplayerContainer::getRand(const std::string &key) {
    auto delayreplayer = getDelayreplayer(key);
    return delayreplayer->getRand();
}

DelayReplayerContainer::~DelayReplayerContainer() {
    for (auto &delayreplayer: delayreplayers) {
        delete delayreplayer.second;
    }
}

} /* namespace d6g */
