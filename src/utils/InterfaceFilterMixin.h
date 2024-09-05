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

#ifndef UTILS_INTERFACEFILTER_H_
#define UTILS_INTERFACEFILTER_H_

#include <omnetpp/ccomponent.h>
#include "inet/common/InitStages.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

#include <set>
namespace d6g {
using namespace omnetpp;
using namespace inet;

template <typename T> class InterfaceFilterMixin : public T
{
  protected:
    std::set<int> indInterfaces;
    std::set<int> reqInterfaces;

  protected:
    void initialize(int stage) override;
    void addInterfacesToSet(std::set<int> &set, const char *interfaceType);
    bool matchesInterfaceConfiguration(Packet *packet) const;
    bool idMatchesSet(int id, const std::set<int> &set) const;
};

template <typename T> void InterfaceFilterMixin<T>::initialize(int stage)
{
    T::initialize(stage);
    if (stage == INITSTAGE_LAST) {
        indInterfaces.clear();
        if (T::hasPar("indInterfaceTypes")) {
            auto indInterfaceTypes = check_and_cast<cValueArray *>(T::par("indInterfaceTypes").objectValue());
            for (int i = 0; i < indInterfaceTypes->size(); i++) {
                addInterfacesToSet(indInterfaces, indInterfaceTypes->get(i).stringValue());
            }
        }
        reqInterfaces.clear();
        if (T::hasPar("reqInterfaceTypes")) {
            auto reqInterfaceTypes = check_and_cast<cValueArray *>(T::par("reqInterfaceTypes").objectValue());
            for (int i = 0; i < reqInterfaceTypes->size(); i++) {
                addInterfacesToSet(reqInterfaces, reqInterfaceTypes->get(i).stringValue());
            }
        }
    }
}

template <typename T> void InterfaceFilterMixin<T>::addInterfacesToSet(std::set<int> &set, const char *interfaceType)
{
    // Check if context has submodule with name interfaceType
    auto node = getContainingNode(this);
    if (!node->hasSubmoduleVector(interfaceType)) {
        throw cRuntimeError("No submodule with name '%s' found in '%s'", interfaceType, node->getFullPath().c_str());
    }

    // Get submodule vector with name interfaceType
    for (int i = 0; i < node->getSubmoduleVectorSize(interfaceType); i++) {
        auto *interface = dynamic_cast<NetworkInterface *>(node->getSubmodule(interfaceType, i));
        if (interface == nullptr) {
            throw cRuntimeError("Submodule with name '%s' is not a NetworkInterface", interfaceType);
        }
        set.insert(interface->getInterfaceId());
    }
}

template <typename T> bool InterfaceFilterMixin<T>::matchesInterfaceConfiguration(Packet *packet) const
{
    auto indTag = packet->findTag<InterfaceInd>();
    bool indInterfaceMatch = !indTag || idMatchesSet(indTag->getInterfaceId(), indInterfaces);

    auto reqTag = packet->findTag<InterfaceReq>();
    bool reqInterfaceMatch = !reqTag || idMatchesSet(reqTag->getInterfaceId(), reqInterfaces);

    return indInterfaceMatch && reqInterfaceMatch;
}

template <typename T> bool InterfaceFilterMixin<T>::idMatchesSet(int id, const std::set<int> &set) const
{
    return set.empty() || set.find(id) != set.end();
}

}/* namespace d6g */

#endif /* UTILS_INTERFACEFILTER_H_ */
