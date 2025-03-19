import tkinter as tk 
import tkinter.ttk as ttk 
from tkinter import scrolledtext
from math import ceil

class GSEFrame(tk.Frame):

  def __init__(self, container):
    super().__init__(container)
    self.TABLE_WIDTH = 8
    self.HIST_SIZE = 1
    self.data_cells = []


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
    table_width = self.winfo_width()
    row_offset = 1 
    # set headers from config file
    header_arr = [ "Header" + str(i) for i in range(10)]
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
        Data.config(textvariable=cell_data)
      self.data_cells.append(row)

  def update(self):
    pass

