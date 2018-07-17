#include "NetMessage.hh"

#include "snappy/snappy.hh"

#include <fstream>

#include "NetMessageReader.hh"

using namespace aion;

void NetMessage::appendData(const char *data, std::size_t count)
{
    mData.resize(mData.size() + count);
    memcpy(&mData[mData.size() - count], data, count);
}

void NetMessage::appendByte(char _c)
{
    mData.push_back(_c);
}

void NetMessage::readData(char *data, std::size_t count)
{
    if (mPosition + count > mData.size())
    {
        mGood = false;
        if (mVerboseErrors)
        {
            std::cerr << "Tried to read more data from a NetMessage than present" << std::endl;
            assert(0);
        }
        return;
    }

    memcpy(data, &mData[mPosition], count);
    mPosition += count;
}

namespace
{
// modifies the hash depending on a given value
int32_t rehash(int32_t hash, int val)
{
    return (int32_t)((hash + (long)0x00C0FFEE + val) * 0x7FFFFFED);
}
// calculates a hopefully as unique as possible 32bit hash for a given type/field pair
int32_t calculateTypeHash(const std::string &type, const std::string &field)
{
    int32_t hash = (int)0xdeadbeef;
    for (unsigned int i = 0; i < type.size(); ++i)
        hash = rehash(hash, 0x12345 + 0x78900 * i + (int)type[i]);
    hash = rehash(hash, 0xbaadf00d);
    for (unsigned int i = 0; i < field.size(); ++i)
        hash = rehash(hash, 0x54321 + 0x98700 * i + (int)field[i]);
    return hash;
}
}

void NetMessage::writeTypeInfo(const std::string &type, const std::string &field)
{
    switch (mTypeInfo)
    {
    case TypeInformation::None:
        break;
    case TypeInformation::Hashes:
    {
        int32_t hash = calculateTypeHash(type, field);
        appendData((char *)&hash, sizeof(hash));
    }
    break;
    case TypeInformation::Strings:
    {
        unsigned int size = (unsigned)type.size();
        if (size > MaxStringSize)
        {
            if (mVerboseErrors)
                std::cerr << "The given type std::string has size " << size << " which is too long (consider "
                                                                               "adjusting MaxStringSize)"
                          << std::endl;
            assert(0);
            mGood = false;
            return;
        }
        appendData((char *)&size, sizeof(size));
        if (size > 0)
            appendData((char *)type.c_str(), size);

        size = (unsigned)field.size();
        if (size > MaxStringSize)
        {
            if (mVerboseErrors)
                std::cerr << "The given field std::string has size " << size << " which is too long (consider "
                                                                                "adjusting MaxStringSize)"
                          << std::endl;
            assert(0);
            mGood = false;
            return;
        }
        appendData((char *)&size, sizeof(size));
        if (size > 0)
            appendData((char *)field.c_str(), size);
    }
    break;
    default:
        assert(0 && "invalid type info");
        mGood = false;
        return;
    }
}

