import tkinter as tk 

class DataFrame(tk.Frame):

  def __init__(self, container):
    super().__init__(container)
    self.file_list = []
    self.COLUMNS = 4

    button_config = {
      "width": 20, 
      "background": "skyblue"
    }
    button_cell_config = {
      "padx": 20,
      "pady": (5,10),
      # "sticky": "nsew",
      "columnspan": 1, 
    }

    # included widgets 
    self.label = tk.Label(self, text="DataFrame")
    self.label.grid(row=0, column=0)

    self.status_label = tk.Label(self, text="Status: -")
    self.status_label.grid(row=0, column=1, columnspan=self.COLUMNS-1, sticky=tk.W)

    self.get_file_list_button = tk.Button(self, text="Get File List", **button_config, command=self.get_file_list)
    self.get_file_list_button.grid(row=1, column=0, **button_cell_config)

    self.file_list_widgets = []

    self.get_file_list()

  def transfer_file(self, file_name: str):
    # implement with serial interface
    self.status_label.config(text=f"Status: File transferred as data/{file_name}")

  def delete_file(self, file_name: str):
    # implement with serial interface
    self.file_list.remove(file_name)

    self.update_file_list()
    self.status_label.config(text=f"Status: {file_name} has been deleted")
    
  def get_file_list(self):
    # implement with serial interface  
    self.file_list = ["file1", "file2"]

    self.update_file_list()
    self.status_label.config(text=f"Status: File list retrieved")

  def update_file_list(self):
    file_name_config = {

    }
    file_button_config = {
      "width": 20, 
      "background": "skyblue"
    }
    cell_config = {
      "padx": 20,
      "pady": (5,10),
      # "sticky": "nsew",
      "columnspan": 1, 
    }

    for i in self.file_list_widgets: 
      i.destroy()

    def transfer_file_factory(file_name):
      def action():
        self.transfer_file(file_name)
      return action
    
    def delete_file_factory(file_name):
      def action():
        self.delete_file(file_name)
      return action 

    row_index = 2
    for file in self.file_list:

      file_entry = [
        tk.Label(self, text=file, **file_name_config),
        tk.Button(self, text="Transfer", **file_button_config, command=transfer_file_factory(file)),
        tk.Button(self, text="Delete", **file_button_config, command=delete_file_factory(file))
      ]

      column_index = 0
      for widget in file_entry:
        widget.grid(row=row_index, column=column_index, **cell_config)
        column_index += 1

      self.file_list_widgets.extend(file_entry)
      row_index += 1

  
  def update(self):
    pass
