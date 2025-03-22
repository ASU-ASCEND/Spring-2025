import multiprocessing.process
from flask import Flask, jsonify, request 
from flask_restful import Resource, Api
import multiprocessing
import threading

data_write_lock = threading.Lock()
server_current_data = {}
class DataTransfer(Resource):

  def get(self):
    # print("GET hit")
    target_fields = request.args.get('fields').split(",")

    data = {}
    found_all = True
    for field in target_fields:
      if field in server_current_data:
        data[field] = server_current_data[field]
      else:
        found_all = False

    data["no_missing_fields"] = found_all

    return jsonify(data)
  
  def post(self):
    print(server_current_data)
    # args = parser.parse_args()
    # print("data:", args['data'])
    args = request.args.to_dict()
    fields_updated = 0
    with data_write_lock:
      for k in args.keys():
        server_current_data[k] = args[k]
        fields_updated += 1
    
    return jsonify({'updated_field_count': fields_updated})


class ServerProcess(multiprocessing.Process):
  
  def __init__(self):
    super().__init__()
    # build the server in here
    self.server_name = "DataServer"
    

  def run(self):
    self.app = Flask(self.server_name)
    api = Api(self.app)

    api.add_resource(DataTransfer, "/dataserver")

    if __name__ == "__main__":
      self.app.run(debug=True)
    else:
      self.app.run(debug=False)

if __name__ == "__main__":
  from time import sleep
  server = ServerProcess()

  server.start()

  sleep(5)
  server.terminate()

  server.join()