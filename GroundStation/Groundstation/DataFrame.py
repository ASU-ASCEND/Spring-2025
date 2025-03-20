import tkinter as tk 
from tkinter import filedialog as fd 
import PacketDecoder
import threading
from construct import (
    Checksum, ConstError, ChecksumError, ConstructError,
    Const, Array, Struct,
    Int32ul, Int16ul, Int8sl,
    Byte, Bytes, this, Pointer
)
from os import path 

class DataFrame(tk.Frame):

  def __init__(self, container, decoder_args: list, header_info: tuple[dict, list]):
    super().__init__(container)
    self.file_list = []
    self.COLUMNS = 4

    button_config = {
      "width": 20, 
      "background": "skyblue"
    }
    button_cell_config = {
      "padx": 20,
      "pady": (5,10),
      # "sticky": "nsew",
      "columnspan": 1, 
    }

    # included widgets 
    self.label = tk.Label(self, text="DataFrame")
    self.label.grid(row=0, column=0)

    self.status_label = tk.Label(self, text="Status: -")
    self.status_label.grid(row=0, column=1, columnspan=self.COLUMNS-1, sticky=tk.W)

    self.get_file_list_button = tk.Button(self, text="Get File List", **button_config, command=self.get_file_list)
    self.get_file_list_button.grid(row=1, column=0, **button_cell_config)

    self.convert_bin_button = tk.Button(self, text="Convert .bin", **button_config, command=self.convert_button)
    self.convert_bin_button.grid(row=1, column=1, **button_cell_config)

    self.file_list_widgets = []
    self.bitmask_to_struck = decoder_args[0]
    self.bitmask_to_name = decoder_args[1]
    self.num_sensors = decoder_args[2]
    self.header_info = header_info

    self.get_file_list()    

  def transfer_file(self, file_name: str):
    # implement with serial interface
    self.status_label.config(text=f"Status: File transferred as data/{file_name}")

  def delete_file(self, file_name: str):
    # implement with serial interface
    self.file_list.remove(file_name)

    self.update_file_list()
    self.status_label.config(text=f"Status: {file_name} has been deleted")
    
  def get_file_list(self):
    # implement with serial interface  
    self.file_list = ["file1", "file2"]

    self.update_file_list()
    self.status_label.config(text=f"Status: File list retrieved")

  def update_file_list(self):
    file_name_config = {

    }
    file_button_config = {
      "width": 20, 
      "background": "skyblue"
    }
    cell_config = {
      "padx": 20,
      "pady": (5,10),
      # "sticky": "nsew",
      "columnspan": 1, 
    }

    for i in self.file_list_widgets: 
      i.destroy()

    def transfer_file_factory(file_name):
      def action():
        self.transfer_file(file_name)
      return action
    
    def delete_file_factory(file_name):
      def action():
        self.delete_file(file_name)
      return action 

    row_index = 2
    for file in self.file_list:

      file_entry = [
        tk.Label(self, text=file, **file_name_config),
        tk.Button(self, text="Transfer", **file_button_config, command=transfer_file_factory(file)),
        tk.Button(self, text="Delete", **file_button_config, command=delete_file_factory(file))
      ]

      column_index = 0
      for widget in file_entry:
        widget.grid(row=row_index, column=column_index, **cell_config)
        column_index += 1

      self.file_list_widgets.extend(file_entry)
      row_index += 1

  
  def update(self):
    pass

  def convert_button(self):
    filename = fd.askopenfilename()

    if filename[-4:].lower() != ".bin": 
      return 
    
    thread = threading.Thread(target = self.convert_bin, args = (filename, ))
    thread.start()

  # Validate checksum
  def validate(self, packet: bytearray):
      total = 0
      for i in range(len(packet)-1):
        total += int.from_bytes(bytes(packet[i]), byteorder='little', signed=False)

      checksum = int.from_bytes(bytes(packet[-1]), byteorder='little', signed=True)
      return total + checksum == 0

  def convert_bin(self, filename: str):
    with open(filename, "r") as f, open(path.join("converted_data", filename[:-4] + ".csv"), "w") as fout: 

      # add header 
      fout.write(",".join(self.header_info[1]) + "\n")

      buffer = bytearray()
      while(byte := f.read(1)):
        buffer.append(byte)

        if buffer.find(b"ASU!") > 0: # not if it's the first one 
          packet_bytes = buffer[0:buffer.find(b"ASU!", start=len(b"ASU!")+1)]
          # Parse packet bytes & catch possible errors
          try:
            if self.validate(packet_bytes) == False: raise ChecksumError("Checksum Error")
            parsed_packet = self.packet_struct.parse(packet_bytes)
          except ConstError as e: # Catch sync byte mistmatch
            print(f"[ERROR] Sync byte mismatch: {e}")
            continue
          except ChecksumError as e: # Catch checksum validation errors
            print(f"[ERROR] Checksum validation failed: {e}")
            continue
          except ConstructError as e: # Catch-all for other parse errors
            print(f"[ERROR] Packet parsing failed: {e}")
            continue

          # Extract sensor ID & sensor data
          bitmask = parsed_packet.bitmask
          sensor_data = bytes(parsed_packet.sensor_data)
          timestamp = parsed_packet.timestamp

          # print("Mask: ", bin(bitmask))

          # Parse each sensor data field
          offset = 0
          parsed = {}
          parse_error = False

          for bitmask_index in reversed(range(self.num_sensors)): # Iterate through each sensor (0 -> num_sensors)
            if bitmask & (1 << bitmask_index):
              if bitmask_index not in self.bitmask_to_struct: # Check if sensor exists
                print(f"[ERROR] No sensor found for bitmask index: {bitmask_index}")
                continue

              # Extract sensor data fields & sensor name
              sensor_fields = self.bitmask_to_struct[self.num_sensors - bitmask_index - 1]
              sensor_name = self.bitmask_to_name[self.num_sensors - bitmask_index - 1]

              try: # Parse sensor data fields
                temp_parsed = sensor_fields.parse(sensor_data[offset:])
                # print("Temp Parsed:", temp_parsed)
              except ConstructError as e: # Catch errors in parsing sensor data
                print(f"[ERROR] Parsing sensor {sensor_name} (bitmask {bitmask_index}) failed: {e}")
                parse_error = True 
                break

              # Store parsed sensor data
              parsed[sensor_name] = temp_parsed
              offset += sensor_fields.sizeof()

          # print(parsed)

          if parse_error == False:
            packet = {
                "timestamp": timestamp,
                "sensor_data": parsed
            }

            row = [] 

            row.append(packet["timestamp"])
            for sensor in self.header_info[0].keys():
              if sensor == "Millis": continue 
              if sensor in packet["sensor_data"]:
                for i in list(packet["sensor_data"][sensor].keys())[1:]:
                  row.append(packet["sensor_data"][sensor][i])
              else:
                for i in range(self.header_info[0][sensor]):
                  row.append("")
            
            fout.write(",".join(row) + "\n")
                 

          else: 
            pass


    
