import json
import os
import time
import argparse

if __name__ == "__main__":
    # Add argument parser wirth named arguments
    parser = argparse.ArgumentParser(description="Dummy Scheduler")
    parser.add_argument("--network_file", "-n", type=str, help="Path to the network file")
    parser.add_argument("--streams_file", "-s", type=str, help="Path to the streams file")
    parser.add_argument("--distribution_file", "-d", type=str, help="Path to the distribution file")
    parser.add_argument("--output_file", "-o", type=str, help="Path to the output file")

    args = parser.parse_args()

    network_file = args.network_file
    streams_file = args.streams_file
    distribution_file = args.distribution_file
    print("=========================")
    print("Scheduler called with arguments:", network_file, streams_file, distribution_file)

    # Load simulation time from streams file
    with open(streams_file, 'r') as f:
        streams = json.load(f)

    simtime_unit = streams["META"]['simulation_time_unit']
    if simtime_unit != "ns":
        raise ValueError("Simulation time unit must be in nanoseconds (ns).")
    simtime_ns = streams["META"]['simulation_time']
    simtime_s = simtime_ns / 1e9

    print("Scheduler was called at simtime:", simtime_s, "s")

    print("Simulating work for 2 seconds...")
    time.sleep(2)

    folder_of_current_script = os.path.dirname(os.path.abspath(__file__))
    dummy_config_filename = f"dummy_config-{simtime_s:.2f}s.json"

    dummy_config_path = os.path.join(folder_of_current_script, dummy_config_filename)

    print("Copying dummy config ", dummy_config_path, " to output file ", args.output_file)
    if not os.path.exists(dummy_config_path):
        raise FileNotFoundError(f"Dummy config file {dummy_config_filename} does not exist.")

    with open(dummy_config_path, 'r') as dummy_config_file:
        dummy_config = json.load(dummy_config_file)
    # Writing the dummy config to the output file
    with open(args.output_file, 'w') as output_file:
        json.dump(dummy_config, output_file, indent=4)

    print("Scheduler finished work.")
    print("=========================")