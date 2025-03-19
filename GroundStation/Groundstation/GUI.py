import threading 
from queue import Queue 
import tkinter as tk 
from tkinter import scrolledtext
import time 
from datetime import datetime 

import GSEFrame
import RadioFrame
import DataFrame

class GUI():

  def __init__(self, end_event: threading.Event, sorter_core0: Queue, sorter_core1: Queue, sorter_misc: Queue, decoder_packets: Queue):
    super().__init__()
    self.end_event = end_event 
    self.COLUMNS = 6

    # create and configure Tkinter window 
    self.window = tk.Tk()
    self.window.title("ASCEND")
    self.window.minsize(1200, 600)
    self.window.tk_setPalette(background="white")
    self.window.grid_columnconfigure(index=list(range(self.COLUMNS)), weight=1)
    self.window.grid_rowconfigure(index=1, weight=1)

    self.current_frame = tk.Frame(self.window)

    self.sorter_core0 = sorter_core0
    self.sorter_core1 = sorter_core1
    self.sorter_misc = sorter_misc 
    self.decoder_packets = decoder_packets

    header_arr = ["Millis"]
    header_key = {
      "Millis": 1, 
    }
    with open("config.csv") as f:
      for line in f: 
        fields = [str(i).strip() for i in line.split(",")]
        if fields[0] == "BitIndex": continue 

        # add to header_key 
        header_key[fields[1]] = len(fields) // 2 - 1 

        # populate header array 
        for i in range(2, len(fields), 2): 
          header_arr.append(fields[1] + " " + fields[i])
    self.header_info = (header_key, header_arr)

  def buildGSEFrame(self):
    return GSEFrame.GSEFrame(self.window, self.sorter_core0, self.sorter_core1, self.decoder_packets, self.header_info)
  
  def buildRadioFrame(self):
    return RadioFrame.RadioFrame(self.window, self.decoder_packets, self.header_info)
  
  def buildDataFrame(self):
    return DataFrame.DataFrame(self.window)

  def run(self):
    print("GUI start")

    top_button_config = {
      "width": 20, 
      "background": "skyblue"
    }
    top_cell_config = {
      "padx": 20,
      "pady": (5,10),
      # "sticky": "nsew",
      "columnspan": self.COLUMNS // 3, 
    }

    def set_frame(new_frame: tk.Frame):
      self.current_frame.destroy()
      self.current_frame = new_frame
      self.current_frame.grid(row=1, column=0, columnspan=self.COLUMNS, sticky="nsew")

    set_frame(self.buildGSEFrame())

    gse_button = tk.Button(self.window, text="GSE", **top_button_config, command=lambda: set_frame(self.buildGSEFrame()))
    gse_button.grid(row=0, column=self.COLUMNS // 3 * 0, **top_cell_config)

    radio_button = tk.Button(self.window, text="Radio", **top_button_config, command=lambda: set_frame(self.buildRadioFrame()))
    radio_button.grid(row=0, column=self.COLUMNS // 3 * 1, **top_cell_config)

    data_button = tk.Button(self.window, text="Data Interface", **top_button_config, command=lambda: set_frame(self.buildDataFrame()))
    data_button.grid(row=0, column=self.COLUMNS // 3 * 2, **top_cell_config)


    def update():
      # throw away for now, eventually an intermediary will do this 
      if self.sorter_misc.empty() == False: self.sorter_misc.get()
      self.current_frame.update()

      self.window.after(500, update)

    self.window.after(0, update)

    def flag_close(): 
      self.end_event.set()
      self.window.destroy()

    self.window.protocol("WM_DELETE_WINDOW", flag_close)
    self.window.mainloop()




if __name__ == '__main__':
  end_event = threading.Event()
  sorter_core0 = Queue()
  sorter_core1 = Queue()
  sorter_misc = Queue()
  decoder_packets = Queue()  

  decoder_packets.put({
    "timestamp": 1000, 
    "sensor_data": {
      "PicoTemp": {
        "Temperature (C)": 69,
      }
    }
  })
  
  gui = GUI(end_event, sorter_core0, sorter_core1, sorter_misc, decoder_packets, )

  gui.run()

  
