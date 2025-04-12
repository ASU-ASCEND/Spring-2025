import csv
import json
import sys

SEMESTER = 'spring-2025'
DATA = 'flash'

input_csv = f'./data/{SEMESTER}/{DATA}-data.csv'
output_json = f'./data/{SEMESTER}/{DATA}-data.json'

with open(input_csv, 'r', newline='') as csv_file:
    reader = csv.reader(csv_file)
    
    # Read the header
    header = next(reader)

    # Parse APRS data
    if DATA == 'aprs':
        position = {
            'latitude': f"{float(header.index('lat')):.4f}",
            'longitude': f"{float(header.index('lng')):.4f}",
            'height': f"{float(header.index('altitude')):.4f}",
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

    # Parse flight GPS data
    elif DATA == 'flash' or DATA == 'sd':
        position = {
            'latitude': header.index('MTK3339 Latitude'),
            'longitude': header.index('MTK3339 Longitude'),
            'height': header.index('MTK3339 Altitude'),
            
            'year': header.index('MTK3339 Year'),
            'month': header.index('MTK3339 Month'),
            'day': header.index('MTK3339 Day'),
            'hour': header.index('MTK3339 Hour'),
            'minute': header.index('MTK3339 Minute'),
            'second': header.index('MTK3339 Second'),
        }

        # Build a list of all rows as dictionaries
        it = 0
        data_list = []
        for row in reader:
            # Check if any required field is missing or empty
            if any(not row[position[field]].strip() for field in ['latitude', 'longitude', 'height', \
                                                                  'year', 'month', 'day', 'hour', 'minute', 'second']):
                continue

            # Check for unusual values
            if int(row[position['year']]) != 2025 or any(float(row[position[field]]) == 0 for field in ('latitude', 'longitude', 'height')):
                continue


            # Concat data into Cesium-readable format
            time = f"{int(row[position['year']]):04d}-{int(row[position['month']]):02d}-{int(row[position['day']]):02d}T" \
                   f"{int(row[position['hour']]):02d}:{int(row[position['minute']]):02d}:{int(row[position['second']]):02d}Z"


            data_dict = {
                'latitude': f"{float(row[position['latitude']]):.4f}",
                'longitude': f"{float(row[position['longitude']]):.4f}",
                'height': f"{float(row[position['height']]) / 1000:.4f}", # Convert to meters
                'timestamp': time,
            }
            data_list.append(data_dict)
            it += 1

    else:
        print(f"ERROR: Incorrect data file - {DATA}")
        sys.exit(1)

# List as a JSON array
with open(output_json, 'w') as json_file:
    json.dump(data_list, json_file, indent=2)

print(f"Total Flight Points: {it}")