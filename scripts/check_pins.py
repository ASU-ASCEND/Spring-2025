import sys
import os

path = os.path.join("..","ascendfsw","include","PayloadConfig.h")

pins = {}
conflicts = {}
with open(path, "r") as f:
  for line in f:
    tokens = line.split()
    if(len(tokens) != 3): continue 
    if(tokens[0] == "#define" and tokens[1].lower().find("pin") != -1):
      if tokens[2] not in pins:
        pins[tokens[2]] = tokens[1]
      else:
        if tokens[2] not in conflicts: 
          conflicts[tokens[2]] = [pins[tokens[2]]]
        conflicts[tokens[2]].append(tokens[1])

if len(conflicts) != 0:
  print("Conficts")
  for key, value in conflicts.items():
    print(f"Conflict on pin #{key}, conflicting macros: {', '.join(value)}")
else:
  print("No conflicts")    
