# ASU ASCEND Ground Station 

## Using the ground station 
Before doing these steps make sure you have the most recent ground station software by running `git pull` somewhere in your local repo for the project (for example the folder that this README is in). All of the commands given in these instructions should be executed from the the folder that is README is in.  
### Starting it up 
The first time you run the ground station on a computer you need to either set up the .venv (see [Development: Using venv](#development-using-venv)) or install the dependencies onto your machine directly. Using venv is preferred, and you'll need to do [step 2](#2-activate-the-new-environment) each time you start a new terminal. 

After venv is set up you should see a `(.venv)` somewhere in the prompt the terminal you are using to indicate that the virtual environment is active.

Then to run the ground station software use:
```
python main.py
```
If you get errors saying a package can't be found try activating the environment again even if it does say `(.venv)` in your terminal prompt, sometimes the `(.venv)` is leftover and isn't actually activated. 

### After running
You should see prompt in the terminal for which port to connect to, enter the index of the port for the usb device in the list. And the a prompt for which device to connect to, this is because the radio and pico serial connections use different baud rates so select the device accordingly. 

### Closing the ground station 
To close the ground station, avoid using CTRL+C in the terminal as this has the danger of orphaning processes. Just close out of the GUI and then if a prompt is still open in the terminal you should be able to just press enter and it'll end the terminal portion of the program as well. 

#### If you do use CRTL+C or the software crashes 
If the software ends in an unexpected way or is interrupted it might be ok, but it does run the risk of not properly ending all of the processes associated with it. After that happens if you try to start it up again and see a bunch of new errors, they should be able to be fixed by the following: 
1. Make sure the current run is ended/closed using CRTL+C and force quits 
2. Open Task Manager and then search for and end every Python process (there may be some that aren't part of the ground station, especially if you are using VSCode, so if a process immediately reopens it should be fine to leave)
3. Rerun the software (you may need to restart your venv)  

## Interacting with the RESTful API 
The RESTful API has a single endpoint which can be interacted with via GET and POST. The server is intended to be hosted locally with the url for the single endpoint being: [http://127.0.0.1:5000/dataserver]("http://127.0.0.1:5000/dataserver"). 

### Adding Data
The server holds a single dictionary of values with keys set by the field name when it was posted. When the server receives a POST method with a duplicate key the new value replaces the old one. The POST method with then return the count of fields update in json format. 

#### POST Example Using `curl` 
```
curl -i -X POST "127.0.0.1:5000/dataserver?f1=1&f2=2&f3=3"
```
Results in 
```
HTTP/1.1 200 OK
Server: Werkzeug/3.1.3 Python/3.9.13
Date: Sat, 22 Mar 2025 07:12:51 GMT
Content-Type: application/json
Content-Length: 21
Connection: close

{"fields_updated":3}
```

### Requesting Data
When requesting data the GET method takes a list of fields separated by commas and returns those fields corresponding value, if they exist in the dictionary, along with a boolean field called "missing_fields" which is true if at least one fields was not found, in json format.

#### GET Example Using `curl`
```
curl -i -X GET "127.0.0.1:5000/dataserver?fields=f1,f2,f3,f4,f5"
```
Results in 
```
HTTP/1.1 200 OK
Server: Werkzeug/3.1.3 Python/3.9.13
Date: Sat, 22 Mar 2025 07:16:03 GMT
Content-Type: application/json
Content-Length: 51
Connection: close

{"f1":"1","f2":"2","f3":"3","missing_fields":true}
```
(Both examples can be called together using the script [testDataServer.sh](/testDataServer.sh))

## Development: Using venv 
Use the provided requirements.txt file to automatically install needed dependencies 

### 1\. Create a new local virtual environment 
```
python -m venv .venv
```
### 2\. Activate the new environment 

#### Windows: 
In Powershell: 
```
.\.venv\Scripts\Activate.ps1
```
In Git Bash: 
```
source .venv/Scripts/activate
```
#### Other OS: 
```
source .venv/bin/activate
```

### 3\. Install the requirements.txt
```
pip install -r requirements.txt
```

### When done using the ground station software 
Exit the virtual environment with: 
```
deactivate
```