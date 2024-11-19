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

#include "DynamicPacketSource.h"

namespace d6g {

Define_Module(DynamicPacketSource);

void DynamicPacketSource::initialize(int stage)
{
    ActivePacketSource::initialize(stage);
}

void DynamicPacketSource::handleMessage(cMessage *msg)
{
    ActivePacketSource::handleMessage(msg);
}

void DynamicPacketSource::handleParameterChange(const char *name)
{
    if (!strcmp(name, "initialProductionOffset")) {
        initialProductionOffset = par("initialProductionOffset");
        cancelEvent(productionTimer);
        initialProductionOffsetScheduled = false;
        scheduleProductionTimerAndProducePacket();
    }
    if (!strcmp(name, "productionInterval")) {
        productionIntervalParameter = &par("productionInterval");
    }
    if (!strcmp(name, "packetLength")) {
        packetLengthParameter = &par("packetLength");
    }
}

} //namespace
