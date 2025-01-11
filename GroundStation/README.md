# Ground Station
Code for the ground station, using the RFD-900x radio

## Setup to run with the python command:
Install python3, run this command in powershell
```
python
```
And then install from windows store


Run this command in powershell to install the pyserial package:
```
pip install pyserial 
```

Download the python file (groundstation.py),  and then to start the groundstation software: 
```
python groundstation.py
```

# Just use the executable:
(Create with: `pyinstaller --onefile groundstation.py`)

To use it just download `/dist/groundstation.exe` and run

# For Backup:
Use run the `/Backup/serialonlybackup.py` script or use `/Backup/dist/serialonlybackup.exe`

These will be terminal applications with no parsing so they're more robust
