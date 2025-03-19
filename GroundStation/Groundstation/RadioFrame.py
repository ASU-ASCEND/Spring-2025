import tkinter as tk 
from queue import Queue 

class RadioFrame(tk.Frame):
  def __init__(self, container, decoder_packets: Queue, header_info: tuple[dict, list]):
    super().__init__(container)
    self.TABLE_WIDTH = 8
    self.HIST_SIZE = 1
    self.data_cells = []
    self.decoder_packets = decoder_packets
    self.header_info = header_info

    # included widgets 
    self.label = tk.Label(self, text="RadioFrame")
    self.label.grid(row=0, column=0)

    self.createTable()

    self.COLUMNS, after_table = self.grid_size()

    self.label = tk.Label(self, text="Packets received: _ \tPackets dropped: _ \tSuccess Rate: _")
    self.label.grid(row=0, column=1, columnspan=self.COLUMNS-1, sticky="w")

    self.grid_columnconfigure(index=list(range(self.COLUMNS)), weight=1)

  def createTable(self):
    table_width = self.winfo_width()
    row_offset = 1 
    # set headers from config file
    header_arr = self.header_info[1]
    for i in range(len(header_arr)):
      header_label = tk.Label(self, text=header_arr[i], font = ("Helvetica", "10", "bold"), background="skyblue")
      header_label.grid(row=row_offset + (i // self.TABLE_WIDTH) * (self.HIST_SIZE+1), column=i % self.TABLE_WIDTH, sticky="nsew")

    # set up for data
    for i in range(self.HIST_SIZE):
      row = []
      for j in range(len(header_arr)):
        cell_data = tk.StringVar()
        cell_data.set("-")
        row.append(cell_data)
        Data = tk.Label(self, font = ("Helvetica", "10"))
        Data.grid(row=i+1+row_offset + (j // self.TABLE_WIDTH) * (self.HIST_SIZE + 1), column=j % self.TABLE_WIDTH, pady=2, sticky="nsew")
        Data.config(textvariable=cell_data, background="pink")
      self.data_cells.append(row)

  def updateTable(self):
    if self.decoder_packets.empty() == False:
      packet = self.decoder_packets.get()

      if packet == "ERROR":
        return # count these for radio 
      
      # read millis
      self.data_cells[0][1].set(packet["timestamp"])
      self.data_cells[0][0].configure(background="lightblue")

      # read the rest of the sensors 
      col_index = 1
      for sensor in self.header_info[0].keys():
        if sensor == "Millis": continue  
        if sensor in packet["sensor_data"]:
          for i in packet["sensor_data"][sensor].keys():
            self.data_cells[col_index][0].configure(background="lightblue")
            self.data_cells[col_index][1].set(packet["sensor_data"][sensor][i])
            col_index += 1
        else: 
          for i in range(self.header_info[0][sensor]):
            self.data_cells[col_index][0].configure(background="pink")
            col_index += 1 

  def update(self):
    self.updateTable()