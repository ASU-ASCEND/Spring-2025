import csv
import threading
from queue import Queue 
from construct import Struct, Const, Int32ul, Int16ul, Array, Byte, Checksum, this

class PacketDecoder(threading.Thread):
    # Define checksum function
    def validate(sensor_data):
        pass

    # Define packet structure
    packet = Struct(
        "sync" / Const(b"\x41\x53\x55\x21"), # ASU!
        "sensor_id" / Int32ub,
        "length" / Int16ub,
        "timestamp" / Int32ub,
        "sensor_data" / Array(this.length, Byte),
        "checksum" / Checksum(Byte, validate, this.sensor_data)
    )

    # Configure packet decoder
    config = {}
    with open('config.csv', mode='r') as file:
        reader = csv.reader(file)
        for row in reader: # Create a dictionary from the CSV file
            if row: 
                sensor_name = row[0]
                measurements = ', '.join(item.strip() for item in row[1:])
                config[sensor_name] = measurements
    
    def __init__(self, sorter_to_decoder: Queue, decoder_packets: Queue):
        super().__init__()
        self.sorter_to_decoder = sorter_to_decoder
        self.decoder_packets = decoder_packets

if __name__ == '__main__':
    pass