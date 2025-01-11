#!/bin/bash
# Standardizes the format of every .h, .cpp, and .ino file in the repo
# Intended to be executed in the OrganizationalScripts folder 

cd ..

FILES=$(find -not -path "*/.*" -not -path "*/lib/*" -not -path "*/docs/*" | grep -E "\.h|\.cpp|\.ino$")

echo "Formatting:"
echo $FILES

clang-format -style=Google -i $FILES

echo "Done."