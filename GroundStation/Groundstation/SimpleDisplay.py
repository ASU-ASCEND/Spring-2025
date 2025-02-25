import threading
from queue import Queue 
import tkinter as tk
import random # for testing
import time

class SimpleDisplay(threading.Thread): 

  def __init__(self, sorter_core0: Queue, sorter_core1: Queue, sorter_misc: Queue, decoder_packets: Queue):
    super().__init__()
    self.sorter_core0 = sorter_core0
    self.sorter_core1 = sorter_core1
    self.sorter_misc = sorter_misc
    self.decoder_packets = decoder_packets 

  def run(self):
    # do thread stuff
    print("Thread start")
    window = tk.Tk()
    window.title("ASCEND")
    window.minsize(800,400)

    data_label = tk.Label(text="Data")
    data_label.grid(row=0, column=0)

    data_content_label = tk.Label(text="")
    data_content_label.grid(row=1, column=0)

    
    core0_label = tk.Label(text="Core 0")
    core0_label.grid(row=2, column=0)

    core0_content_label = tk.Label(text="")
    core0_content_label.grid(row=3, column=0)

    
    core1_label = tk.Label(text="Core 1")
    core1_label.grid(row=4, column=0)

    core1_content_label = tk.Label(text="")
    core1_content_label.grid(row=5, column=0)
    
    
    misc_label = tk.Label(text="Misc")
    misc_label.grid(row=6, column=0)

    misc_content_label = tk.Label(text="")
    misc_content_label.grid(row=7, column=0)

    def update():
      print("update")
      if self.decoder_packets.empty() == False:
        data_content_label.config(text=self.decoder_packets.get())
      if self.sorter_core0.empty() == False:
        core0_content_label.config(text=self.sorter_core0.get())
      if self.sorter_core1.empty() == False:
        core1_content_label.config(text=self.sorter_core1.get())
      if self.sorter_misc.empty() == False:
        misc_content_label.config(text=self.sorter_misc.get())
      
      window.after(500, update)

    window.after(0, update)
    window.mainloop()


if __name__ == '__main__':
  sorter_core0 = Queue()
  sorter_core1 = Queue()
  sorter_misc = Queue()
  decoder_packets = Queue()
  print("Starting")
  simple_display = SimpleDisplay(sorter_core0, sorter_core1, sorter_misc, decoder_packets, )

  simple_display.start()

  for i in range(5):
    qs = [["core0", sorter_core0], ["core1", sorter_core1], ["misc", sorter_misc], ["packet", decoder_packets]]
    random.shuffle(qs)

    for q in qs:
      r = str(int(random.random() * 5))
      print("Adding", r, "to", q[0])
      q[1].put(q[0] + " " + r)
      time.sleep(int(r))

  simple_display.join()