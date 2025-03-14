import csv
import json

with open('./data/aprs-data-fall-2024.csv', 'r', newline='') as csv_file, \
     open('./data/aprs-data-fall-2024.json', 'w') as json_file:
    
    reader = csv.reader(csv_file)

    # Read the CSV header
    header = next(reader)

    # Locate the indices of our columns of interest
    position = {
        'latitude': header.index('lat'),
        'longitude': header.index('lng'),
        'altitude': header.index('altitude'),
    }

    # Convert each row to JSON and write to the file line-by-line
    for row in reader:
        # Build a dictionary with the data you care about
        data_dict = {
            'latitude': row[position['latitude']],
            'longitude': row[position['longitude']],
            'altitude': row[position['altitude']],
        }
        
        # Write each record as one line of JSON in the output file
        json_file.write(json.dumps(data_dict) + '\n')
