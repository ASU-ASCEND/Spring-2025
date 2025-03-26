import os
import sys
import threading
import serial
import serial.tools.list_ports
from datetime import datetime
from time import sleep 
from os import path 
from queue import Queue, Empty

class SerialInput(threading.Thread):
    def __init__(self, end_event: threading.Event, input_queue: Queue, serial_stream: Queue, serial_output: Queue):
        super().__init__()
        self.input_queue = input_queue
        self.end_event = end_event
        self.port = None
        self.ser = None
        self.serial_stream = serial_stream
        self.serial_output = serial_output
        self.baud = 115200
        

    def list_serial_ports(self):
        return [(port.device, port) for port in serial.tools.list_ports.comports()]

    def select_serial_port(self):
        ports = self.list_serial_ports()
        if not ports:
            print("No serial ports found. Exiting.")
            sys.exit(0)
        print("Available serial ports:")
        for i, p in enumerate(ports, start=1):
            print(f"{i}. {p[1]}")
        while self.end_event.is_set() == False:
            try:
                index = int(input("Enter the index of the port you want to use: ")) - 1
                if 0 <= index < len(ports):
                    return ports[index][0]
                print("Invalid selection. Try again.")
            except ValueError:
                print("Please enter a valid integer.")
    
    def select_device(self):
        # for baud 
        while self.end_event.is_set() == False: 
            try: 
                choice = int(input("Set Input Device (0 for Hardline, 1 for Radio): "))
                if choice == 0:
                    self.baud = 115200
                elif choice == 1:
                    self.baud = 57600
                else:
                    raise ValueError
                return 
            except ValueError:
                print("Please enter a valid integer (0 or 1).")

    def command_output(self):
        # send everything it gets to output 
        while self.end_event.is_set() == False:
            try: 
                data_out = self.serial_output.get_nowait()

                self.ser.write(data_out)
            except Empty: 
                sleep(0.1)

    def run(self):
        self.port = self.select_serial_port()
        self.select_device()

        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baud,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                timeout=1.0
            )
        except serial.SerialException as e:
            print(f"Error opening serial port {self.port}: {e}")
            sys.exit(1)

        output_thread = threading.Thread(target=self.command_output)
        output_thread.start()
        while not (self.end_event and self.end_event.is_set()):
            data = self.ser.read(512)   #1024)
            if data:
                # print(data.decode(), end="")
                self.input_queue.put(data)
                self.serial_stream.put(data)
        output_thread.join()

        self.ser.close()

if __name__ == "__main__":
    from queue import Queue
    q = Queue()
    end_event = threading.Event()
    s = SerialInput(end_event, q)
    s.start()
    s.join()