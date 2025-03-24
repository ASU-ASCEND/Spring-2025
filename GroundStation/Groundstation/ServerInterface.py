import threading
from queue import Queue, Empty
from time import sleep 
import requests
import multiprocessing

class ServerInterface(threading.Thread):

  def __init__(self, end_event: threading.Event, server_process: multiprocessing.Process, data_in: Queue):
    super().__init__()
    self.end_event = end_event
    self.data_in = data_in
    self.server_process = server_process
    self.post_endpoint = "http://127.0.0.1:5000/dataserver"
    


  def run(self):
    while self.end_event.is_set() == False:
      try: 
        data = self.data_in.get_nowait()
      except Empty:
        sleep(0.1)
        continue 
      # print("data:", data["sensor_data"].keys())
      post_data = {
        "timestamp": data["timestamp"]
      }
      for sensor_name in data["sensor_data"].keys():
        for value_name in list(data["sensor_data"][sensor_name].keys())[1:]:
          # print("adding", f"{sensor_name}_{value_name}")
          post_data[f"{sensor_name}_{value_name}"] = data["sensor_data"][sensor_name][value_name]

      requests.post(url = self.post_endpoint, params=post_data)
    self.server_process.terminate()

if __name__ == "__main__":
  import ServerProcess
  from time import sleep
  from random import random 

  data_stream = Queue()
  end_event = threading.Event()

  server_process = ServerProcess.ServerProcess()
  server_interface = ServerInterface(end_event, server_process, data_stream)

  server_process.start()
  server_interface.start()

  for i in range(20):
    data_stream.put({
        "timestamp": i,
        "sensor_data":{
          "f1": {"f1_1": str(i), "f1_2": str(i)},
          "f2": {"f2_1": str(i)},
          "f3": {"f3_1": str(i)}
        }
      })
    sleep(random())
    
  end_event.set()


  server_interface.join()
  server_process.join()

