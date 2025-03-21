import threading
from queue import Queue, Empty 
from time import sleep
from os import path 
from datetime import datetime

class SerialSaver(threading.Thread):

  def __init__(self, end_event: threading.Event, serial_stream: Queue):
    self.end_event = end_event
    self.serial_stream = serial_stream
    self.session_filename = path.join("session_data", f"ASCEND_DATA_{datetime.now().strftime('%H_%M_%S')}.bin")

  def run(self):
    with open(self.session_filename, "wb") as fout: 
      while self.end_event.is_set() == False:
        try: 
          byte_input: bytes = self.serial_stream.get_nowait()
          print(byte_input.decode())
          fout.write(byte_input)
        except Empty:
          sleep(0.1)
          continue