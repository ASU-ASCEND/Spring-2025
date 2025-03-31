import csv
import json

semester = 'spring-2025'

input_csv = f'./data/{semester}/aprs-data.csv'
output_json = f'./data/{semester}/aprs-data.json'

with open(input_csv, 'r', newline='') as csv_file:
    reader = csv.reader(csv_file)
    
    # Read the header
    header = next(reader)
    position = {
        'latitude': header.index('lat'),
        'longitude': header.index('lng'),
        'height': header.index('altitude'),
        'timestamp': header.index('time'),
    }

    # Build a list of all rows as dictionaries
    data_list = []
    for row in reader: # Extract the data we want from each row
        data_dict = {
            'latitude': row[position['latitude']],
            'longitude': row[position['longitude']],
            'height': row[position['height']],
            'timestamp': row[position['timestamp']].replace(' ', 'T') + 'Z',
        }
        data_list.append(data_dict)

# List as a JSON array
with open(output_json, 'w') as json_file:
    json.dump(data_list, json_file, indent=2)
