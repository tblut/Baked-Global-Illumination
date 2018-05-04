#pragma once

//! Includes System
#include <cstdint>

#include "shared.hh"

#include "MessageQueue.hh"

namespace aion
{
AION_SHARED(class, NetMessage);

/**
 * @brief Convenience class for reading netmessages
 */
class NetMessageReader
{
private:
    /// Queue of completed messages
    MessageQueue<NetMessage *> mMessages;

    /// Current message
    NetMessage *mCurrMessage;

    /// Is the next byte still a size byte?
    bool mSizeByte;
    /// Position in the msg
    int mMsgPos;
    /// Msg size
    int mMsgSize;
    /// Msg size pos
    int mSizePos;
    /// Type info
    int mTypeInfo;
    /// options byte
    uint8_t mOptionsByte;

    void pushByte(uint8_t _c);

public:
    /// Receives a message or returns false if no msg available
    bool receiveMessage(NetMessage *&_msg, bool _doNotDecompress = false);

    /// "receives" a bytes
    void pushBytes(const char *_data, int _count);

public:
    /// ctor
    NetMessageReader();
    /// dtor
    ~NetMessageReader();
};
}
