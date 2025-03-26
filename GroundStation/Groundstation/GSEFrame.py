import tkinter as tk 
import tkinter.ttk as ttk 
from tkinter import scrolledtext
from math import ceil
from datetime import datetime
from queue import Queue

class GSEFrame(tk.Frame):

  def __init__(self, container, sorter_core0: Queue, sorter_core1: Queue, decoder_packets: Queue, header_info: tuple[dict, list]):
    super().__init__(container)
    self.TABLE_WIDTH = 8
    self.HIST_SIZE = 1
    self.data_cells = []
    self.sorter_core0 = sorter_core0
    self.sorter_core1 = sorter_core1
    self.decoder_packets = decoder_packets
    self.header_info = header_info

    # included widgets 
    self.label = tk.Label(self, text="GSEFrame")
    self.label.grid(row=0, column=0)

    self.createTable()

    self.COLUMNS, after_table = self.grid_size()

    self.grid_columnconfigure(index=list(range(self.COLUMNS)), weight=1)

    scroller_config = {
      "master": self,
      "wrap": tk.WORD,
      "height": 5,
      "width": 20,
      "font": ("Times New Roman", 12),
      # "relief": "flat"
    }

    subheader_config = {
      "master": self,
      "font": ("Times New Roman", 15),
    }

    # print(after_table, self.COLUMNS // 2)
    # core 0 
    core0_label = tk.Label(text="Core 0", **subheader_config)
    core0_label.grid(row=after_table, column=0, columnspan=self.COLUMNS // 2, sticky="nsew")

    self.core0_scrolled = scrolledtext.ScrolledText(**scroller_config)
    self.core0_scrolled.grid(row=after_table+1, column=0, columnspan=self.COLUMNS // 2, sticky="nsew")
    self.core0_scrolled.configure(state="disabled")

    # core 1 
    core1_label = tk.Label(text="Core 1", **subheader_config)
    core1_label.grid(row=after_table, column=self.COLUMNS // 2, columnspan=self.COLUMNS // 2, sticky="nsew")

    self.core1_scrolled = scrolledtext.ScrolledText(**scroller_config)
    self.core1_scrolled.grid(row=after_table+1, column=self.COLUMNS // 2, columnspan=self.COLUMNS // 2, sticky="nsew")
    self.core1_scrolled.configure(state="disabled")

    self.grid_rowconfigure(index=after_table+1, weight=1)


  def createTable(self): 
    row_offset = 1 
    # set headers from config file
    header_arr = self.header_info[1]
    for i in range(len(header_arr)):
      header_label = tk.Label(self, text=header_arr[i], font = ("Helvetica", "10", "bold"), background="skyblue")
      header_label.grid(row=row_offset + (i // self.TABLE_WIDTH) * (self.HIST_SIZE+1), column=i % self.TABLE_WIDTH, sticky="nsew")

    # set up for data
    self.data_cells = []
    for j in range(len(header_arr)):
      cell_data = tk.StringVar()
      cell_data.set("-")
      Data = tk.Label(self, font = ("Helvetica", "10"))
      Data.grid(row=row_offset+1 + (j // self.TABLE_WIDTH) * (self.HIST_SIZE + 1), column=j % self.TABLE_WIDTH, pady=2, sticky="nsew")
      Data.config(textvariable=cell_data, background="pink")

      self.data_cells.append((Data, cell_data))

  def updateTable(self):
    if self.decoder_packets.empty() == False:
      packet = self.decoder_packets.get()

      if packet == "ERROR":
        pass # count these for radio 
      else:
        # read millis
        self.data_cells[0][1].set(packet["timestamp"])
        self.data_cells[0][0].configure(background="lightblue")

        # read the rest of the sensors 
        col_index = 1
        for sensor in self.header_info[0].keys():
          if sensor == "Millis": continue  
          if sensor in packet["sensor_data"]:
            for i in list(packet["sensor_data"][sensor].keys())[1:]:
              self.data_cells[col_index][0].configure(background="lightblue")
              self.data_cells[col_index][1].set(round(packet["sensor_data"][sensor][i], 6))
              col_index += 1
          else: 
            for i in range(self.header_info[0][sensor]):
              self.data_cells[col_index][0].configure(background="pink")
              col_index += 1 

  def update_frame(self):
    self.updateTable()

    # update scrolledtexts
    while self.sorter_core0.empty() == False or self.sorter_core1.empty() == False:
      prefix =  datetime.now().strftime('%H:%M:%S.%f')[:-3] + " -> "
      if self.sorter_core0.empty() == False:
        self.core0_scrolled.configure(state="normal")
        self.core0_scrolled.insert(tk.INSERT, prefix + str(self.sorter_core0.get()) + "\n")
        self.core0_scrolled.yview(tk.END)
        self.core0_scrolled.configure(state="disabled")
      if self.sorter_core1.empty() == False:
        self.core1_scrolled.configure(state="normal")
        self.core1_scrolled.insert(tk.INSERT, prefix + str(self.sorter_core1.get()) + "\n")
        self.core1_scrolled.yview(tk.END)
        self.core1_scrolled.configure(state="disabled")  

