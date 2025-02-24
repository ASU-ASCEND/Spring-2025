import threading
from queue import Queue 

class SimpleDisplay(threading.Thread): 

  def __init__(self, sorter_core0: Queue, sorter_core1: Queue, sorter_misc: Queue, decoder_packets: Queue):
    super().__init__()
    self.sorter_core0 = sorter_core0
    self.sorter_core1 = sorter_core1
    self.sorter_misc = sorter_misc
    self.decoder_packets = decoder_packets 

  def run(self):
    # do thread stuff
    pass
