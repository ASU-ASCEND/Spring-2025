#ifndef COMMAND_MESSAGE_H
#define COMMAND_MESSAGE_H

enum CommandType {
    CMD_NONE,
    CMD_STATUS,
    CMD_DOWNLOAD,
    CMD_DELETE
};

struct CommandMessage {
    CommandType type;
    int file_number;
    bool system_paused;
};

#endif // COMMAND_MESSAGE_H