import threading
from queue import Queue, Empty
from time import sleep 

# .encode("UTF-8")

class SerialSorter(threading.Thread):

  # calc once
  encodings = {
    "ASU!": "ASU!".encode(),
    "[Core 0]": "[Core 0]".encode(),
    "[Core 1]": "[Core 1]".encode()
  }

  def __init__(self, end_event: threading.Event, input_to_sorter: Queue, sorter_to_decoder: Queue, sorter_core0: Queue, sorter_core1: Queue, sorter_misc: Queue):
    super().__init__()
    self.input_to_sorter = input_to_sorter
    self.sorter_core0 = sorter_core0
    self.sorter_core1 = sorter_core1
    self.sorter_misc = sorter_misc
    self.sorter_to_decoder = sorter_to_decoder 
    self.buf = bytearray()
    self.end_event = end_event

  def run(self):
    while self.end_event.is_set() == False: 
      try: 
        # get input as bytes 
        c_in: bytes = self.input_to_sorter.get_nowait()
        # append to bytearray 
        self.buf += c_in

        if self.buf.find(self.encodings["ASU!"]) != -1:
          # print("hit packet")
          # flush before to misc 
          before, sep, after = self.buf.partition(self.encodings["ASU!"])
          if before.decode().strip() != "": self.sorter_misc.put(before.decode())
          self.buf = sep + after 

          while(len(self.buf) < 4+4+2):
            if self.end_event.is_set(): return 
            try: 
              c_in = self.input_to_sorter.get_nowait()
              self.buf.append(c_in)
            except Empty:
              sleep(0.1)
          
          # pull length out of what we have of the packet (bytes 8 and 9)
          packet_len = int.from_bytes(self.buf[8:10], byteorder="little", signed=False)

          while len(self.buf) < packet_len:
            if self.end_event.is_set(): return 
            try: 
              c_in = self.input_to_sorter.get_nowait()
              self.buf.append(c_in)
            except Empty:
              sleep(0.1)
          
          # now self.buf has an entire packet (or at least has a packet worth of bytes)
          # send to be decoded
          self.sorter_to_decoder.put(self.buf.copy())
          
          # clear buffer 
          self.buf.clear()

        elif self.buf.find(self.encodings["[Core 0]"]) != -1: 
          # flush before to misc
          before, sep, after = self.buf.partition(self.encodings["[Core 0]"])
          if before.decode().strip() != "": self.sorter_misc.put(before.decode())
          self.buf = after 

          # wait for \n to end message if not already received (probably not)
          if self.buf.find("\n".encode()) == -1: 
            c_in = self.input_to_sorter.get(block=True)
            self.buf.append(c_in)
            while c_in != "\n".encode()[0]:
              if self.end_event.is_set(): return 
              try: 
                c_in = self.input_to_sorter.get_nowait()
                self.buf.append(c_in)
              except Empty:
                sleep(0.1)

          # pull out message until '\n' 
          message, endline, after = self.buf.partition("\n".encode())

          self.sorter_core0.put(sep.decode() + message.decode())
          self.buf = after

        elif self.buf.find(self.encodings["[Core 1]"]) != -1: 
          # flush before to misc
          before, sep, after = self.buf.partition(self.encodings["[Core 1]"])
          if before.decode().strip() != "": self.sorter_misc.put(before.decode())
          self.buf = after 

          # wait for \n to end message if not already received (probably not)
          if self.buf.find("\n".encode()) == -1: 
            c_in = self.input_to_sorter.get(block=True)
            self.buf.append(c_in)
            while c_in != "\n".encode()[0]:
              if self.end_event.is_set(): return
              try: 
                c_in = self.input_to_sorter.get_nowait()
                self.buf.append(c_in)
              except Empty:
                sleep(0.1)

          # pull out message until '\n' 
          message, endline, after = self.buf.partition("\n".encode())

          self.sorter_core1.put(sep.decode() + message.decode())
          self.buf = after         
        elif self.buf.find("\n".encode()) != -1:
          before, sep, after = self.buf.partition("\n".encode())
          # flush before to misc 
          self.sorter_misc.put(before.decode())
          # erase it 
          self.buf = after

      except Empty:
        sleep(0.1)



if __name__ == '__main__':
  import SimpleDisplay
  from random import randint

  end_event = threading.Event()
  input_to_sorter = Queue()
  sorter_core0 = Queue()
  sorter_core1 = Queue()
  sorter_misc = Queue()
  decoder_packets = Queue()
  print("Starting")

  sorter = SerialSorter(end_event=end_event, input_to_sorter=input_to_sorter, sorter_to_decoder=decoder_packets, sorter_core0=sorter_core0, sorter_core1=sorter_core1, sorter_misc=sorter_misc)
  simple_display = SimpleDisplay.SimpleDisplay(end_event=end_event, sorter_core0=sorter_core0, sorter_core1=sorter_core1, sorter_misc=sorter_misc, decoder_packets=decoder_packets, )

  simple_display.start()
  sorter.start()

  # add to sorter input
  test_input = "Hello\n ASCEND\n testing testing\n Done [Core 0] From core 0\n[Core 1] from core 1\n[Core 0] Hello, hello\n[Core 1] Hey\n back to misc\n tail".encode() + "ASU!".encode() + bytearray([0x00,0x00,0x1F,0x46,0x0C,0x00, 0xAB, 0xCD])
  for i in test_input: 
    if end_event.is_set(): break
    input_to_sorter.put(i)
    sleep(randint(1, 100) / 1000)

  sorter.join()
  simple_display.join()

