import threading
from queue import Queue
from construct import (
    Checksum, ConstError, ChecksumError, ConstructError,
    Const, Array, Struct,
    Int32ul, Int16ul,
    Byte, this
)

class PacketDecoder(threading.Thread):
    # Initialize PacketDecoder
    def __init__(
            self, sorter_to_decoder: Queue, 
            decoder_packets: Queue,
            bitmask_to_struct: dict,
            bitmask_to_name: dict,
            num_sensors: int
    ):
        super().__init__()
        self.sorter_to_decoder = sorter_to_decoder
        self.decoder_packets = decoder_packets
        self.bitmask_to_struct = bitmask_to_struct
        self.bitmask_to_name = bitmask_to_name 
        self.num_sensors = num_sensors

        # Define packet structure
        self.packet_struct = Struct(
            "sync"        / Const(b"ASU!"), # Sync byte: b"\x41\x53\x55\x21"
            "bitmask"     / Int32ul,
            "length"      / Int16ul,
            "timestamp"   / Int32ul,
            "sensor_data" / Array(this.length, Byte),
            "checksum"    / Checksum(Byte, self.validate, this.sensor_data)
        )

    # Validate checksum
    def validate(self, sensor_data):
        total = sum(sensor_data) & 0xFF
        return ((~total) + 1) & 0xFF
    
    # Run PacketDecoder
    def run(self):
        while True:
            # Get packet bytes from sorter
            packet_bytes = self.sorter_to_decoder.get()

            # Parse packet bytes & catch possible errors
            try:
                parsed_packet = self.packet_struct.parse(packet_bytes)
            except ConstError as e: # Catch sync byte mistmatch
                print(f"[ERROR] Sync byte mismatch: {e}")
                continue
            except ChecksumError as e: # Catch checksum validation errors
                print(f"[ERROR] Checksum validation failed: {e}")
                continue
            except ConstructError as e: # Catch-all for other parse errors
                print(f"[ERROR] Packet parsing failed: {e}")
                continue

            # Extract sensor ID & sensor data
            bitmask = parsed_packet.bitmask
            sensor_data = bytes(parsed_packet.sensor_data)
            timestamp = parsed_packet.timestamp

            # Parse each sensor data field
            offset = 0
            parsed = {}
            for bitmask_index in range(self.num_sensors): # Iterate through each sensor (0 -> num_sensors)
                if bitmask & (1 << bitmask_index):
                    if bitmask_index not in self.bitmask_to_struct: # Check if sensor exists
                        print(f"[ERROR] No sensor found for bitmask index: {bitmask_index}")
                        continue

                    # Extract sensor data fields & sensor name
                    sensor_fields = self.bitmask_to_struct[bitmask_index]
                    sensor_name = self.bitmask_to_name[bitmask_index]

                    try: # Parse sensor data fields
                        temp_parsed = sensor_fields.parse(sensor_data[offset:])
                    except ConstructError as e: # Catch errors in parsing sensor data
                        print(f"[ERROR] Parsing sensor {sensor_name} (bitmask {bitmask_index}) failed: {e}")
                        break

                    # Store parsed sensor data
                    parsed[sensor_name] = temp_parsed
                    offset += sensor_fields.sizeof()

            # Store pass parsed packets to queue
            self.decoder_packets.put({
                "timestamp": timestamp,
                "sensor_data": parsed
            })
