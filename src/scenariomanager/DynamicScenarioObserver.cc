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

#include "DynamicScenarioObserver.h"
#include "ChangeMonitor.h"
#include "inet/common/scenario/ScenarioTimer_m.h"

#include "../distribution/histogram/Histogram.h"
#include "../distribution/histogram/HistogramContainer.h"

#include "inet/common/INETUtils.h"
#include "inet/common/XMLUtils.h"

namespace d6g {

const simsignal_t DynamicScenarioObserver::scenarioEventSignal =
    cComponent::registerSignal("scenario-event");
const simsignal_t DynamicScenarioObserver::parameterChangeSignal =
    cComponent::registerSignal("parameter-change-event");
const simsignal_t DynamicScenarioObserver::distributionChangeSignal =
    cComponent::registerSignal("distribution-event");

DynamicScenarioObserver::DynamicScenarioObserver(ChangeMonitor *monitor)
    : monitor(monitor) {

      };

void DynamicScenarioObserver::receiveSignal(cComponent *source,
                                            simsignal_t signalID, cObject *obj,
                                            cObject *details) {
  cMessage *msg = check_and_cast<cMessage *>(obj);

  if (signalID == scenarioEventSignal) {
    EV << "Received message (scenariomanager): " << msg->getName()
       << " from source: " << source->getFullName() << endl;
  }
  if (signalID == parameterChangeSignal) {
    EV << "Received message (app): " << msg->getName()
       << " from source: " << source->getFullPath() << endl;
    DynamicPacketSource *sourceModule =
        dynamic_cast<DynamicPacketSource *>(source);
    if (sourceModule) {
      cValueMap *element = sourceModule->getConfiguration();
      monitor->updateStreamConfigurations(element);
      delete element;
    }
    monitor->notify(source->getFullPath());
  }
  if (signalID == distributionChangeSignal) {
    std::cout << "Received message (distribution): " << msg->getName()
              << " from source: " << source->getFullPath() << endl;
    TsnTranslator *sourceModule = dynamic_cast<TsnTranslator *>(source);
    if (sourceModule) {
      std::string paramName =
          check_and_cast<cMsgPar *>(details)->stringValue(); // details->str();
      // paramName.erase(0, 1);
      // paramName.erase(paramName.size() - 1);
      auto expr = sourceModule->getDistributionExpression(paramName.c_str());
      auto element = createHistogram(*expr);
      delete expr;

      paramName.erase(0, 5);
      auto bridgePort = std::string(source->getParentModule()->getName()) +
                        "." + source->getFullName() + "_" + paramName;
      monitor->updateDistributions(bridgePort, element);
    }
    monitor->notify(source->getFullPath());
  }
}

cValueArray *
DynamicScenarioObserver::createHistogram(cDynamicExpression &dynExpr,
                                         cObject *details) {
  cValueArray *jsonBins = new cValueArray();
  Histogram *h = nullptr;

  if (dynExpr.isAConstant()) {
    double constDelay = dynExpr.evaluate(this).doubleValueInUnit("ms");

    if (constDelay == 0) {
      return jsonBins;
    }
    h = new Histogram();
    cXMLElement *histogramEntity =
        h->createHistogramEntity({constDelay, constDelay}, {1, 0}, 1);

    h->parseHistogramConfig(histogramEntity);

    h->convertHistogramToJSONBins(jsonBins);
    jsonBins->setName("constant");
    delete histogramEntity;
    delete h;
  } else {
    auto delay = dynExpr.str();
    cStringTokenizer tokenizer(delay.c_str(), " \t\n\r\f(),*\"");
    const char *token = tokenizer.nextToken();

    if (details == nullptr && !strcmp(token, "rngProvider")) {

      auto container = tokenizer.nextToken();
      auto key = tokenizer.nextToken();
      // cModule *module = monitor->getModuleByPath(container);
      HistogramContainer *sourceModule = dynamic_cast<HistogramContainer *>(
          monitor->getModuleByPath(container));
      h = sourceModule->getHistogram(key);
      h->convertHistogramToJSONBins(jsonBins);
      jsonBins->setName("rngProvider");
    } else {
      auto numberOfSamples =
          monitor->par("numberOfSamples").getValue().intValue();
      double num_bins = round(log2(numberOfSamples) + 1); // Sturges-formula
      std::vector<double> samples(numberOfSamples);

      for (int i = 0; i < numberOfSamples; ++i) {
        samples[i] = dynExpr.evaluate(monitor).doubleValueInUnit(
            "ms"); // expression->evaluate(monitor).doubleValueInUnit( "ms"); //
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
        int bin_index =
            std::min(static_cast<int>((value - min_val) / bin_width),
                     static_cast<int>(num_bins - 1));
        frequencies[bin_index]++;
      }
      h = new Histogram();
      cXMLElement *histogramEntity =
          h->createHistogramEntity(bin_edges, frequencies, num_bins);
      h->parseHistogramConfig(histogramEntity);
      h->convertHistogramToJSONBins(jsonBins);
      jsonBins->setName("expression");
      delete histogramEntity;
      delete h;
      if (details != nullptr)
        jsonBins->setName("convolution");
    }
  }

  return jsonBins;
}

} /* namespace d6g */
