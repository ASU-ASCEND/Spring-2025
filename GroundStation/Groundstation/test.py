import requests 
from time import sleep
from random import random
import threading

post_endpoint = "http://127.0.0.1:5000/dataserver"

def test_endpoint():
    for i in range(100): 
        data = {
        "timestamp": i,
        "sensor_data":{
            "f1": {"f1_1": str(i), "f1_2": str(i)},
            "f2": {"f2_1": str(i)},
            "f3": {"f3_1": str(i)}
        }
        }

        post_data = {
            "timestamp": data["timestamp"]
            }
        for sensor_name in list(data["sensor_data"].keys())[1:]:
            for value_name in data["sensor_data"][sensor_name].keys():
                post_data[f"{sensor_name}_{value_name}"] = data["sensor_data"][sensor_name][value_name]

        print(requests.post(url = post_endpoint, params=post_data))

        sleep(random())

test_thread = threading.Thread(target=test_endpoint)

test_thread.start()

test_thread.join()