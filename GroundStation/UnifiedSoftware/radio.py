import tkinter as tk 
from tkinter import ttk 

import hardline

class Radio(tk.Frame): 

  def __init__(self, parent, controller):
    tk.Frame.__init__(self, parent)

    label = ttk.Label(self, text = "Radio", font = ("Verdana", 35))

    label.grid(row = 0, column = 4, padx = 10, pady = 10) 

    button1 = ttk.Button(self, text ="Hardline",
                            command = lambda : controller.show_frame(hardline.Hardline))

    button1.grid(row = 1, column = 1, padx = 10, pady = 10)