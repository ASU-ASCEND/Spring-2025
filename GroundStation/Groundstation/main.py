import threading
from queue import Queue

import PacketDecoder
import SerialInput
import SerialSorter
import SimpleDisplay
import PacketSaver
import SerialSaver 
import ServerProcess
import ServerInterface
import GUI
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
    sorter_flash = Queue()
    decoder_packets = Queue()
    decoder_packet_saver = Queue()
    packet_saver_server = Queue()
    serial_stream = Queue()
    serial_output = Queue()

    # Load configuration file
    bitmask_to_struct, bitmask_to_name, num_sensors = load_config(FILE_PATH)

    # set up header 
    header_arr = ["Millis"]
    header_key = {
      "Millis": 1, 
    }
    sensor_arr = []
    sensor_reading_order_key = {}
    with open(FILE_PATH) as f:
      for line in f: 
        fields = [str(i).strip() for i in line.split(",")]
        if fields[0] == "BitIndex": continue 

        # add to header_key 
        header_key[fields[1]] = len(fields) // 2 - 1 
        sensor_arr.append(fields[1])
        sensor_reading_order_key[fields[1]] = [] 

        # populate header array 
        for i in range(2, len(fields), 2): 
          header_arr.append(fields[1] + " " + fields[i]) # spaces are ok (only for display)
          sensor_reading_order_key[fields[1]].append("_".join(fields[i].split())) # no spaces in actual keys only underscores 
    header_info = (header_key, header_arr, sensor_arr, sensor_reading_order_key)

    # Create threads
    serial_input = SerialInput.SerialInput(
        end_event,
        input_to_sorter,
        serial_stream, 
        serial_output
    )

    serial_saver = SerialSaver.SerialSaver(
       end_event,
       serial_stream
    )

    serial_sorter = SerialSorter.SerialSorter(
        end_event, 
        input_to_sorter, 
        sorter_to_decoder, 
        sorter_core0, 
        sorter_core1, 
        sorter_misc,
        sorter_flash
    )
    
    packet_decoder = PacketDecoder.PacketDecoder(
        end_event,
        sorter_to_decoder, 
        decoder_packets,
        decoder_packet_saver,
        bitmask_to_struct,
        bitmask_to_name,
        num_sensors
    )

    packet_saver = PacketSaver.PacketSaver(
        end_event, 
        decoder_packet_saver, 
        packet_saver_server,
        header_info
    )
    
    # simple_display = SimpleDisplay.SimpleDisplay(
    #     end_event, 
    #     sorter_core0, 
    #     sorter_core1, 
    #     sorter_misc, 
    #     decoder_packets
    # )
    gui = GUI.GUI(
        end_event,
        sorter_core0,
        sorter_core1,
        sorter_misc,
        sorter_flash, 
        decoder_packets,
        [bitmask_to_struct, bitmask_to_name, num_sensors],
        header_info, 
        serial_output
    )

    server_process = ServerProcess.ServerProcess()

    server_interface = ServerInterface.ServerInterface(
       end_event,
       server_process,
       packet_saver_server
    )

    # start process for server
    server_process.start()
    sleep(1) # wait for server to start so that port selection prints after

    # Start threads
    serial_input.start()
    serial_saver.start()
    serial_sorter.start()
    packet_decoder.start()
    packet_saver.start()
    server_interface.start()
    # simple_display.start()

    # run the gui in the main thread 
    gui.run()

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
    serial_saver.join()
    serial_sorter.join()
    packet_decoder.join()
    packet_saver.join()
    server_interface.join()
    # simple_display.join()

    # wait for process to finish 
    server_process.join()