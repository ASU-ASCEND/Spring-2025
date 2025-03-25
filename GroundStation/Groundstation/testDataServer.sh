
# post some data
echo "POSTING"
curl -i -X POST "127.0.0.1:5000/dataserver?f1=1&f2=2&f3=3"

echo "GETTING"
# get that data 
curl -i -X GET "127.0.0.1:5000/dataserver?fields=f1,f2,f3,f4,f5"

