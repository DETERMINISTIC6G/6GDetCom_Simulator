// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.dynamicscenario;

import inet.common.scenario.ScenarioManager;

//
// This module extends ScenarioManager module by adding a signal for an observer when new scenario events occur.
//
simple ObservedScenarioManager extends ScenarioManager
{
    parameters:
        @class(ObservedScenarioManager);        
     	@signal[scenario-event](type=cMessage);
     	
    
}
