# ASU ASCEND Ground Station 

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

### When done
Exit the virtual environment with: 
```
deactivate
```