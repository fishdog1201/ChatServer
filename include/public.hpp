#pragma once

enum msgType {
    LOGIN_MSG = 1,
    LOGIN_MSG_ACK,

    REG_MSG,
    REG_MSG_ACK,

    ONE_CHAT_MSG,

    ADD_FRIEND_MSG,

    CREATE_GROUP_MSG,
    ADD_GROUP_MSG,
    GROUP_CHAT_MSG
};