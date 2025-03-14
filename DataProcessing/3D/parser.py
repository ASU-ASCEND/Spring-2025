import csv
import json

input_csv = './data/aprs-data-fall-2024.csv'
output_json = './data/aprs-data-fall-2024.json'

with open(input_csv, 'r', newline='') as csv_file:
    reader = csv.reader(csv_file)
    
    # Read the header
    header = next(reader)
    position = {
        'latitude': header.index('lat'),
        'longitude': header.index('lng'),
        'altitude': header.index('altitude'),
    }

    # Build a list of all rows as dictionaries
    data_list = []
    for row in reader:
        data_dict = {
            'latitude': row[position['latitude']],
            'longitude': row[position['longitude']],
            'altitude': row[position['altitude']],
        }
        data_list.append(data_dict)

# Now write out the entire list as a JSON array
with open(output_json, 'w') as json_file:
    json.dump(data_list, json_file, indent=2)   # indent=2 for pretty printing
