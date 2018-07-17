#pragma once

#include <vector>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <cstring>
#include <iostream>
#include <typeinfo>

#include "shared.hh"
#include "property.hh"

#include "NetMessageFlags.hh"

namespace aion
{
#ifdef NETMESSAGE_VALUE_TYPE
#error "NETMESSAGE_VALUE_TYPE is defined, but it shouldn't be"
#endif
#ifdef NETMESSAGE_VECTOR_OF_VALUE_TYPE
#error "NETMESSAGE_VECTOR_OF_VALUE_TYPE is defined, but it shouldn't be"
#endif
#ifdef NETMESSAGE_ARRAY_OF_VALUE_TYPE
#error "NETMESSAGE_ARRAY_OF_VALUE_TYPE is defined, but it shouldn't be"
#endif
#ifdef NETMESSAGE_VALUE_TYPE_FULL
#error "NETMESSAGE_VALUE_TYPE_FULL is defined, but it shouldn't be"
#endif

#define NETMESSAGE_ARRAY_OF_VALUE_TYPE(type, typeName)                                              \
    void write##typeName##Array(type const *value, std::size_t size, const std::string &_field)     \
    {                                                                                               \
        assert(value == nullptr || size > 0);                                                       \
        if (size > MaxArraySize)                                                                    \
        {                                                                                           \
            if (mVerboseErrors)                                                                     \
                std::cerr << "The given array has size " << size                                    \
                          << " which is too long (consider adjusting MaxArraySize)" << std::endl;   \
            assert(0);                                                                              \
            return;                                                                                 \
        }                                                                                           \
        writeTypeInfo(#type "[]", _field);                                                          \
        if (value == nullptr)                                                                       \
            size = 0;                                                                               \
        appendData((char *) & size, sizeof(size));                                                  \
        if (size > 0)                                                                               \
            appendData((char *) & (value[0]), size * sizeof(type));                                 \
    }                                                                                               \
    type *read##typeName##Array(const std::string &_field)                                          \
    {                                                                                               \
        verifyTypeInfo(#type "[]", _field);                                                         \
        std::size_t size = 0;                                                                       \
        readData((char *) & size, sizeof(size));                                                    \
        if (size == 0)                                                                              \
            return nullptr;                                                                         \
        if (size > MaxArraySize)                                                                    \
        {                                                                                           \
            if (mVerboseErrors)                                                                     \
                std::cerr << "Trying to read an array with size " << size << "which is quite "      \
                                                                             "too long"             \
                          << std::endl;                                                             \
            mGood = false;                                                                          \
            return nullptr;                                                                         \
        }                                                                                           \
        type *value = new type[size];                                                               \
        readData((char *)value, size * sizeof(type));                                               \
        return value;                                                                               \
    }                                                                                               \
    std::size_t read##typeName##Array(type *value, std::size_t maxcount, const std::string &_field) \
    {                                                                                               \
        verifyTypeInfo(#type "[]", _field);                                                         \
        std::size_t size = 0;                                                                       \
        readData((char *) & size, sizeof(size));                                                    \
        if (size > MaxArraySize)                                                                    \
        {                                                                                           \
            if (mVerboseErrors)                                                                     \
                std::cerr << "Trying to read an array with size " << size << "which is quite "      \
                                                                             "too long"             \
                          << std::endl;                                                             \
            mGood = false;                                                                          \
            return -1;                                                                              \
        }                                                                                           \
        if (size == 0)                                                                              \
            return 0;                                                                               \
        std::size_t readsize = size > maxcount ? maxcount : size;                                   \
        readData((char *)value, readsize * sizeof(type));                                           \
        if (size > readsize)                                                                        \
            mPosition += (size - readsize) * sizeof(type);                                          \
        return readsize;                                                                            \
    }

#define NETMESSAGE_VECTOR_OF_VALUE_TYPE(type, typeName)                                             \
    void write##typeName##Vector(const std::vector<type> &value, const std::string &_field)         \
    {                                                                                               \
        writeTypeInfo("vector<" #type ">", _field);                                                 \
        std::size_t size = value.size();                                                            \
        if (size > MaxArraySize)                                                                    \
        {                                                                                           \
            if (mVerboseErrors)                                                                     \
                std::cerr << "The given std::vector has size " << size                              \
                          << " which is too long (consider adjusting MaxArraySize)" << std::endl;   \
            assert(0);                                                                              \
            return;                                                                                 \
        }                                                                                           \
        appendData((char *) & size, sizeof(size));                                                  \
        if (size > 0)                                                                               \
            appendData((char *) & (value[0]), size * sizeof(type));                                 \
    }                                                                                               \
    std::vector<type> read##typeName##Vector(const std::string &_field)                             \
    {                                                                                               \
        verifyTypeInfo("vector<" #type ">", _field);                                                \
        std::size_t size = 0;                                                                       \
        readData((char *) & size, sizeof(size));                                                    \
        std::vector<type> value;                                                                    \
        if (size > MaxArraySize)                                                                    \
        {                                                                                           \
            if (mVerboseErrors)                                                                     \
                std::cerr << "Trying to read a std::vector with size " << size << "which is quite " \
                                                                                  "too long"        \
                          << std::endl;                                                             \
            mGood = false;                                                                          \
            return value;                                                                           \
        }                                                                                           \
        if (size > 0)                                                                               \
        {                                                                                           \
            value.resize(size);                                                                     \
            readData((char *) & value[0], size * sizeof(type));                                     \
        }                                                                                           \
        return value;                                                                               \
    }                                                                                               \
    void read##typeName##Vector(std::vector<type> &value, const std::string &_field)                \
    {                                                                                               \
        verifyTypeInfo("vector<" #type ">", _field);                                                \
        std::size_t size;                                                                           \
        readData((char *) & size, sizeof(size));                                                    \
        if (size > MaxArraySize)                                                                    \
        {                                                                                           \
            if (mVerboseErrors)                                                                     \
                std::cerr << "Trying to read a std::vector with size " << size << "which is quite " \
                                                                                  "too long"        \
                          << std::endl;                                                             \
            mGood = false;                                                                          \
            return;                                                                                 \
        }                                                                                           \
        if (size > 0)                                                                               \
        {                                                                                           \
            value.resize(size);                                                                     \
            readData((char *) & (value[0]), size * sizeof(type));                                   \
        }                                                                                           \
        else                                                                                        \
            value.clear();                                                                          \
    }

#define NETMESSAGE_VALUE_TYPE(type, typeName)                                                           \
    void write##typeName(const type &value, std::string _field)                                         \
    {                                                                                                   \
        writeTypeInfo(#type, _field);                                                                   \
        appendData<type>(value);                                                                        \
    }                                                                                                   \
    size_t writeDummy##typeName(std::string _field)                                                     \
    {                                                                                                   \
        writeTypeInfo(#type, _field);                                                                   \
        type value;                                                                                     \
        memset(&value, 0xFF, sizeof(type));                                                             \
        size_t pos = mData.size();                                                                      \
        appendData<type>(value);                                                                        \
        return pos;                                                                                     \
    }                                                                                                   \
    void setDummy##typeName(const type &value, size_t pos) { *((type *)(mData.data() + pos)) = value; } \
    type read##typeName(const std::string &_field)                                                      \
    {                                                                                                   \
        verifyTypeInfo(#type, _field);                                                                  \
        type value = (type)0;                                                                           \
        readData((char *) & value, sizeof(type));                                                       \
        return value;                                                                                   \
    }

#define NETMESSAGE_VALUE_TYPE_FULL(type, typeName) \
    NETMESSAGE_VALUE_TYPE(type, typeName)          \
    NETMESSAGE_ARRAY_OF_VALUE_TYPE(type, typeName) \
    NETMESSAGE_VECTOR_OF_VALUE_TYPE(type, typeName)

AION_SHARED(class, NetMessage);
/**
 * A simple container for a network message
 * May contain type data
 */
class NetMessage
{
public:
    /// Amount of type information sent
    enum class TypeInformation
    {
        None = 0x0,   // only 2 bit overhead per message
        Hashes = 0x1, // 32 bit overhead per entry
        Strings = 0x2 // entire type and field std::string per entry
    };

    /// Maximum size of a std::string
    static const unsigned int MaxStringSize = 256 * 1024;
    /// Maximum size of an array
    static const unsigned int MaxArraySize = 100 * 1024 * 1024;

    ~NetMessage() {}

    NetMessage(NetMessageOptions::Type options = 0, TypeInformation typeInfo = TypeInformation::Strings);

private:
    /// The pure data
    std::vector<char> mData;
    /// The current position in the stream
    std::size_t mPosition;
    /// Is the netmessage still valid (as-in types still match)
    bool mGood;
    /// True, if verbose errors should be displayed
    bool mVerboseErrors;
    /// Amount of type info sent
    TypeInformation mTypeInfo;
    /// The first byte of the message
    NetMessageOptions::Type mOptions;
    /// has the compress function already been called?
    bool mIsCompressed;


private:
    NetMessage(const NetMessage &);
    NetMessage &operator=(const NetMessage &);

    /// Appends one byte
    void appendByte(char _c);

    /// Appends some byte data to the data
    void appendData(const char *data, std::size_t count);

    /// Appends one data type stuff
    template <typename T>
    void appendData(T _value)
    {
        mData.resize(mData.size() + sizeof(T));

        // T *dataptr = (T *) ((&mData[mData.size() - 1]) - (sizeof(T) - 1));
        T *dataptr = (T *)(mData.data() + (mData.size() - sizeof(T)));
        *dataptr = _value;
    }

    /// Reads a certain amount of data and writes it to "data"
    /// Advances mPosition by count bytes
    void readData(char *data, std::size_t count);

    /// Writes (optional) type information about type and field
    void writeTypeInfo(const std::string &type, const std::string &field);

    /// Verifies type information about type and field
    void verifyTypeInfo(const std::string &type, const std::string &field);

    /// writes a packed int without type info
    void internalWriteIntPacked(int64_t value);

    /// reads a packed int without type info
    int64_t internalReadIntPacked();


public:
    void compress();

    void decompress();

    /// Creates a copy of this message
    NetMessage *clone() const;

    /// Sets the complete data of the msg, also resets position
    void setData(std::vector<char> const &_data, bool _compressed);

    /// Sets the complete data of the msg, also resets position
    void setData(const char *_data, size_t _dataSize, bool _compressed);

    /// Creates a copy of this message
    UniqueNetMessage cloneUnique() const;
    // TODO: copy ctor, assignment op, create from data

    /// Writes a header and the content of this message to a stream
    void send(std::vector<char> &output);

    void writeToFile(std::string const& filename);
    static SharedNetMessage readFromFile(std::string const& filename);

    /// Append another netMessage
    void append(NetMessage *_msg);

    /// Append another netMessage
    void append(const UniqueNetMessage &_msg);

    /// Append rawData
    /// CAUTION: Does not adapt the stored size! Do NOT use if you don't know what you are doing!
    void append(const char *_begin, const char *_end);

    /// Manipulates the internal position status
    /// CAUTION: Do NOT use if you don't know what you are doing!
    void setPosition(const std::size_t &_pos) { mPosition = _pos; }
    /// Returns the size of the data that has not been read out yet.
    int getSizeOfNonReadData() const { return (int)mData.size() - (int)mPosition; }
    /// Returns the size of the data
    int getSize() const { return (int)mData.size(); }
    /// Reserves a certain amount of bytes in the msg
    /// Does not directly change data, but may improve performance
    void reserve(int size);

    /// Returns the options byte
    NetMessageOptions::Type options() const { return mOptions; }
    /// Checks if the NetMessage is still good and warm
    bool good() const { return mGood; }
    /// Checks if the NetMessage is at the end
    bool isEOM() const { return mPosition >= mData.size(); }
    /// Enable verbose error output
    void enableVerboseErrors() { mVerboseErrors = true; }
    /// Disable verbose error output
    void disableVerboseErrors() { mVerboseErrors = false; }
    /// Places a debug marker in the NetMessage
    void writeDebugMarker(const std::string &marker);

    /// Verifies a debug marker in the NetMessage
    void verifyDebugMarker(const std::string &marker);

    /// Clears position and content of this msg
    void clear();

    /// Resets the position to the beginning
    void resetPosition();

    /// Const reference to the internal data
    const std::vector<char> &getData() const { return mData; }
    AION_GETTER(Position);

    // Supported types:

    // value types
    NETMESSAGE_VALUE_TYPE(bool, Bool)
    NETMESSAGE_ARRAY_OF_VALUE_TYPE(bool, Bool)
    //     std::vector<bool> is evil, you should not use it!

    NETMESSAGE_VALUE_TYPE_FULL(char, Char)
    NETMESSAGE_VALUE_TYPE_FULL(int8_t, Int8)
    NETMESSAGE_VALUE_TYPE_FULL(int16_t, Int16)
    NETMESSAGE_VALUE_TYPE_FULL(int32_t, Int32)
    NETMESSAGE_VALUE_TYPE_FULL(int64_t, Int64)

    NETMESSAGE_VALUE_TYPE_FULL(uint8_t, UInt8)
    NETMESSAGE_VALUE_TYPE_FULL(uint16_t, UInt16)
    NETMESSAGE_VALUE_TYPE_FULL(uint32_t, UInt32)
    NETMESSAGE_VALUE_TYPE_FULL(uint64_t, UInt64)

    NETMESSAGE_VALUE_TYPE_FULL(float, Float)
    NETMESSAGE_VALUE_TYPE_FULL(double, Double)


    void writeIntPacked(int64_t value, const std::string &field);
    int64_t readIntPacked(const std::string &field);

    void writeString(const char *value, const std::string &field) { writeString(std::string(value), field); }
    void writeString(const std::string &value, const std::string &field);
    std::string readString(const std::string &field);

    template <typename T>
    void writeStruct(T &value, const std::string &field)
    {
        writeTypeInfo(std::string("struct ") + typeid(T).name(), field);
        appendData((char *)&value, sizeof(T));
    }

    template <typename T>
    T readStruct(const std::string &field)
    {
        verifyTypeInfo(std::string("struct ") + typeid(T).name(), field);
        T value;
        readData((char *)&value, sizeof(T));
        return value;
    }

    template <typename T>
    void readStruct(T &value, const std::string &field)
    {
        verifyTypeInfo(std::string("struct ") + typeid(T).name(), field);
        readData((char *)&value, sizeof(T));
    }

    friend class NetMessageReader;
};
}
