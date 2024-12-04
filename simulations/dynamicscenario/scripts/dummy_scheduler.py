import time
import sys


def schedule():
    start_time = time.perf_counter()
    print("This is a dummy script for testing purposes: ", time.ctime())
    time.sleep(5)
    print("finished after 5 seconds: ", time.ctime())
    end_time = time.perf_counter()
    elapsed_time_ns = (end_time - start_time) * 1e9

if __name__ == "__main__":
    schedule()
