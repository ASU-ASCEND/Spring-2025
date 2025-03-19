import threading
from queue import Queue

import PacketDecoder
import SerialInput
import SerialSorter
import SimpleDisplay
from time import sleep

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
    bitmask_to_struct, bitmask_to_name, num_sensors = load_config(FILE_PATH)

    # Create threads
    serial_input = SerialInput.SerialInput(
        end_event,
        input_to_sorter
    )

    serial_sorter = SerialSorter.SerialSorter(
        end_event, 
        input_to_sorter, 
        sorter_to_decoder, 
        sorter_core0, 
        sorter_core1, 
        sorter_misc
    )
    
    packet_decoder = PacketDecoder.PacketDecoder(
        end_event,
        sorter_to_decoder, 
        decoder_packets,
        bitmask_to_struct,
        bitmask_to_name,
        num_sensors
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

    # useful for finding straggling threads 
    # while True:
    #     print(
    #         "serial_input:", serial_input.is_alive(),
    #         "serial_sorter:", serial_sorter.is_alive(),
    #         "packet_decoder:", packet_decoder.is_alive(),
    #         "simple_display:", simple_display.is_alive()
    #     )
    #     sleep(1)
        

    # Wait for threads to finish
    serial_input.join()
    serial_sorter.join()
    packet_decoder.join()
    simple_display.join()