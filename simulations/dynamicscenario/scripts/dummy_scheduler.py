import time
import sys
import subprocess


def schedule():
    start_time = time.perf_counter()
    print("This is a dummy script for testing purposes: ", time.ctime())
    result = subprocess.run(["python3", "scripts/modify_input.py"])
    time.sleep(3)
    print(f"finished after 3 seconds with {result.returncode} returncode: ", time.ctime())
    end_time = time.perf_counter()
    elapsed_time_ns = (end_time - start_time) * 1e9

if __name__ == "__main__":
    schedule()