void NetMessage::verifyTypeInfo(const std::string &type, const std::string &field)
{
    switch (mTypeInfo)
    {
    case TypeInformation::None:
        break;
    case TypeInformation::Hashes:
    {
        int32_t expected_hash = calculateTypeHash(type, field);
        int32_t hash = 0;
        readData((char *)&hash, sizeof(hash));
        if (expected_hash != hash)
        {
            if (mVerboseErrors)
            {
                std::cerr << "Expected type/field hash " << expected_hash << " but read " << hash << " (type '" << type
                          << "', field '" << field << "')" << std::endl;
                assert(0);
            }
            mGood = false;
        }
    }
    break;
    case TypeInformation::Strings:
    {
        // Read Type Info
        unsigned int size = 0;
        readData((char *)&size, sizeof(size));
        if (size > MaxStringSize)
        {
            if (mVerboseErrors)
            {
                std::cerr << "Trying to read a std::string with size " << size << "which is quite too long" << std::endl;
                assert(0);
            }
            mGood = false;
            return;
        }
        char *stype = new char[size + 1];
        if (size > 0)
            readData(stype, size);
        stype[size] = '\0';

        // Read Field Info
        readData((char *)&size, sizeof(size));
        if (size > MaxStringSize)
        {
            if (mVerboseErrors)
            {
                std::cerr << "Trying to read a std::string with size " << size << "which is quite too long" << std::endl;
                assert(0);
            }
            delete[] stype;
            mGood = false;
            return;
        }
        char *sfield = new char[size + 1];
        if (size > 0)
            readData(sfield, size);
        sfield[size] = '\0';

        // Verify Type and Field
        if (strcmp(stype, type.c_str()) != 0 || strcmp(sfield, field.c_str()) != 0)
        {
            mGood = false;
            if (mVerboseErrors)
            {
                std::cerr << "Expected [type `" << type << "', field `" << field << "'] but found [type `" << stype
                          << "', field `" << sfield << "']" << std::endl;
                assert(0);
            }
            delete[] stype;
            delete[] sfield;
            return;
        }
        else
        {
            delete[] stype;
            delete[] sfield;
        }
    }
    break;
    default:
        assert(0 && "invalid type info");
        mGood = false;
        return;
    }
}

void NetMessage::internalWriteIntPacked(int64_t value)
{
    // write sign and first byte
    uint8_t data = value >= 0 ? 0 : 1;
    uint64_t val = value >= 0 ? value : -value;
    data |= (val & 0x3F) << 1;
    assert((data & 0x80) == 0);

    val >>= 6;

    do
    {
        // append 1 bit if continue
        if (val > 0)
            data |= 0x80;
        assert(((data & 0x80) == 0) == (val == 0));

        // add data
        mData.push_back(data);

        data = val & 0x7F;
        val >>= 7;
    } while (val > 0 || data != 0);
}

int64_t NetMessage::internalReadIntPacked()
{
    // read
    uint64_t val = 0;
    bool positive = true;

    // first byte
    uint8_t data = 0;
    readData((char *)&data, 1);

    // read sign
    if (data & 1)
        positive = false;
    data >>= 1;

    val += data & 0x3F;

    // read more data
    int pos = 6;
    bool more = (data & 0x40) != 0;
    while (more)
    {
        readData((char *)&data, 1);
        more = (data & 0x80) != 0;
        val += (uint64_t)(data & 0x7F) << pos;
        pos += 7;
    }

    return (int64_t)val * (positive ? 1 : -1);
}

NetMessage::NetMessage(NetMessageOptions::Type options, TypeInformation typeInfo)
  : mPosition(0), mGood(true), mVerboseErrors(true), mTypeInfo(typeInfo), mOptions(options), mIsCompressed(false)
{
    assert(options != NetMessageOptions::UNINITIALIZED);
    mOptions |= NetMessageOptions::compressed;
}

NetMessage *NetMessage::clone() const
{
    auto msg = new NetMessage(mOptions, mTypeInfo);
    msg->mGood = mGood;
    msg->mPosition = mPosition;
    msg->mVerboseErrors = mVerboseErrors;
    msg->mIsCompressed = mIsCompressed;
    if (mData.size() > 0)
        msg->appendData((char *)(void *)&mData[0], mData.size());
    return msg;
}

void NetMessage::setData(const std::vector<char> &_data, bool _compressed)
{
    mData = _data;
    mPosition = 0;
    mIsCompressed = _compressed;
}

void NetMessage::setData(const char *_data, size_t _dataSize, bool _compressed)
{
    // TODO(marc): which is faster?
    // mData.assign(_data, _data+_dataSize);
    mData.resize(_dataSize);
    std::copy(_data, _data + _dataSize, mData.begin());
    mPosition = 0;
    mIsCompressed = _compressed;
}

UniqueNetMessage NetMessage::cloneUnique() const
{
    return UniqueNetMessage(clone());
}

