#pragma once

#include <glow/common/shared.hh>
#include <glow/common/warn_unused.hh>

#include "Buffer.hh"

#include <map>
#include <string>
#include <vector>

namespace glow
{
GLOW_SHARED(class, UniformBuffer);
/**
 * A uniform buffer for generic data
 *
 * Usage:
 *   program->setUniformBuffer("bufferName", buffer);
 *
 * See std140.hh for ways to safely use C++ structs
 *
 * addVerification(...) can be used to add offset-name pairs to be verified *
 * Usage:
 *   buffer->addVerification(offsetAsInt, "nameInBuffer");
 *   buffer->addVerification(&MyStruct::varName, "nameInBuffer");
 *   buffer->addVerification({{&MyStruct::varName, "nameInBuffer"}, ... });
 * Caution:
 *   * will only check when bound to particular shader
 *   * may not work if variable is not used in shader
 */
class UniformBuffer : public Buffer
{
public:
    struct BoundUniformBuffer;

private:
    /// Offset to verify
    std::map<std::string, int> mVerificationOffsets;

public: // getter
    /// Gets the currently bound UniformBuffer (nullptr if none)
    static BoundUniformBuffer* getCurrentBuffer();

    std::map<std::string, int> const& getVerificationOffsets() const { return mVerificationOffsets; }

public:
    /// RAII-object that defines a "bind"-scope for an UniformBuffer
    /// All functions that operate on the currently bound buffer are accessed here
    struct BoundUniformBuffer
    {
        GLOW_RAII_CLASS(BoundUniformBuffer);

        /// Backreference to the buffer
        UniformBuffer* const buffer;

        /// Sets the data of this uniform buffer (generic version)
        void setData(size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);
        /// Sets the data of this uniform buffer (vector-of-data version)
        template <typename DataT>
        void setData(std::vector<DataT> const& data, GLenum usage = GL_STATIC_DRAW)
        {
            setData(data.size() * sizeof(DataT), data.data(), usage);
        }
        /// Sets the data of this uniform buffer (POD version)
        template <typename DataT>
        void setData(DataT const& data, GLenum usage = GL_STATIC_DRAW)
        {
            setData(sizeof(data), &data, usage);
        }

    private:
        GLint previousBuffer;                  ///< previously bound buffer
        BoundUniformBuffer* previousBufferPtr; ///< previously bound buffer
        BoundUniformBuffer(UniformBuffer* buffer);
        friend class UniformBuffer;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundUniformBuffer(BoundUniformBuffer&&); // allow move
        ~BoundUniformBuffer();
    };

public:
    UniformBuffer();

    /// Adds a verification target to shader
    /// Emits warnings/erorrs when expected offset does not match reported one (when used in shader)
    void addVerification(int offset, std::string const& nameInShader);
    template <typename StructT, typename DataT>
    void addVerification(DataT StructT::*member, std::string const& nameInShader)
    {
        addVerification(reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<StructT const volatile*>(0)->*member)), nameInShader);
    }
    struct VerificationPair
    {
        int offset;
        std::string name;

        VerificationPair() = default;
        template <typename StructT, typename DataT>
        VerificationPair(DataT StructT::*member, std::string const& nameInShader)
        {
            offset = reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<StructT const volatile*>(0)->*member));
            name = nameInShader;
        }
    };
    void addVerification(std::vector<VerificationPair> const& members)
    {
        for (auto const& p : members)
            addVerification(p.offset, p.name);
    }

    /// Binds this uniform buffer.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundUniformBuffer bind() { return {this}; }

public: // static construction
    /// Creates an empty array buffer
    /// Same as std::make_shared<UniformBuffer>();
    static SharedUniformBuffer create();
};
}
