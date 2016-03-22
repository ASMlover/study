
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2013, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 2.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

#include <RCF/MemStream.hpp>

#include <RCF/Tools.hpp>

namespace RCF {

    // MemIstreamBuf implementation

    MemIstreamBuf::MemIstreamBuf(   
        char * buffer, 
        std::size_t bufferLen) : 
            mBuffer(buffer),
            mBufferLen(bufferLen)
    {   
        setg(mBuffer, mBuffer, mBuffer + mBufferLen);
    }   

    MemIstreamBuf::~MemIstreamBuf()
    {
    }

    void MemIstreamBuf::reset(char * buffer, std::size_t bufferLen)
    {
        mBuffer = buffer;
        mBufferLen = bufferLen;
        setg(mBuffer, mBuffer, mBuffer + mBufferLen);
    }

    std::streambuf::int_type MemIstreamBuf::underflow()   
    {   
        if (gptr() < egptr())
        {
            return traits_type::to_int_type(*gptr());
        }

        return traits_type::eof();   
    }

    MemIstreamBuf::pos_type MemIstreamBuf::seekoff(
        MemIstreamBuf::off_type offset, 
        std::ios_base::seekdir dir,
        std::ios_base::openmode mode)
    {
        RCF_UNUSED_VARIABLE(mode);

        char * pBegin = mBuffer;
        char * pEnd = mBuffer + mBufferLen;
        
        char * pBase = NULL;
        switch(dir)
        {
            case std::ios::cur: pBase = gptr(); break;
            case std::ios::beg: pBase = pBegin; break;
            case std::ios::end: pBase = pEnd; break;
            default: assert(0); break; 
        }

        char * pNewPos = pBase + offset;
        if (pBegin <= pNewPos && pNewPos <= pEnd)
        {
            setg(pBegin, pNewPos, pEnd);
            return pNewPos - pBegin;
        }
        else
        {
            return pos_type(-1);
        }
    }

    // MemOstreamBuf implementation

    MemOstreamBuf::MemOstreamBuf()
    {   
        mWriteBuffer.resize(10);

        setp( 
            &mWriteBuffer[0], 
            &mWriteBuffer[0] + mWriteBuffer.size());
    }

    MemOstreamBuf::~MemOstreamBuf()
    {
    }

    std::streambuf::int_type MemOstreamBuf::overflow(std::streambuf::int_type ch)
    {
        if (ch == traits_type::eof())
        {
            return traits_type::eof();
        }

        mWriteBuffer.resize( 2*mWriteBuffer.size() );
        
        std::size_t nextPos = pptr() - pbase();

        setp( 
            &mWriteBuffer[0],
            &mWriteBuffer[0] + mWriteBuffer.size());

        pbump( static_cast<int>(nextPos) );

        *pptr() = static_cast<char>(ch);
        pbump(1);
        return ch;
    }

    MemOstreamBuf::pos_type MemOstreamBuf::seekoff(
        MemOstreamBuf::off_type offset, 
        std::ios_base::seekdir dir,
        std::ios_base::openmode mode)
    {
        RCF_UNUSED_VARIABLE(mode);

        char * pBegin = pbase();
        char * pEnd = epptr();
        
        char * pBase = NULL;
        switch(dir)
        {
            case std::ios::cur: pBase = pptr(); break;
            case std::ios::beg: pBase = pBegin; break;
            case std::ios::end: pBase = pEnd; break;
            default: assert(0); break; 
        }

        char * pNewPos = pBase + offset;
        if (pBegin <= pNewPos && pNewPos <= pEnd)
        {
            setp(pBegin, pEnd);
            pbump( static_cast<int>(pNewPos - pBegin) );
            return pNewPos - pBegin;
        }
        else
        {
            return pos_type(-1);
        }
    }

    // MemIstream
    MemIstream::MemIstream(const char * buffer, std::size_t bufferLen) :
        std::basic_istream<char>(new MemIstreamBuf(const_cast<char *>(buffer), bufferLen))
    {   
        mpBuf = static_cast<MemIstreamBuf *>(rdbuf());
    }

    MemIstream::~MemIstream()
    {
        delete mpBuf;
    }

    void MemIstream::reset(const char * buffer, std::size_t bufferLen)
    {
        clear();
        mpBuf->reset(const_cast<char *>(buffer), bufferLen);
    }

    std::istream::pos_type MemIstream::getReadPos()
    {
        return tellg();
    }

    bool MemIstream::fail()
    {
        return std::istream::fail();
    }

    void MemIstream::get(char& ch)
    {
        std::istream::get(ch);
    }

