#include "NetMessageReader.hh"

#include "NetMessage.hh"

using namespace aion;

void NetMessageReader::pushByte(uint8_t _c)
{
    const int nextMask = 1 << 7;

    if (mOptionsByte == NetMessageOptions::UNINITIALIZED)
    {
        mOptionsByte = _c;
    }
    else if (mCurrMessage == nullptr)
    {
        mSizeByte = (_c & nextMask) != 0;
        _c &= ~nextMask;

        mMsgPos = 0;

        mTypeInfo = _c & 0x3;
        // TODO: check for invalid type

        mMsgSize = _c >> 2;
        mSizePos = 5;

        mCurrMessage = new NetMessage(mOptionsByte, (NetMessage::TypeInformation)mTypeInfo);
        if (mOptionsByte & NetMessageOptions::compressed)
            mCurrMessage->mIsCompressed = true;

        // exception: empty msg
        if (mMsgSize == 0 && !mSizeByte)
        {
            mMessages.send(mCurrMessage);
            mCurrMessage = nullptr;
            mOptionsByte = NetMessageOptions::UNINITIALIZED;
        }

        if (mCurrMessage != nullptr && !mSizeByte)
            mCurrMessage->reserve(mMsgSize);
    }
    else if (mSizeByte)
    {
        mSizeByte = (_c & nextMask) != 0;
        _c &= ~nextMask;

        mMsgSize += _c << mSizePos;
        mSizePos += 7;

        if (mCurrMessage != nullptr && !mSizeByte)
            mCurrMessage->reserve(mMsgSize);
    }
    else
    {
        ++mMsgPos;
        mCurrMessage->appendByte(_c);

        assert(mMsgPos <= mMsgSize);
        if (mMsgPos == mMsgSize)
        {
            mMessages.send(mCurrMessage);
            mCurrMessage = nullptr;
            mOptionsByte = NetMessageOptions::UNINITIALIZED;
        }
    }
}

void NetMessageReader::pushBytes(const char *_data, int _count)
{
    int i = 0;
    while (i < _count)
    {
        if (mCurrMessage == nullptr || mSizeByte)
        {
            pushByte(_data[i]);
            ++i;
        }
        else
        {
            int count = mMsgSize - mMsgPos;
            if (count > _count - i)
                count = _count - i;

            assert(count > 0);

            mCurrMessage->appendData(_data + i, count);
            mMsgPos += count;

            assert(mMsgPos <= mMsgSize);
            if (mMsgPos == mMsgSize)
            {
                mMessages.send(mCurrMessage);
                mCurrMessage = nullptr;
                mOptionsByte = NetMessageOptions::UNINITIALIZED;
            }

            i += count;
        }
    }
}

bool NetMessageReader::receiveMessage(NetMessage *&_msg, bool _doNotDecompress)
{
    bool r = mMessages.receive(_msg);
    if (!_doNotDecompress && r)
        _msg->decompress();
    return r;
}

NetMessageReader::NetMessageReader()
  : mCurrMessage(nullptr), mSizeByte(false), mMsgPos(-1), mMsgSize(-1), mSizePos(-1), mTypeInfo(-1), mOptionsByte(NetMessageOptions::UNINITIALIZED)
{
}

NetMessageReader::~NetMessageReader()
{
    delete mCurrMessage;
    mCurrMessage = nullptr;

    NetMessage *msg(nullptr);
    while (mMessages.receive(msg))
        delete msg;
}
