import json
import sys

if __name__ == "__main__":
    arguments = sys.argv[1:]
    # Reading JSON files
     #'../data/omnet/network.json'
   # with open('/home/elmos/Forschungsprojekt/dynamic-scenario/simulations/dynamicscenario/data/network.json', 'r') as network_file:
    with open(arguments[0], 'r') as network_file:
        network = json.load(network_file)
    #'../data/omnet/streams.json'
    #with open('/home/elmos/Forschungsprojekt/dynamic-scenario/simulations/dynamicscenario/data/streams.json', 'r') as streams_file:
    with open(arguments[1], 'r') as streams_file:
        streams = json.load(streams_file)
    #'../data/omnet/histograms.json'
    #with open('/home/elmos/Forschungsprojekt/dynamic-scenario/simulations/dynamicscenario/data/histograms.json', 'r') as histograms_file:
    with open(arguments[2], 'r') as histograms_file:
        histograms = json.load(histograms_file)
    
    # creating a new structure based on the scheduler input
    network_data = {
        "nodes": [],
        "links": []
    }
    streams_data = []
    
    histogram_data = {}
    
    x_pos, y_pos = 100, 100  # dummy
    
    # creating a mapping from device name to ID
    id_mapping_dict = {item["node"]: item["id"] for item in network["id_mapping"]}
    
    # add end devices
    for device in network['end_devices'] :
        node = {
            "id": id_mapping_dict.get(device["name"]),
            "name": device['name'],
            "processing_delay": device['processing_delay'] * 1000, # ns
            "type": device['device_type'],
            "position": {"x": x_pos, "y": y_pos}
        }
        network_data['nodes'].append(node)
        y_pos += 100  # dummy
    
    # add switches
    for switch in network['switches']:
        node = {
            "id": id_mapping_dict.get(switch["name"]),
            "name": switch['name'],
            "processing_delay": switch['processing_delay'] * 1000, #ns
            "type": switch['device_type'],
            "position": {"x": x_pos, "y": y_pos}
        }
        network_data['nodes'].append(node)
        y_pos += 100
    
    # creating links between the devices
    for switch in network['switches']:
        for port in switch['ports']:
            link = {
                "type": port.get("link_type"),
                "data_rate": port.get("data_rate") * 10**6 if port.get("data_rate") < 1e+308 else 1000000000, #1e+308, # b/s,  +inf if DetCom link
                "propagation_delay": int(port.get("propagation_delay") * 1000), #ns
                #"multiple_subcarriers": port.get("multiple_subcarriers"),
                "source": id_mapping_dict.get(switch['name']),
                "target": id_mapping_dict.get(port['connects_to'])
            }
            if "multiple_subcarriers" in port:
                link["multiple_subcarriers"] = port["multiple_subcarriers"]
            network_data['links'].append(link)
    
    # write network
    #with open('../data/network.json', 'w') as outfile:
    with open('/home/elmos/Forschungsprojekt/libtsndgm2.0/data/network.json', 'w') as outfile:
        json.dump(network_data, outfile, indent=4)
    
    # add flows
    for stream in streams['flows'] :
        flow = {
            "name": stream['name'],
            "period": stream['packet_periodicity'] * 1000, #ns
            "phase": stream['phase'],
            "pcp": stream['pcp'],
            "objective_type": stream['objective_type'],
            "e2e_latency": stream['hard_constraint_time_latency'] * 1000, #ns
            "jitter": stream['hard_constraint_time_jitter'] * 1000, #ns
            "frame_loss": stream['packet_loss'],
            "frame_size": int(stream['packet_size'] / 8), #B
            "weight": stream['weight'],
            "route": [
                [id_mapping_dict.get(item["currentNodeName"]),
                 id_mapping_dict.get(item["nextNodeName"])] 
                for item in stream["route"]
                ],
            #"pdb_map": None,
            #"pdb_map": [{
                #"link": [
                        #id_mapping_dict.get(item['link']["currentNodeName"]),
                        #id_mapping_dict.get(item['link']["nextNodeName"])
                    #],
                #"reliability": item['reliability'],
                #"policy": item['policy'],
                #"histogram": item['histogram']}
                #for item in stream["pdb_map"]]
        }
        if not stream["pdb_map"]:
            flow["pdb_map"] = None
        else :
            flow["pdb_map"] = [{
                "link": [
                        id_mapping_dict.get(item['link']["currentNodeName"]),
                        id_mapping_dict.get(item['link']["nextNodeName"])
                    ],
                "reliability": item['reliability'],
                "policy": item['policy'],
                "histogram": item['histogram']}
                for item in stream["pdb_map"]]
        
        
        
        
        streams_data.append(flow)
    
    # write flows
    #with open('../data/streams.json', 'w') as outfile:
    with open('/home/elmos/Forschungsprojekt/libtsndgm2.0/data/streams.json', 'w') as outfile:
        json.dump(streams_data, outfile, indent=4)
        
    
        
    # write each histogram to separate file
    for hist in histograms['distributions'] :
        #with open(f"../data/histograms/{hist['name']}.json", 'w') as outfile:
        with open(f"/home/elmos/Forschungsprojekt/libtsndgm2.0/data/histograms/{hist['name']}.json", 'w') as outfile:
            json.dump(hist, outfile, indent=4)
        
    
    print("Conversion completed.")
