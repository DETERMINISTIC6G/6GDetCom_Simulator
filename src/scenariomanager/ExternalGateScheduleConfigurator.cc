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

#include "ExternalGateScheduleConfigurator.h"

namespace d6g {

Define_Module(ExternalGateScheduleConfigurator);

void ExternalGateScheduleConfigurator::initialize(int stage)
        {
            if (stage == INITSTAGE_LOCAL) {
                gateCycleDuration = par("gateCycleDuration");
                configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
            }
            else if (stage == INITSTAGE_GATE_SCHEDULE_CONFIGURATION) {
                computeConfiguration();
               // configureGateScheduling();
               // configureApplicationOffsets();
            }
        }

/*void ExternalGateScheduleConfigurator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}*/

void ExternalGateScheduleConfigurator::handleParameterChange(const char *name)
{
    if (!strcmp(name, "configuration")) {
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
        clearConfiguration();
        computeConfiguration();
        //configureGateScheduling();
        //configureApplicationOffsets();
    }
}


void ExternalGateScheduleConfigurator::executeTSNsched(std::string inputFileName) const
{
    int result = std::system("python3 scripts/dummy_scheduler.py");
}

ExternalGateScheduleConfigurator::Output *ExternalGateScheduleConfigurator::computeGateScheduling(const Input& input) const
{
    std::string baseName = "test";
    std::string inputFileName = baseName + "-TSNsched-input.json";
    // TODO: std::string outputFileName = baseName + "-TSNsched-output.json";
    std::string outputFileName = "output.json";
    //std::remove(outputFileName.c_str());
    writeInputToFile(input, inputFileName);


    executeTSNsched(inputFileName);


    return new Output();//readOutputFromFile(input, outputFileName);
}

} //namespace
