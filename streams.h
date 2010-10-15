/******************************************************************************
 * This file is part of dirtsand.                                             *
 *                                                                            *
 * dirtsand is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * dirtsand is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with dirtsand.  If not, see <http://www.gnu.org/licenses/>.          *
 ******************************************************************************/

#ifndef _DS_STREAMS_H
#define _DS_STREAMS_H

#include "strings.h"
#include <exception>
#include <cstdio>

namespace DS
{
    class EofException : public std::exception
    {
    public:
        virtual const char* what() const throw()
        { return "[EofException] Unexpected end of stream"; }
    };

    class FileIOException : public std::exception
    {
    public:
        FileIOException(const char* msg) : m_errmsg(msg) { }
        virtual ~FileIOException() throw() { }

        virtual const char* what() const throw()
        { return (DS::String("[FileIOException] ") + m_errmsg).c_str(); }

    private:
        DS::String m_errmsg;
    };


    class Stream
    {
    public:
        Stream() { }
        virtual ~Stream() { }

        virtual ssize_t readBytes(void* buffer, size_t count) = 0;
        virtual ssize_t writeBytes(const void* buffer, size_t count) = 0;

        template <typename tp> tp read()
        {
            tp value;
            if (readBytes(&value, sizeof(value)) != sizeof(value))
                throw EofException();
            return value;
        }

        String readString(size_t length, DS::StringType format = e_StringRAW8);
        String readSafeString(DS::StringType format = e_StringRAW8);
        bool readBool() { return read<uint8_t>() != 0; }

        template <typename tp> void write(tp value)
        { writeBytes(&value, sizeof(value)); }

        void writeString(const String& value, DS::StringType format = e_StringRAW8);
        void writeSafeString(const String& value, DS::StringType format = e_StringRAW8);
        void writeBool(bool value) { write<uint8_t>(value ? 1 : 0); }

        virtual uint32_t tell() = 0;
        virtual void seek(sint32_t offset, int whence) = 0;
        virtual uint32_t size() = 0;
        virtual bool atEof() = 0;
        virtual void flush() = 0;
    };

    /* Stream for reading/writing regular files */
    class FileStream : public Stream
    {
    public:
        FileStream() : m_file(0) { }
        virtual ~FileStream() { close(); }

        void open(const char* filename, const char* mode);
        void close()
        {
            if (m_file)
                fclose(m_file);
            m_file = 0;
        }

        virtual ssize_t readBytes(void* buffer, size_t count)
        { return fread(buffer, 1, count, m_file); }

        virtual ssize_t writeBytes(const void* buffer, size_t count)
        { return fwrite(buffer, 1, count, m_file); }

        virtual uint32_t tell() { return static_cast<uint32_t>(ftell(m_file)); }
        virtual void seek(sint32_t offset, int whence) { fseek(m_file, offset, whence); }
        virtual uint32_t size();
        virtual bool atEof();
        virtual void flush() { fflush(m_file); }

    private:
        FILE* m_file;
    };

    /* Read/Write RAM stream */
    class BufferStream : public Stream
    {
    public:
        BufferStream() : m_buffer(0), m_position(0), m_size(0), m_alloc(0) { }
        BufferStream(const void* data, size_t size);
        virtual ~BufferStream() { delete[] m_buffer; }

        virtual ssize_t readBytes(void* buffer, size_t count);
        virtual ssize_t writeBytes(const void* buffer, size_t count);

        virtual uint32_t tell() { return static_cast<uint32_t>(m_position); }
        virtual void seek(sint32_t offset, int whence);
        virtual uint32_t size() { return m_size; }
        virtual bool atEof() { return m_position >= m_size; }
        virtual void flush() { }

        void truncate()
        {
            m_size = 0;
            m_position = 0;
        }

        const uint8_t* buffer() const { return m_buffer; }

    private:
        uint8_t* m_buffer;
        size_t m_position;
        size_t m_size, m_alloc;
    };

    /* Read-only ref-counted RAM stream */
    class Blob
    {
    public:
        Blob(const uint8_t* buffer, size_t size)
            : m_buffer(buffer), m_size(size), m_refs(1) { }

        const uint8_t* buffer() const { return m_buffer; }
        size_t size() const { return m_size; }

        void ref() { ++m_refs; }
        void unref()
        {
            if (--m_refs == 0)
                delete this;
        }

    private:
        const uint8_t* m_buffer;
        size_t m_size;
        int m_refs;

        Blob(const Blob&) { }
        ~Blob() { delete[] m_buffer; }
    };

    class BlobStream : public Stream
    {
    public:
        BlobStream(Blob* blob) : m_blob(blob) { m_blob->ref(); }
        virtual ~BlobStream() { m_blob->unref(); }

        virtual ssize_t readBytes(void* buffer, size_t count);
        virtual ssize_t writeBytes(const void* buffer, size_t count);

        virtual uint32_t tell() { return static_cast<uint32_t>(m_position); }
        virtual void seek(sint32_t offset, int whence);
        virtual uint32_t size() { return static_cast<uint32_t>(m_blob->size()); }
        virtual bool atEof() { return tell() >= size(); }
        virtual void flush() { }

    private:
        Blob* m_blob;
        size_t m_position;
    };
}

#endif