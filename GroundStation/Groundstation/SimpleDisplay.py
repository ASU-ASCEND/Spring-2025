import threading
from queue import Queue 
import tkinter as tk
from tkinter import scrolledtext
import time 
from datetime import datetime 

class SimpleDisplay(threading.Thread): 

  def __init__(self, end_event: threading.Event, sorter_core0: Queue, sorter_core1: Queue, sorter_misc: Queue, decoder_packets: Queue):
    super().__init__()
    self.sorter_core0 = sorter_core0
    self.sorter_core1 = sorter_core1
    self.sorter_misc = sorter_misc
    self.decoder_packets = decoder_packets 
    self.end_event = end_event

  def run(self):
    # do thread stuff
    print("Thread start")
    # create and configure Tkinter window 
    window = tk.Tk()
    window.title("ASCEND")
    window.minsize(800,400)
    window.tk_setPalette(background="white")
    window.grid_columnconfigure(0, weight=1)

    scroller_config = {
      "master": window,
      "wrap": tk.WORD,
      "height": 5,
      "font": ("Times New Roman", 12),
    }

    #create labels and scrolled text 
    data_label = tk.Label(text="Data")
    data_label.grid(row=0, column=0, sticky=tk.W)

    data_scrolled = scrolledtext.ScrolledText(**scroller_config)
    data_scrolled.grid(row=1, column=0, sticky=tk.W+tk.E)
    data_scrolled.configure(state="disabled")

    
    core0_label = tk.Label(text="Core 0")
    core0_label.grid(row=2, column=0, sticky=tk.W)

    core0_scrolled = scrolledtext.ScrolledText(**scroller_config)
    core0_scrolled.grid(row=3, column=0, sticky=tk.W+tk.E)
    core0_scrolled.configure(state="disabled")

    
    core1_label = tk.Label(text="Core 1")
    core1_label.grid(row=4, column=0, sticky=tk.W)

    core1_scrolled = scrolledtext.ScrolledText(**scroller_config)
    core1_scrolled.grid(row=5, column=0, sticky=tk.W+tk.E)
    core1_scrolled.configure(state="disabled")
    
    
    misc_label = tk.Label(text="Misc")
    misc_label.grid(row=6, column=0, sticky=tk.W)

    misc_scrolled = scrolledtext.ScrolledText(**scroller_config)
    misc_scrolled.grid(row=7, column=0, sticky=tk.W+tk.E)
    misc_scrolled.configure(state="disabled")

    # update function called every half second 
    def update():
      # print("update")
      prefix =  datetime.now().strftime('%H:%M:%S.%f')[:-3] + " -> "
      if self.decoder_packets.empty() == False:
        data_scrolled.configure(state="normal")
        data_scrolled.insert(tk.INSERT, prefix + str(self.decoder_packets.get()) + "\n")
        data_scrolled.yview(tk.END)
        data_scrolled.configure(state="disabled")
      if self.sorter_core0.empty() == False:
        core0_scrolled.configure(state="normal")
        core0_scrolled.insert(tk.INSERT, prefix + str(self.sorter_core0.get()) + "\n")
        core0_scrolled.yview(tk.END)
        core0_scrolled.configure(state="disabled")
      if self.sorter_core1.empty() == False:
        core1_scrolled.configure(state="normal")
        core1_scrolled.insert(tk.INSERT, prefix + str(self.sorter_core1.get()) + "\n")
        core1_scrolled.yview(tk.END)
        core1_scrolled.configure(state="disabled")      
      if self.sorter_misc.empty() == False:
        misc_scrolled.configure(state="normal")
        misc_scrolled.insert(tk.INSERT, prefix + str(self.sorter_misc.get()) + "\n")
        misc_scrolled.yview(tk.END)
        misc_scrolled.configure(state="disabled")

      window.after(500, update)

    window.after(0, update)
  
    def flag_close(): 
      self.end_event.set()
      window.destroy()

    window.protocol("WM_DELETE_WINDOW", flag_close)
    window.mainloop()


# testing code 
if __name__ == '__main__':
  import random # for testing

  end_event = threading.Event()
  sorter_core0 = Queue()
  sorter_core1 = Queue()
  sorter_misc = Queue()
  decoder_packets = Queue()
  print("Starting")
  simple_display = SimpleDisplay(end_event, sorter_core0, sorter_core1, sorter_misc, decoder_packets, )

  simple_display.start()

  for i in range(8):
    if end_event.is_set(): break
    qs = [["core0", sorter_core0], ["core1", sorter_core1], ["misc", sorter_misc], ["packet", decoder_packets]]
    random.shuffle(qs)

    for q in qs:
      r = str(int(random.random() * 5))
      print("Adding", r, "to", q[0])
      q[1].put(q[0] + " " + r)
      time.sleep(int(r)/10)

  simple_display.join()