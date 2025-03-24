import multiprocessing.process
from flask import Flask, jsonify, request 
from flask_restful import Resource, Api
import multiprocessing
import threading
import logging 
from logging.handlers import RotatingFileHandler 
from os import path, mkdir 

data_write_lock = threading.Lock()
server_current_data = {}

class AllData(Resource):

  def get(self):
    
    return jsonify(server_current_data)

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

    data["missing_fields"] = not found_all

    return jsonify(data)
  
  def post(self):
    # print(server_current_data)
    # args = parser.parse_args()
    # print("data:", args['data'])
    args = request.args.to_dict()
    fields_updated = 0
    with data_write_lock:
      for k in args.keys():
        server_current_data[k] = args[k]
        fields_updated += 1
    
    return jsonify({'fields_updated': fields_updated})


class ServerProcess(multiprocessing.Process):
  
  def __init__(self):
    super().__init__()
    # build the server in here
    self.server_name = "DataServer"
    if path.isdir("session_data") == False:
      mkdir("session_data")
    self.log_file = path.join("session_data", '_server.log')
    

  def run(self):
    self.app = Flask(self.server_name)
    api = Api(self.app)

    api.add_resource(DataTransfer, "/dataserver")
    api.add_resource(AllData, "/alldata")

    handler = RotatingFileHandler(self.log_file, maxBytes=1000000, backupCount=1)
    handler.setLevel(logging.INFO)
    # self.app.logger.addHandler(handler)
    log = logging.getLogger('werkzeug')
    log.setLevel(logging.INFO)
    log.addHandler(handler)

    self.app.run(debug=False)

if __name__ == "__main__":
  from time import sleep
  server = ServerProcess()

  server.start()

  sleep(10)
  server.terminate()

  server.join()