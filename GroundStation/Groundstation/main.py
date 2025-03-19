import threading
from queue import Queue

import PacketDecoder
import SerialInput
import SerialSorter
import SimpleDisplay

from ConfigLoader import load_config

# Define the path to the configuration file
FILE_PATH = "./config.csv"

if __name__ == "__main__":
    end_event = threading.Event()
    input_to_sorter = Queue()
    sorter_to_decoder = Queue()
    sorter_core0 = Queue()
    sorter_core1 = Queue()
    sorter_misc = Queue()
    decoder_packets = Queue()

    # Load configuration file
    bitmask_to_struct, bitmask_to_name = load_config(FILE_PATH)

    # Create threads
    serial_input = SerialInput.SerialInput(input_to_sorter)
    serial_sorter = SerialSorter.SerialSorter(
        end_event, 
        input_to_sorter, 
        sorter_to_decoder, 
        sorter_core0, 
        sorter_core1, 
        sorter_misc
    )
    
    packet_decoder = PacketDecoder.PacketDecoder(
        sorter_to_decoder, 
        decoder_packets,
        bitmask_to_struct,
        bitmask_to_name
    )
    
    simple_display = SimpleDisplay.SimpleDisplay(
        end_event, 
        sorter_core0, 
        sorter_core1, 
        sorter_misc, 
        decoder_packets
    )

    # Start threads
    serial_input.start()
    serial_sorter.start()
    packet_decoder.start()
    simple_display.start()

    # Wait for threads to finish
    serial_input.join()
    serial_sorter.join()
    packet_decoder.join()
    simple_display.join()