void NetMessage::send(std::vector<char> &output)
{
    compress();

    unsigned int size = (unsigned)mData.size();

    // assemble header

    // Write type and option byte
    output.clear();
    output.push_back(mOptions);


    // decompose size
    unsigned int firstSize = size & 0x1f;
    unsigned int secondSize = (size - firstSize) & (0x7f << 5);
    unsigned int thirdSize = (size - secondSize) & (0x7f << (5 + 7));
    unsigned int forthSize = (size - thirdSize) & (0x7f << (5 + 7 + 7));
    if (firstSize + secondSize + thirdSize + forthSize != size)
    {
        if (mVerboseErrors)
            std::cerr << "NetMessage with size " << size << " is too big!" << std::endl;
        assert(0);
        return;
    }
    assert((firstSize & ~0x1f) == 0);
    assert((secondSize & ~(0x7f << 5)) == 0);
    assert((thirdSize & ~(0x7f << (5 + 7))) == 0);
    assert((forthSize & ~(0x7f << (5 + 7 + 7))) == 0);

    // first byte: 0..1 type info, 2..6 first 5 bit size
    char firstByte = 0;
    firstByte += (unsigned int)mTypeInfo & 0x3;
    firstByte += (firstSize << 2);
    assert((firstByte & (1 << 7)) == 0);
    bool needNextByte = false;
    if (size > firstSize)
    {
        firstByte += 1 << 7;
        needNextByte = true;
    }
    output.push_back(firstByte);

    if (needNextByte)
    {
        // second byte: 0..6 next 7 bit size
        char secondByte = secondSize >> 5;
        assert((secondByte & (1 << 7)) == 0);

        needNextByte = false;
        if (size > firstSize + secondSize)
        {
            secondByte += 1 << 7;
            needNextByte = true;
        }
        output.push_back(secondByte);

        if (needNextByte)
        {
            // third byte: 0..6 next 7 bit size
            char thirdByte = thirdSize >> (5 + 7);
            assert((thirdByte & (1 << 7)) == 0);

            needNextByte = false;
            if (size > firstSize + secondSize + thirdSize)
            {
                thirdByte += 1 << 7;
                needNextByte = true;
            }
            output.push_back(thirdByte);

            if (needNextByte)
            {
                // forth byte: 0..6 next 7 bit size
                char forthByte = forthSize >> (5 + 7 + 7);
                assert((forthByte & (1 << 7)) == 0);

                needNextByte = false;
                if (size > firstSize + secondSize + thirdSize + forthSize)
                {
                    forthByte += 1 << 7;
                    needNextByte = true;
                }
               output.push_back(forthByte);

                assert(!needNextByte);
            }
        }
    }

    // write actual data
    if (size > 0)
    {
        auto hSize = output.size();
        output.resize(output.size() + size);
        std::copy(begin(mData), end(mData), begin(output) + hSize);
    }
}

void NetMessage::writeToFile(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.good())
    {
        std::cerr << "Unable to open " << filename << " for writing" << std::endl;
        return;
    }

    std::vector<char> data;
    send(data);

    file.write(data.data(), data.size());
}

static std::vector<char> ReadAllBytes(std::string const &filename)
{
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    std::vector<char> result(pos);

    ifs.seekg(0, std::ios::beg);
    ifs.read(result.data(), pos);

    return result;
}

SharedNetMessage NetMessage::readFromFile(const std::string &filename)
{
    if (!std::ifstream(filename).good())
    {
        std::cerr << "Unable to open " << filename << " for reading" << std::endl;
        return nullptr;
    }

    auto bytes = ReadAllBytes(filename);
    NetMessageReader r;
    r.pushBytes(bytes.data(), bytes.size());

    NetMessage *m = nullptr;
    if (!r.receiveMessage(m))
    {
        std::cerr << "No message found in " << filename << std::endl;
        return nullptr;
    }

    return SharedNetMessage(m);
}

