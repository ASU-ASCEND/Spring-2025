import serial
import time 
from datetime import datetime
import serial.tools.list_ports as sp
import tkinter as tk
import sys
import os
import logging

#----------------- error stuff
class StreamToLogger(object):
  """
  Fake file-like stream object that redirects writes to a logger instance.
  """
  def __init__(self, logger, log_level=logging.INFO):
    self.logger = logger
    self.log_level = log_level
    self.linebuf = ''

  def write(self, buf):
    for line in buf.rstrip().splitlines():
          self.logger.log(self.log_level, line.rstrip())

logging.basicConfig(
level=logging.DEBUG,
format='%(asctime)s:%(levelname)s:%(name)s:%(message)s',
filename="error_output.log",
filemode='a'
)

# stdout_logger = logging.getLogger('STDOUT')
# sl = StreamToLogger(stdout_logger, logging.INFO)
# sys.stdout = sl

stderr_logger = logging.getLogger('STDERR')
sl = StreamToLogger(stderr_logger, logging.ERROR)
sys.stderr = sl
#------------------

print("Starting Ground Station...")
now = datetime.now()
current_time = now.strftime("%H:%M:%S")
print(f"Time: {current_time}\n")

# potential full header, based on sensors in flight software main, used to build header
# avoids trouble from missing transmit header 
full_no_fail_header = [
  "Header,",
  "Millis,",

  "PCFTime, ", 
  # "DS3231Time,DS3231TempC,",
  "BMETemp(C),BMEPress(hPa),BMEHum(%),BMEGas(KOhms),BMEAlt(m),",    
  # "GeigerSensor(CPS),", 
  "INACurr(mA),INAVolt(mV),INAPow(mW),",                   
  "LSM9DS1 AccX, LSM9DS1 AccY, LSM9DS1 AccZ, LSM9DS1 GyroX, LSM9DS1 GyroY, LSM9DS1 GyroZ, LSM9DS1 MagX, LSM9DS1 MagY, LSM9DS1 MagZ,", 
  # "SHTHum(%), SHTTemp(C),",
  "PicoTemp(C),",
  #"WindADC,TempADC,", 
  "SGPTVOC(ppb),SGPeCO2(ppm),SGPTVOCBase(ppb),SGPeCO2Base(ppb),SGPH2,SGPEtha,",
  "BME280RelHum %,BME280Pres Pa,BME280Alt m,BME280TempC,DewPointC,",  
  "ENSAQI,ENSTVOC ppb,ENSECO2 ppm,",
  "UVA2(nm),UVB2(nm),UVC2(nm),",
  "UVA0(nm),UVB0(nm),UVC0(nm),",
  "ICM AccX,ICM AccY,ICM AccZ,ICM GyroX,ICM GyroY,ICM GyroZ,ICM MagX,ICM MagY,ICM MagZ,ICM TempC,",
  "MTK_Date,MTK_Lat,MTKLong,MTKSpeed,MTKAngle,MTKAlt,MTKSats,",
  " "  # final space for matching fsw?
]
# # User enter serial port
ports = list(sp.comports())
for i in ports:
    print(i)
print("Enter the COM Port (COM4, COM5, COM9, COM12, etc.) ")
comport = str(input())
print()

# Open Serial Port, 
ser = serial.Serial(
  port = comport,
  baudrate = 57600,
  parity = serial.PARITY_NONE,
  stopbits = serial.STOPBITS_ONE,
  bytesize = serial.EIGHTBITS,
  timeout = None
  )

folder_path = "data"
if not os.path.exists(folder_path):
  os.makedirs(folder_path)
fileName = f"ASCEND_DATA_{now.strftime('%Y_%m_%d_%H_%M_%S')}.csv"
file = open(os.path.join(folder_path, fileName), "a")
file.close()
print(fileName + " created to hold data. If file exists (it shouldn't), data will be appended\n")
 
HIST_SIZE = 3
TABLE_WIDTH = 9
data_line = []
time_line = []
header = "Receive time,header,"
# header = "Receive time," + "".join(full_no_fail_header)

def _quit():
  print("Done.")
  root.quit()
  sys.exit()

root = tk.Tk()
root.protocol("WM_DELETE_WINDOW", _quit)
root.title("ASU ASCEND Ground Station")

Title = tk.Label(root, font = ("Helvetica", "20", "bold"))
Title.grid(row=0,column=0,padx=(0, 0), pady=(0,0))
Title.config(text=('ASU ASCEND Data'))

# get first transmission and use it to set up gui 
data_cells = []
with open(os.path.join(folder_path, fileName), "a", newline = '\n') as f:
  # last_line = data_line
  data_line.insert(0, str(ser.readline())[2:-5])
  if len(data_line) > HIST_SIZE: data_line.pop() 

  now = datetime.now()
  current_time = now.strftime("%H:%M:%S")
  time_line.insert(0, current_time)
  if len(time_line) > HIST_SIZE: time_line.pop()

  time.sleep(1)

  # parse new header
  header = "Receive time, "
  print(data_line[0].split(",")[0])
  header_field = int(data_line[0].split(",")[0], 16) # read header field, convert from hex 
  header_bin = bin(header_field)[2:]
  header_bin = header_bin[header_bin.find("1"):] # trim to first 1
  for i in range(len(header_bin)): # add each from full header if 1
    if header_bin[i] == "1": header += full_no_fail_header[i]
  print(header)
  f.write(header + "\n")

  print(f"{time_line[0]}, {data_line[0]}")
  f.write(f"{time_line[0]}, {data_line[0]}\n")

header_arr = header.split(",")
for i in range(len(header_arr)):
  header_label = tk.Label(root, font = ("Helvetica", "10", "bold"))
  header_label.grid(row=1 + (i // TABLE_WIDTH) * (HIST_SIZE+1), column=i % TABLE_WIDTH, padx=5, pady=0)
  header_label.config(text=header_arr[i])

for i in range(HIST_SIZE): 
  combined_arr = f"{time_line[0]},{data_line[0]}".split(",") 
  if i < len(time_line):
    combined_arr = f"{time_line[i]},{data_line[i]}".split(",")
  row = []
  for j in range(len(combined_arr)):
    cell_data = tk.StringVar()
    cell_data.set(combined_arr[j])
    row.append(cell_data)
    Data = tk.Label(root, font = ("Helvetica", "10"))
    Data.grid(row=i+2 + (j // TABLE_WIDTH) * (HIST_SIZE + 1), column=j % TABLE_WIDTH, padx=5, pady=0)
    Data.config(textvariable=cell_data)
  data_cells.append(row)

def read_data():
  with open(os.path.join(folder_path, fileName), "a", newline = '\n') as f:
    # last_line = data_line
    data_line.insert(0, str(ser.readline())[2:-5])
    if len(data_line) > HIST_SIZE: data_line.pop() 

    now = datetime.now()
    current_time = now.strftime("%H:%M:%S")
    time_line.insert(0, current_time)
    if len(time_line) > HIST_SIZE: time_line.pop()

    time.sleep(1)

    print(f"{time_line[0]}, {data_line[0]}")
    f.write(f"{time_line[0]}, {data_line[0]}\n")

  for i in range(len(data_line)):
    combined_arr = f"{time_line[i]},{data_line[i]}".split(",")
    # print("Combined arr", combined_arr)
    for j in range(len(combined_arr)):
      data_cells[i][j].set(combined_arr[j])

def gui_handler():
  root.update_idletasks()
  read_data()
  root.update()
  root.after(500, gui_handler) 

root.after(0, gui_handler)
root.mainloop()
