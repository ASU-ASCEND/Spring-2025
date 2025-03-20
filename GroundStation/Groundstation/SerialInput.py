import os
import sys
import threading
import serial
import serial.tools.list_ports
from datetime import datetime

class SerialInput(threading.Thread):
    def __init__(self, input_queue, end_event=None):
        super().__init__()
        self.input_queue = input_queue
        self.end_event = end_event
        self.port = self.select_serial_port()
        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=115200,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                timeout=1.0
            )
        except serial.SerialException as e:
            print(f"Error opening serial port {self.port}: {e}")
            sys.exit(1)

    def list_serial_ports(self):
        return [port.device for port in serial.tools.list_ports.comports()]

    def select_serial_port(self):
        ports = self.list_serial_ports()
        if not ports:
            print("No serial ports found. Exiting.")
            sys.exit(0)
        print("Available serial ports:")
        for i, p in enumerate(ports, start=1):
            print(f"{i}. {p}")
        while True:
            try:
                index = int(input("Enter the number of the port you want to use: ")) - 1
                if 0 <= index < len(ports):
                    return ports[index]
                print("Invalid selection. Try again.")
            except ValueError:
                print("Please enter a valid integer.")

    def run(self):
        while not (self.end_event and self.end_event.is_set()):
            data = self.ser.read(1024)
            if data:
                for byte in data:
                    self.input_queue.put(byte)
        self.ser.close()

if __name__ == "__main__":
    from queue import Queue
    q = Queue()
    s = SerialInput(q)
    s.start()
    s.join()