    void MemIstream::read(char *_Str, std::streamsize _Count)
    {
        std::istream::read(_Str, _Count);
    }

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996) // warning C4996: 'std::basic_istream<_Elem,_Traits>::readsome': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'
#endif

    std::streamsize MemIstream::readsome(char *_Str, std::streamsize _Count)
    {
        return std::istream::readsome(_Str, _Count);
    }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    std::streamsize MemIstream::gcount()
    {
        return std::istream::gcount();
    }

    void MemIstream::putback(char _Ch)
    {
        std::istream::putback(_Ch);
    }

    std::istream::pos_type MemIstream::moveReadPos(std::istream::pos_type newPos)
    {
        //return std::istream::seekg(
        //  static_cast<std::istream::off_type>(newPos), 
        //  std::ios_base::beg);

        //return std::istream::seekg( newPos );

        //return seekg( newPos );

        seekg(
            static_cast<std::istream::off_type>(newPos), 
            std::ios_base::beg);

        return 0;

    }



    // iostream impl

    MemIstream & operator>>(MemIstream & is, std::string & s)
    {
        static_cast<std::istream&>(is) >> s;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, char & ch)
    {
        static_cast<std::istream&>(is) >> ch;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, signed char & ch)
    {
        static_cast<std::istream&>(is) >> ch;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, unsigned char & ch)
    {
        static_cast<std::istream&>(is) >> ch;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, bool & b)
    {
        static_cast<std::istream&>(is) >> b;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, short & b)
    {
        static_cast<std::istream&>(is) >> b;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, unsigned short & b)
    {
        static_cast<std::istream&>(is) >> b;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, int & n)
    {
        static_cast<std::istream&>(is) >> n;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, unsigned int & n)
    {
        static_cast<std::istream&>(is) >> n;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, long & n)
    {
        static_cast<std::istream&>(is) >> n;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, unsigned long & n)
    {
        static_cast<std::istream&>(is) >> n;
        return is;
    }

#ifdef _MSC_VER

    MemIstream & operator>>(MemIstream & is, __int64 & n)
    {
        static_cast<std::istream&>(is) >> n;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, unsigned __int64 & n)
    {
        static_cast<std::istream&>(is) >> n;
        return is;
    }

#else

    MemIstream & operator>>(MemIstream & is, long long & n)
    {
        static_cast<std::istream&>(is) >> n;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, unsigned long long & n)
    {
        static_cast<std::istream&>(is) >> n;
        return is;
    }

#endif


    MemIstream & operator>>(MemIstream & is, float & d)
    {
        static_cast<std::istream&>(is) >> d;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, double & d)
    {
        static_cast<std::istream&>(is) >> d;
        return is;
    }

    MemIstream & operator>>(MemIstream & is, long double & d)
    {
        static_cast<std::istream&>(is) >> d;
        return is;
    }


    // MemOstream
    MemOstream::MemOstream() :
        std::basic_ostream<char>(new MemOstreamBuf())
    {   
        mpBuf = static_cast<MemOstreamBuf *>(rdbuf());
    }

    MemOstream::~MemOstream()
    {
        delete mpBuf;
    }

    char * MemOstream::str()
    {
        return & mpBuf->mWriteBuffer[0];
    }

    std::size_t MemOstream::length()
    {
        return static_cast<std::size_t>(tellp());
    }

    std::string MemOstream::string()
    {
        return std::string(str(), length());
    }

    std::size_t MemOstream::capacity()
    {
        return mpBuf->mWriteBuffer.capacity();
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4995) // 'sprintf': name was marked as #pragma deprecated
#pragma warning(disable: 4996) // 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#endif

    // iostream impl

    MemOstream & operator<<(MemOstream & os, const std::string & s)
    {
        static_cast<std::ostream&>(os) << s.c_str();
        return os;
    }

    MemOstream & operator<<(MemOstream & os, const char * sz)
    {
        static_cast<std::ostream&>(os) << sz;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, char ch)
    {
        static_cast<std::ostream&>(os) << ch;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, signed char ch)
    {
        static_cast<std::ostream&>(os) << ch;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, unsigned char ch)
    {
        static_cast<std::ostream&>(os) << ch;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, bool b)
    {
        char buffer[1024];
        sprintf(buffer, "%d", (int) b);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, short b)
    {
        char buffer[1024];
        sprintf(buffer, "%d", (int) b);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, unsigned short b)
    {
        char buffer[1024];
        sprintf(buffer, "%d", (int) b);
        static_cast<std::ostream&>(os) << buffer;

        return os;
    }

    MemOstream & operator<<(MemOstream & os, int n)
    {
        char buffer[1024];
        sprintf(buffer, "%d", n);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, unsigned int n)
    {
        char buffer[1024];
        sprintf(buffer, "%u", n);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, long n)
    {
        char buffer[1024];
        sprintf(buffer, "%d", (int) n);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, unsigned long n)
    {
        char buffer[1024];
        sprintf(buffer, "%u", (unsigned int) n);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

#ifdef _MSC_VER

    MemOstream & operator<<(MemOstream & os, __int64 n)
    {
        char buffer[1024];
        sprintf(buffer, "%I64d", n);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, unsigned __int64 n)
    {
        char buffer[1024];
        sprintf(buffer, "%I64u", n);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

#else

    MemOstream & operator<<(MemOstream & os, long long n)
    {
        char buffer[1024];
        sprintf(buffer, "%lld", n);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, unsigned long long n)
    {
        char buffer[1024];
        sprintf(buffer, "%llu", n);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

#endif

    MemOstream & operator<<(MemOstream & os, float d)
    {
        char buffer[1024];
        sprintf(buffer, "%f", (double) d);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, double d)
    {
        char buffer[1024];
        sprintf(buffer, "%f", d);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    MemOstream & operator<<(MemOstream & os, long double d)
    {
        char buffer[1024];
        sprintf(buffer, "%f", (double) d);
        static_cast<std::ostream&>(os) << buffer;
        return os;
    }

    typedef std::ostream& (*Pfn)(std::ostream&);
    MemOstream & operator<<(MemOstream & os, Pfn pfn)
    {
        static_cast<std::ostream&>(os) << pfn;
        return os;
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace RCF
