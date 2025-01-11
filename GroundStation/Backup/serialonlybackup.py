import serial
import time 
from datetime import datetime
import serial.tools.list_ports as sp
import os

# User enter serial port
ports = list(sp.comports())
for i in ports:
    print(i)
print("Enter the COM Port (COM4, COM5, COM9, COM12, etc.) ")
comport = str(input())
print()

# Open Serial Port, 
ser = serial.Serial(
  port = comport,
  baudrate = 112500,
  parity = serial.PARITY_NONE,
  stopbits = serial.STOPBITS_ONE,
  bytesize = serial.EIGHTBITS,
  timeout = None
  )

folder_path = "data"
if not os.path.exists(folder_path):
  os.makedirs(folder_path)
now = datetime.now()
fileName = f"ASCEND_DATA_{now.strftime('%Y_%m_%d_%H_%M_%S')}.csv"
file = open(os.path.join(folder_path, fileName), "a")
file.close()
print(fileName + " created to hold data. If file exists (it shouldn't), data will be appended\n")
 

while(1):
  with open(os.path.join(folder_path, fileName), "a", newline = '\n') as f:
    # last_line = data_line
    data = str(ser.readline())[2:-5]

    now = datetime.now()
    current_time = now.strftime("%H:%M:%S")

    # time.sleep(1)

    print(f"{current_time}, {data}")
    f.write(f"{current_time}, {data}\n")