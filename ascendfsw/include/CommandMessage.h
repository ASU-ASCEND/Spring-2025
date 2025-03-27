#ifndef COMMAND_MESSAGE_H
#define COMMAND_MESSAGE_H
#include "pico/multicore.h"
#include "pico/stdlib.h"

enum CommandType {
  CMD_NONE,
  CMD_STATUS,
  CMD_DOWNLOAD,
  CMD_DELETE,
  CMD_ERASE_ALL
};

struct CommandMessage {
  CommandType type;
  int file_number;
  bool system_paused;
};

extern mutex_t cmd_data_mutex;

CommandMessage getCmdData();

void setCmdData(CommandMessage new_cmd_data);

#endif  // COMMAND_MESSAGE_H