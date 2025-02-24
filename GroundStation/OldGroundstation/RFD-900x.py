# Import Packages
import serial
import time
import tkinter as tk
import csv
import serial.tools.list_ports as sp

# Prove timing intervals for FDR
from datetime import datetime
now = datetime.now()
current_time = now.strftime("%H:%M:%S")
print("Time = ", current_time, "\n")

Decoded_Raw_Data = []
Final_Data = []

# CSV HEADERS
header = ["Packet Number", "SIV", "FixType", "Latitude", \

          "Longitude", "Altitude", "Year", "Month", "Day", \

          "Hour", "Min", "Sec", "NNV", "NEV", "NDV", "Battery" , \

          "3v3 Supply", "5v Supply", "Radio Supply", "Analog Internal", \

          "Analog External", "Altimeter Temp", "Digital Internal", \

          "Digital Eternal", "Pressure", "Accel A", "Accel Y", "Accel z", \

          "Pitch", "Roll", "Yaw"]

blank = []
fileName = "RFD900x_Data.csv"
file = open(fileName, "a")
file.close()
print(fileName + " created to hold data. If file exists, data will be appended\n")

# write headers when program is restarted

with open(fileName, "a", newline = '\n') as f:
            writer = csv.writer(f, delimiter=',')
            writer.writerow(header)
            file.close()

# Degree Symbol for GUI
degree_sign = u"\N{DEGREE SIGN}"

# Sleep before starting for 1 Second
time.sleep(1)

# 
root = tk.Tk()
root.title("MSGC RFD900x")

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
    baudrate = 57600,
    parity = serial.PARITY_NONE,
    stopbits = serial.STOPBITS_ONE,
    bytesize = serial.EIGHTBITS,
    timeout = None
    )

#
packet = 0 #
siv = "" #
fix = ""#
lat = ""#
lon = ""#
alt = ""#
year = "" #
month = ""#
day = ""#
hour = ""#
minute = ""#
sec = ""#
nedN = ""#
nedE = ""#
nedD = ""#
bat = "" #
bat33 = ""#
bat51 = ""#
bat52 = ""#
aint = ""#
aext = ""#
ptemp = ""#
dint = ""#
dent = ""#
pres = ""#
ax = ""
ay = ""
az = ""
pitch = ""
roll = ""
yaw = ""
a1 = ""

packet_count = 0

