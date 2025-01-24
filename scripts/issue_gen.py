"""
Generate issues for sensor packeting
Execute from /scripts 
"""
import os 

milestone = "Packet System (FSW)"
label = "fsw"

classes_path = os.path.join("..","ascendfsw","src")

classes = os.listdir(classes_path)
sensors = [s for s in classes if s.find("Sensor") != -1]
sensor_names = [s[:-10] for s in sensors]
storages = [s for s in classes if s.find("Storage") != -1]
storage_names = [s[:-11] for s in storages]

for name in sensor_names: 
  title = f"Expand the {name} sensor for the packet system" 
  body = f"Implement the readData(uint8_t*& packet) function for the {name} sensor, overriding the one found in Sensor.h for the new packet system. \nAppend the data from the sensor to the packet and return the amount of bytes it appended"
  command = f"gh issue create --title \"{title}\" --body \"{body}\" --label \"{label}\" --milestone \"{milestone}\""
  print(command)
  os.system(command)

for name in storage_names:
  title = f"Expand the {name} storage for the packet system" 
  body = f"Implement the store(uint8_t* packet) function for the {name} storage, overriding the one found in Storage.h for the new packet system. \nReceive and properly store data packets so they can be retrieved after storage."
  command = f"gh issue create --title \"{title}\" --body \"{body}\" --label \"{label}\" --milestone \"{milestone}\""
  print(command)
  os.system(command) 

# for i in range(26,29):
#   os.system(f"gh issue delete {i} --yes")