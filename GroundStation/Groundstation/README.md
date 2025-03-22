# ASU ASCEND Ground Station 

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