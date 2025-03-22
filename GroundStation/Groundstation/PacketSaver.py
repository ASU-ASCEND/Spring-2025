import threading 
from queue import Queue, Empty
from datetime import datetime
from os import path, mkdir
from time import sleep

class PacketSaver(threading.Thread):

  def __init__(self, 
               end_event: threading.Event, 
               decoder_packet_saver: Queue, 
               packet_saver_server: Queue,
               header_info: tuple[dict, list]
               ):
    super().__init__()
    self.end_event = end_event
    self.decoder_packet_saver = decoder_packet_saver
    self.packet_saver_server = packet_saver_server
    self.header_info = header_info

    if path.isdir("session_data") == False:
      mkdir("session_data")
    self.session_filename = path.join("session_data", f"ASCEND_DATA_{datetime.now().strftime('%H_%M_%S')}.csv")
    # print("Saving to:", self.session_filename)

  def run(self):
    with open(self.session_filename, "w") as fout:
      fout.write(",".join(self.header_info[1]) + "\n")

      while self.end_event.is_set() == False:
        try: 
          packet = self.decoder_packet_saver.get_nowait()
        except Empty:
          sleep(0.1) 
          continue

        # forward to server_interface - here not in PacketDecoder because I think PacketDecoder has slower throughput currently 
        self.packet_saver_server.put(packet)

        # print("Got packet")
        row = [] 

        row.append(str(packet["timestamp"]))
        for sensor in self.header_info[0].keys():
          if sensor == "Millis": continue 
          if sensor in packet["sensor_data"]:
            for i in list(packet["sensor_data"][sensor].keys())[1:]:
              row.append(str(packet["sensor_data"][sensor][i]))
          else:
            for i in range(self.header_info[0][sensor]):
              row.append("")
        
        fout.write(",".join(row) + "\n")

      
