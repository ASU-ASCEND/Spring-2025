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

  def __init__(self, end_event: threading.Event):
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

    set_frame(GSEFrame.GSEFrame(self.window))

    gse_button = tk.Button(self.window, text="GSE", **top_button_config, command=lambda: set_frame(GSEFrame.GSEFrame(self.window)))
    gse_button.grid(row=0, column=self.COLUMNS // 3 * 0, **top_cell_config)

    radio_button = tk.Button(self.window, text="Radio", **top_button_config, command=lambda: set_frame(RadioFrame.RadioFrame(self.window)))
    radio_button.grid(row=0, column=self.COLUMNS // 3 * 1, **top_cell_config)

    data_button = tk.Button(self.window, text="Data Interface", **top_button_config, command=lambda: set_frame(DataFrame.DataFrame(self.window)))
    data_button.grid(row=0, column=self.COLUMNS // 3 * 2, **top_cell_config)


    def update():
      self.current_frame.update()

      self.window.after(500, update)

    self.window.after(0, update)
    self.window.mainloop()




if __name__ == '__main__':
  end_event = threading.Event()
  gui = GUI(end_event)

  gui.run()
