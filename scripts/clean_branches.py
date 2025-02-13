"""
Prunes branches that are deleted remotely 
"""
import subprocess

print("Fetching from remote...\n")
res = "failure"
try:
  res = subprocess.check_output("git fetch -p".split())
except:
  print()
if(res == "failure"): print("Using local info only.")

print("Getting branch info...")
branch_output = subprocess.check_output("git branch -v".split()).decode("utf-8")

branch_lines = branch_output.splitlines() #[i.strip() for i in branch_output.splitlines()]

gone_branches = []

for line in branch_lines: 
  tokens = line.split()
  if tokens[2] == "[gone]": gone_branches.append(tokens[0])

print("Branches deleted on remote: ", "".join(gone_branches))

print("Pruning on local...")
for branch in gone_branches:
  print(f"Deleting branch {branch}...", end="\t")
  try:
    subprocess.check_output(f"git branch -d {branch}".split())
    print("success.")
  except: 
    print("fail.")

print("Done.")
