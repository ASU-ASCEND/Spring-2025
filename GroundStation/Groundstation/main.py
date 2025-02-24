import threading
from queue import Queue

import PacketDecoder
import SerialInput
import SerialSorter
import SimpleDisplay

input_to_sorter = Queue()
sorter_to_decoder = Queue()
sorter_core0 = Queue()
sorter_core1 = Queue()
sorter_misc = Queue()
decoder_packets = Queue()

serial_input = SerialInput.SerialInput((input_to_sorter, ))
serial_sorter = SerialSorter.SerialSorter((input_to_sorter, sorter_to_decoder, sorter_core0, sorter_core1, sorter_misc,))
packet_decoder = PacketDecoder.PacketDecoder((sorter_to_decoder, decoder_packets,))
simple_display = SimpleDisplay.SimpleDisplay((sorter_core0, sorter_core1, sorter_misc, decoder_packets, ))

serial_input.start()
serial_sorter.start()
packet_decoder.start()
simple_display.start()

serial_input.join()
serial_sorter.join()
packet_decoder.join()
simple_display.join()