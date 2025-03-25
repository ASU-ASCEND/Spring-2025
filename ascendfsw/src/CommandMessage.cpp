#include "CommandMessage.h"

mutex_t cmd_data_mutex;
static CommandMessage cmd_data = {CMD_NONE, 0, false};

CommandMessage getCmdData() {
  mutex_enter_blocking(&cmd_data_mutex);
  CommandMessage temp = cmd_data;
  mutex_exit(&cmd_data_mutex);
  return temp;
}

void setCmdData(CommandMessage new_cmd_data) {
  mutex_enter_blocking(&cmd_data_mutex);
  cmd_data = new_cmd_data;
  mutex_exit(&cmd_data_mutex);
}