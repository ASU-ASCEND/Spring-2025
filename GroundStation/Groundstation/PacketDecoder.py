import threading
from queue import Queue
from construct import (
    Checksum, ConstError, ChecksumError, ConstructError,
    Const, Array, Struct,
    Int32ul, Int16ul, Int8sl,
    Byte, Bytes, this, Pointer
)
from time import sleep
from queue import Empty

class PacketDecoder(threading.Thread):
    # Initialize PacketDecoder
    def __init__(
            self, 
            end_event: threading.Event,
            sorter_to_decoder: Queue, 
            decoder_packets: Queue,
            packet_saver: Queue, 
            bitmask_to_struct: dict,
            bitmask_to_name: dict,
            num_sensors: int
    ):
        super().__init__()
        self.end_event = end_event
        self.sorter_to_decoder = sorter_to_decoder
        self.decoder_packets = decoder_packets
        self.packet_saver = packet_saver
        self.bitmask_to_struct = bitmask_to_struct
        self.bitmask_to_name = bitmask_to_name 
        self.num_sensors = num_sensors

        self.header_size = 4 + 4 + 2 + 4
        self.checksum_size = 1
        # Define packet structure
        self.packet_struct = Struct(
            "sync"        / Const(b"ASU!"), # Sync byte: b"\x41\x53\x55\x21"
            "bitmask"     / Int32ul,
            "length"      / Int16ul,
            "timestamp"   / Int32ul,
            "sensor_data" / Array(this.length - self.header_size - self.checksum_size, Byte),
            "checksum"    / Int8sl #Checksum(Byte, self.validate, this)
        )

    # Validate checksum
    def validate(self, packet: bytearray):
        total = 0
        for i in range(len(packet)-1):
            total += int.from_bytes(bytes(packet[i]), byteorder='little', signed=False)

        checksum = int.from_bytes(bytes(packet[-1]), byteorder='little', signed=True)
        return total + checksum == 0
    
    # Run PacketDecoder
    def run(self):
        while self.end_event.is_set() == False:
            # Get packet bytes from sorter
            try: 
                packet_bytes = self.sorter_to_decoder.get_nowait()
            except Empty:
                sleep(0.1)
                continue 

            # Parse packet bytes & catch possible errors
            try:
                if self.validate(packet_bytes) == False: raise ChecksumError("Checksum Error")
                parsed_packet = self.packet_struct.parse(packet_bytes)
            except ConstError as e: # Catch sync byte mistmatch
                print(f"[ERROR] Sync byte mismatch: {e}")
                self.decoder_packets.put("ERROR") # convey this failure to the gui 
                continue
            except ChecksumError as e: # Catch checksum validation errors
                print(f"[ERROR] Checksum validation failed: {e}")
                self.decoder_packets.put("ERROR") # convey this failure to the gui 
                continue
            except ConstructError as e: # Catch-all for other parse errors
                print(f"[ERROR] Packet parsing failed: {e}")
                self.decoder_packets.put("ERROR") # convey this failure to the gui 
                continue
            

            # Extract sensor ID & sensor data
            bitmask = parsed_packet.bitmask
            sensor_data = bytes(parsed_packet.sensor_data)
            timestamp = parsed_packet.timestamp

            # print("Mask: ", bin(bitmask))

            # Parse each sensor data field
            offset = 0
            parsed = {}
            parse_error = False

            for bitmask_index in reversed(range(self.num_sensors)): # Iterate through each sensor (0 -> num_sensors)
                if bitmask & (1 << bitmask_index):
                    if bitmask_index not in self.bitmask_to_struct: # Check if sensor exists
                        print(f"[ERROR] No sensor found for bitmask index: {bitmask_index}")
                        continue

                    # Extract sensor data fields & sensor name
                    sensor_fields = self.bitmask_to_struct[self.num_sensors - bitmask_index - 1]
                    sensor_name = self.bitmask_to_name[self.num_sensors - bitmask_index - 1]

                    try: # Parse sensor data fields
                        temp_parsed = sensor_fields.parse(sensor_data[offset:])
                        # print("Temp Parsed:", temp_parsed)
                    except ConstructError as e: # Catch errors in parsing sensor data
                        print(f"[ERROR] Parsing sensor {sensor_name} (bitmask {bitmask_index}) failed: {e}")
                        parse_error = True 
                        break

                    # Store parsed sensor data
                    parsed[sensor_name] = temp_parsed
                    offset += sensor_fields.sizeof()

            # print(parsed)

            if parse_error == False: 
                # Store pass parsed packets to queue
                self.decoder_packets.put({
                    "timestamp": timestamp,
                    "sensor_data": parsed
                })
                self.packet_saver.put({
                    "timestamp": timestamp, 
                    "sensor_data": parsed 
                })
            else: 
                self.decoder_packets.put("ERROR")
