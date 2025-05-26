// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "PdcDelayer.h"

#include "../../timestamping/DetComTimeTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

using namespace omnetpp;
using namespace inet;

namespace d6g {
Define_Module(PdcDelayer);

PdcDelayer::~PdcDelayer()
{
    for (auto &mapping : mappings) {
        delete mapping.second;
    }
}

void PdcDelayer::initialize(int stage)
{
    InterfaceFilterMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        setDefaultPdc(&par("defaultPdc"));
        setDefaultJitter(&par("defaultJitter"));
        configureMappings();
    }
}

void PdcDelayer::configureMappings()
{
    auto mappingParameter = check_and_cast<cValueArray *>(par("mapping").objectValue());

    mappings.clear();
    for (int i = 0; i < mappingParameter->size(); i++) {
        auto element = check_and_cast<cValueMap *>(mappingParameter->get(i).objectValue());
        auto mapping = new Mapping();
        if (element->containsKey("pdc")) {
            mapping->pdc = new cDynamicExpression();
            mapping->pdc->parseNedExpr(element->get("pdc").stringValue());
        }
        if (element->containsKey("jitter")) {
            mapping->jitter = new cDynamicExpression();
            mapping->jitter->parseNedExpr(element->get("jitter").stringValue());
        }
        auto streamName = element->get("stream").stringValue();
        mappings[streamName] = mapping;
    }
}

clocktime_t PdcDelayer::computeDelay(Packet *packet) const
{
    if (!matchesInterfaceConfiguration(packet)) {
        return 0;
    }

    // auto detComIngressTag = packet->findTag<DetComIngressTimeTag>();

    auto residenceTimeTag = packet->findTag<DetComResidenceTimeTag>();
    auto streamIdTag = packet->findTag<StreamInd>();

    clocktime_t pdc = defaultPdc->doubleValue();
    clocktime_t jitter = defaultJitter->doubleValue();

    if (residenceTimeTag == nullptr) {
        return 0;
    }
    clocktime_t residenceTime = residenceTimeTag->getResidenceTime();
    if (streamIdTag != nullptr) {
        auto streamID = streamIdTag->getStreamName();
        auto mapping = mappings.find(streamID);
        // This is ugly, I could call getModuleByPath(".") instead, but the implementation of that also uses const_cast
        //  so it does not matter...
        auto nonConstThis = const_cast<PdcDelayer *>(this);
        if (mapping != mappings.end()) {
            if (mapping->second->pdc != nullptr) {
                pdc = mapping->second->pdc->doubleValue(nonConstThis, "s");
            }
            if (mapping->second->jitter != nullptr) {
                jitter = mapping->second->jitter->doubleValue(nonConstThis, "s");
            }
        }
    }
    if (residenceTime > pdc + jitter) {
        return 0;
    }
    auto delay = pdc + jitter - residenceTime;
    return delay;
}

void PdcDelayer::setDefaultPdc(cPar *delay) { defaultPdc = delay; }

void PdcDelayer::setDefaultJitter(cPar *jitter) { defaultJitter = jitter; }

void PdcDelayer::handleParameterChange(const char *parname)
{
    if (!strcmp(parname, "defaultPdc")) {
        setDefaultPdc(&par("defaultPdc"));
    }
    if (!strcmp(parname, "defaultJitter")) {
        setDefaultJitter(&par("defaultJitter"));
    }
    if (!strcmp(parname, "mapping")) {
        configureMappings();
    }
}
} /* namespace d6g */
