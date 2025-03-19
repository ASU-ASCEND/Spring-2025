# config_loader.py
import csv
from construct import (
    Struct, Int8ul, Int16ul, Int32ul, Int8sl, Int16sl, Int32sl,
    Float32l, Float64l
)

# Define field types
TYPE_KEY = {
    "uint8_t":  Int8ul,
    "uint16_t": Int16ul,
    "uint32_t": Int32ul,
    "int8_t":   Int8sl,
    "int16_t":  Int16sl,
    "int32_t":  Int32sl,
    "float":    Float32l,
    "double":   Float64l
}

# Convert CSV to Construct-readable format
def load_config(filepath):
    bitmask_to_struct = {}
    bitmask_to_name = {}
    num_sensors = 0

    with open(filepath, 'r', newline='') as f:
        reader = csv.reader(f)
        header = next(reader)
        header = [str(i).strip() for i in header]

        # Identify columns
        bit_index   = header.index('BitIndex')
        sensor_name = header.index('SensorName')

        # Read CSV row-by-row
        for row in reader:
            if not row: # Skip empty lines
                continue

            # Compartmentalize CSV row data
            index = int(row[bit_index])
            name  = row[sensor_name].strip().replace(' ', '_')
            fields = row[2:] 

            # Pair fields two-by-two: (field_label, field_type)
            it = iter(fields)
            pairs = list(zip(it, it))

            struct_fields = []
            for (label, type_str) in pairs:
                # Clean up field & type strings
                label = label.strip().replace(' ', '_')
                type_str = type_str.strip()
                construct_type = TYPE_KEY[type_str]

                # Adjust format for Construct: "FieldName" / <construct_type>
                struct_fields.append(label / construct_type)

            # Increment sensor count
            num_sensors += 1

            # Build Construct struct & store for later use
            bitmask_to_name[index] = name
            bitmask_to_struct[index] = Struct(*struct_fields)

    return bitmask_to_struct, bitmask_to_name, num_sensors