void NetMessage::append(NetMessage *_msg)
{
    assert(mIsCompressed == _msg->mIsCompressed);
    reserve((int)(_msg->mData.size() + mData.size()));
    mData.insert(mData.end(), _msg->mData.begin(), _msg->mData.end());
}

void NetMessage::append(const UniqueNetMessage &_msg)
{
    append(_msg.get());
}

void NetMessage::append(const char *_begin, const char *_end)
{
    mData.insert(mData.end(), _begin, _end);
}

void NetMessage::reserve(int size)
{
    mData.reserve(size);
}

void NetMessage::writeDebugMarker(const std::string &marker)
{
    writeTypeInfo("Debug Marker", marker);
}

void NetMessage::verifyDebugMarker(const std::string &marker)
{
    verifyTypeInfo("Debug Marker", marker);
}

void NetMessage::clear()
{
    mPosition = 0;
    mData.clear();
    mGood = true;
    // mTypeInfo = std::strings;
    mOptions = 0;
}

void NetMessage::resetPosition()
{
    mPosition = 0;
}

void NetMessage::compress()
{
    if (mIsCompressed)
        return;
    if (mData.size() == 0)
        return;
    if (!(mOptions & NetMessageOptions::compressed))
        return;

    std::string compressedData;
    auto bytes_written = snappy::Compress(mData.data(), mData.size(), &compressedData);
    assert(bytes_written > 0);

    // UE_LOG(Info, Low, "Compression : " << mData.size() << " -> " << compressedData.size() << " (" <<
    // double(compressedData.size()*100)/mData.size() << "%)");

    mData.resize(bytes_written);
    std::copy(begin(compressedData), end(compressedData), begin(mData));

    mIsCompressed = true;
}

void NetMessage::decompress()
{
    if (!mIsCompressed)
        return;
    if (mData.size() == 0)
        return;
    if (!(mOptions & NetMessageOptions::compressed))
        return;


    std::string uncompressedData(snappy::MaxCompressedLength(mData.size()), 'x');
    auto success = snappy::Uncompress(mData.data(), mData.size(), &uncompressedData);
    if (!success)
        std::cerr << "Decompression of NetMessage was not successful" << std::endl;
    // UE_LOG(Info, Low, "Decompression: " << mData.size() << " -> " << uncompressedData.size() << " ("
    // << double(mData.size())/uncompressedData.size() << "%)");
    mData.resize(uncompressedData.size());
    std::copy(begin(uncompressedData), end(uncompressedData), begin(mData));

    mIsCompressed = false;
}

void NetMessage::writeIntPacked(int64_t value, const std::string &field)
{
    writeTypeInfo("Packed Int", field);

    internalWriteIntPacked(value);
}

int64_t NetMessage::readIntPacked(const std::string &field)
{
    verifyTypeInfo("Packed Int", field);

    return internalReadIntPacked();
}

void NetMessage::writeString(const std::string &value, const std::string &field)
{
    writeTypeInfo("string", field);

    unsigned int size = (unsigned)value.size();
    if (size > MaxStringSize)
    {
        if (mVerboseErrors)
            std::cerr << "The given std::string has size " << size << " which is too long (consider adjusting "
                                                                      "MaxStringSize)"
                      << std::endl;
        assert(0);
        return;
    }
    appendData((char *)&size, sizeof(size));
    if (size > 0)
        appendData((char *)value.c_str(), size);
}
std::string NetMessage::readString(const std::string &field)
{
    verifyTypeInfo("string", field);

    unsigned int size = 0;
    readData((char *)&size, sizeof(size));
    if (size > MaxStringSize)
    {
        if (mVerboseErrors)
            std::cerr << "Trying to read a std::string with size " << size << "which is quite too long" << std::endl;
        mGood = false;
        return "";
    }
    char *stype = new char[size + 1];
    if (size > 0)
        readData((char *)stype, size);
    stype[size] = '\0';

    std::string value(stype);
    delete[] stype;
    return value;
}
