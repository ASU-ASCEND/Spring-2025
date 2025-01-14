import tkinter as tk
from tkinter import ttk

import hardline
import radio

class Main(tk.Tk):

  def __init__(self, *args, **kwargs):
    tk.Tk.__init__(self, *args, **kwargs)

    container = tk.Frame(self)
    container.pack(side = "top", fill = "both", expand = True)

    container.grid_rowconfigure(0, weight = 1)
    container.grid_columnconfigure(0, weight = 1) 

    self.frames = {}

    for F in (hardline.Hardline, radio.Radio):

      frame = F(container, self) 
      self.frames[F] = frame 

      frame.grid(row = 0, column = 0, sticky = "nsew")
    
    self.show_frame(hardline.Hardline)
  
  def show_frame(self, cont):
    frame = self.frames[cont]
    frame.tkraise()


app = Main()
app.mainloop()