def Label_Update():
    print("LAB")

    def count():
        now = datetime.now()
        current_time = now.strftime("%H:%M:%S.%f")
        print("Time = ", current_time, "\n")

        global packet_count
        y = ""
        xx = ""
        xxx = ""
        xxxx = ""
        y = ""
        xx = ""
        xxx = ""
        xxxx = ""

        ser.reset_input_buffer()
        y = ser.readline()

        # CSV
        Decoded_Raw_Data = y.decode("utf-8")
        Final_Data = Decoded_Raw_Data.split(",")

        # write a new line in the csv if there is data
        if len(Final_Data) > 10:
            with open(fileName, "a", newline = '\n') as f:
                writer = csv.writer(f, delimiter=',')
                writer.writerow(Final_Data)
                packet_count = packet_count + 1
                file.close()

        xx = y.decode('utf-8')
        xxx = str(xx)
        xxxx = xxx.split(",")

        # Save radio data
        if len(xxxx) >= 30:
            packet = int(xxxx[0].strip())
            siv = xxxx[1].strip()
            fix = xxxx[2].strip()
            lat = float(xxxx[3].strip())
            lat = lat * .0000001
            lon = float(xxxx[4].strip())
            lon = lon * .0000001
            alt = float(xxxx[5].strip())
            alt = alt / 1000
            year = xxxx[6].strip()
            month = xxxx[7].strip()
            day = xxxx[8].strip()
            hour = xxxx[9].strip()
            minute = xxxx[10].strip()
            sec = xxxx[11].strip()
            nedN = xxxx[12].strip()
            nedE = xxxx[13].strip()
            nedD = xxxx[14].strip()
            bat = xxxx[15].strip()
            bat33 = xxxx[16].strip()
            bat51 = xxxx[17].strip()
            bat52 = xxxx[18].strip()
            aint = xxxx[19].strip()
            aext = xxxx[20].strip()
            ptemp = xxxx[21].strip()
            dint = xxxx[22].strip()
            dent = xxxx[23].strip()
            pres = xxxx[24].strip()
            ax = xxxx[25].strip()
            ay = xxxx[26].strip()
            az = xxxx[27].strip()
            pitch = xxxx[28].strip()
            roll = xxxx[29].strip()
            yaw = xxxx[30].strip()

            #if type(fix) == int:
            if int(fix) == 0:
                a1 = "No Fix"
            elif int(fix) == 1:
                a1 ="Dead Reckoning"
            elif int(fix) == 2:
                a1 ="2D"
            elif int(fix) == 3:
                a1 ="3D"
            elif int(fix) == 4:
                a1 ="GNSS + Dead Reckoning"
            #else:
              #  a1 = " "

        else:
            packet = 1 #
            siv = "" #
            fix = ""#
            lat = ""#
            lon = ""#
            alt = ""#
            year = "" #
            month = ""#
            day = ""#
            hour = ""#
            minute = ""#
            sec = ""#
            nedN = ""#
            nedE = ""#
            nedD = ""#
            bat = "" #
            bat33 = ""#
            bat51 = ""#
            bat52 = ""#
            aint = ""#
            aext = ""#
            ptemp = ""#
            dint = ""#
            dent = ""#
            pres = ""#
            ax = ""
            ay = ""
            az = ""
            pitch = ""
            roll = ""
            yaw = ""
            a1 = ""

        Title = tk.Label(root, font = ("Helvetica", "20"))
        Title.grid(row=0,column=0,padx=(0, 0), pady=(0,0))
        Title.config(text=('RFD-900x Data'))



        # Col 0
        Data1 = tk.Label(root, font = ("Helvetica", "10"))
        Data1.grid(row=1,column=0,padx=(0, 5), pady=(0,0))
        Data1.config(text=(
                                'Current Packet #' + str(packet) + "\n" +
                                'Packets Received: ' + str(packet_count) + '\n' + 
                                #'Packet Loss: ' + str('{0:.2g}'.format(packet_count/packet * 100)) + '%\n' +  
                                
                                #str(round(((packet_count/packet)*100),2))+ "%\n" +
                                'Date: ' + year + "-" + month +"-" + day + "\n" +
                                'ZULU Time: ' + hour + ":" + minute +":" + sec + "\n\n" +
                                'Time: ' + current_time + "\n" +
                                'Battery Voltage: ' + str(bat) + " V\n" +
                                '3.3 Voltage: ' + str(bat33) + " V\n" +
                                '5.0 Voltage: ' + str(bat51) + " V\n" +
                                'Radio Voltage: ' + str(bat52) + " V\n\n" +
                                'Analog Int Temp: ' + str(aint)+ degree_sign + "C\n" +
                                'Analog Ext Temp: ' + str(aext) + degree_sign + "C\n" +
                                'Digital Int Temp: ' + str(dint) + degree_sign + "C\n" +
                                'Digital Ext Temp: ' + str(dent) + degree_sign + "C\n" +
                                'Pres Sensor Temp: ' + str(ptemp) + degree_sign + "C\n")
                            )

        # Col 1
        Data2 = tk.Label(root, font = ("Helvetica", "10"))
        Data2.grid(row=1,column=1,padx=(5, 0), pady=(0,0))
        Data2.config(text=(
                        'Satellites In View: ' + str(siv)+ "\n" +
                        'Fix Type: ' + a1 + " (" + str(fix)+ ")" +  "\n" +
                        'Latitude: ' + (str(round(float(lat), 6))if lat!= "" else "") + "\n" +
                        'Longitude: ' + (str(round(float(lon), 6)) if lon != ""else "") + "\n" +
                        'Altitude: ' + str(alt)+ " m\n" +
                        'Pressure: ' + str(pres) + " mbar\n\n" +
                        'NedNorthVel: ' + str(nedN) + " mm/s\n" +
                        'NedEastVel: ' + str(nedE) + " mm/s\n" +
                        'NedDownVel: ' + str(nedD)  + " mm/s\n\n" +              
                        'Acceleration X: ' + str(ax)+  " m/s\u00b2 \n" +
                        'Acceleration Y: ' + str(ay)+ " m/s\u00b2 \n" +
                        'Acceleration Z: ' + str(az)+  " m/s\u00b2 \n" +
                        'Pitch: ' + str(pitch)+ degree_sign +"\n" +
                        'Roll: ' + str(roll)+ degree_sign +"\n" +
                        'Yaw: ' + str(yaw)+ degree_sign +"" 
            ))        

        #
        now2 = datetime.now()
        current_time2 = now2.strftime("%H:%M:%S.%f")
        #print("Time2 = ", current_time2, "\n")
        

        # Verify program runs in less than 1 second
        #print(now - now2)

        time = now2 - now
        deltatie5 = int(time.microseconds)
        inttime = deltatie5 / 1000000

        #print(deltatie5)
        #print(inttime)
        #print()

        global average
        global averagetime
        global avt

        if average < 1000:
            averagetime = averagetime + inttime
            average = average + 1
            #print(average)

        else:
            #print(averagetime)
            #print(averagetime / 100)

            avt = round((averagetime / 1000), 3)
        


            #print()
            #print("\nFrom 1000 averages, this program takes " + str(avt) \
                 # + " seconds to update the display. \n")
        
        #Schedule the count function to run again afte r1000 milliseconds
        root.after(500, count)

    count()

average = 0
averagetime = 0
avt = 0



# 
root.after(0, Label_Update)

#
root.mainloop()

