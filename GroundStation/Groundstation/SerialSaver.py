import threading
from queue import Queue, Empty 
from time import sleep
from os import path, mkdir 
from datetime import datetime

class SerialSaver(threading.Thread):

  def __init__(self, end_event: threading.Event, serial_stream: Queue):
    super().__init__()
    self.end_event = end_event
    self.serial_stream = serial_stream
    if path.isdir("session_data") == False:
      mkdir("session_data")
    self.session_filename = path.join("session_data", f"ASCEND_DATA_SERIAL_{datetime.now().strftime('%m_%d_%H_%M_%S')}.bin")

  def run(self):
    with open(self.session_filename, "wb") as fout: 
      while self.end_event.is_set() == False:
        try: 
          byte_input = self.serial_stream.get_nowait()
          for b in byte_input:
            print(chr(b), end="")
          fout.write(byte_input)
        except Empty:
          sleep(0.1)
          continue