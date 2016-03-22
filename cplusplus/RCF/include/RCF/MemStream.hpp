
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

#ifndef INCLUDE_RCF_MEMSTREAM_HPP
#define INCLUDE_RCF_MEMSTREAM_HPP

#include <istream>
#include <streambuf>

// std::size_t for vc6
#include <boost/cstdint.hpp> 

#include <boost/noncopyable.hpp>

#include <RCF/Config.hpp>
#include <RCF/ByteBuffer.hpp>
#include <RCF/ReallocBuffer.hpp>

namespace RCF {

    // MemIstreamBuf

    class MemIstreamBuf :
        public std::streambuf, 
        boost::noncopyable   
    {   
      public:   
        MemIstreamBuf(char * buffer = NULL, std::size_t bufferLen = 0);
        ~MemIstreamBuf();
        void reset(char * buffer, std::size_t bufferLen);
           
      private:   
        std::streambuf::int_type underflow();   

        pos_type seekoff(
            off_type off, 
            std::ios_base::seekdir dir,
            std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
           
        char * mBuffer;
        std::size_t mBufferLen; 
    };   

    // MemIstream - a replacement for std::istrstream.

    class RCF_EXPORT MemIstream : 
        public std::basic_istream<char>
    {   
    public:   
        MemIstream(const char * buffer = NULL, std::size_t bufferLen = 0);
        ~MemIstream();
        void reset(const char * buffer, std::size_t bufferLen);

    private:   

        MemIstreamBuf * mpBuf;

    public:

        std::istream::pos_type  getReadPos();
        bool                    fail();
        void                    get(char& ch);
        void                    read(char *_Str, std::streamsize _Count);
        std::streamsize         readsome(char *_Str, std::streamsize _Count);
        std::streamsize         gcount();
        void                    putback(char _Ch);
        std::istream::pos_type  moveReadPos(std::istream::pos_type newPos);

    };   

    RCF_EXPORT MemIstream & operator>>(MemIstream & is, std::string &       s);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, char &              ch);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, signed char &       ch);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, unsigned char &     ch);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, bool &              b);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, short &             b);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, unsigned short &    b);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, int &               n);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, unsigned int &      n);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, long &              n);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, unsigned long &     n);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, float &             d);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, double &            d);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, long double &       d);

#ifdef _MSC_VER
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, __int64 &    n);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, unsigned __int64 &  n);
#else
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, long long &    n);
    RCF_EXPORT MemIstream & operator>>(MemIstream & is, unsigned long long &    n);
#endif

    // MemOstreamBuf

    class MemOstreamBuf :
        public std::streambuf, 
        boost::noncopyable   
    {   
    public:   
        MemOstreamBuf();
        ~MemOstreamBuf();

    private:   
        std::streambuf::int_type overflow(std::streambuf::int_type ch);

        pos_type seekoff(
            off_type off, 
            std::ios_base::seekdir dir,
            std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

        friend class MemOstream;
        ReallocBuffer mWriteBuffer;
    };   

    // MemOstream - a replacement for std::ostrstream.

    class RCF_EXPORT MemOstream : 
        public std::basic_ostream<char>
    {   
    public:   
        MemOstream();
        ~MemOstream();

        char * str();
        std::size_t length();
        std::string string();

        std::size_t capacity();

        void rewind()
        {
            rdbuf()->pubseekoff(0, std::ios::beg, std::ios::out);
        }

        
    private:   

        MemOstreamBuf * mpBuf;

    public:

        /*
        // std:ostream interface.

        std::ostream::pos_type tellp()
        {
            return std::ostream::tellp();
        }

        void write(const char * _Str, std::streamsize _Count)
        {
            std::ostream::write(_Str, _Count);
        }

        bool fail()
        {
            return std::ostream::fail();
        }

        void clear()
        {
            std::ostream::clear();
        }
        */

    };   

    typedef boost::shared_ptr<MemOstream> MemOstreamPtr;

    // iostream impl

    template<typename T>
    inline MemOstream & operator<<(MemOstream & os, const T * pt)
    {
        static_cast<std::ostream&>(os) << pt;
        return os;
    }

    template<typename T>
    inline MemOstream & operator<<(MemOstream & os, const boost::shared_ptr<T> & pt)
    {
        static_cast<std::ostream&>(os) << pt.get();
        return os;
    }

    RCF_EXPORT MemOstream & operator<<(MemOstream & os, const std::string & s);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, const char * sz);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, char ch);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, signed char ch);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, unsigned char ch);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, bool b);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, short b);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, unsigned short b);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, int n);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, unsigned int n);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, long n);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, unsigned long n);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, float d);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, double d);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, long double d);

#ifdef _MSC_VER
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, __int64 n);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, unsigned __int64 n);
#else
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, long long n);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, unsigned long long n);
#endif

    typedef std::ostream& (*Pfn)(std::ostream&);
    RCF_EXPORT MemOstream & operator<<(MemOstream & os, Pfn pfn);

} // namespace RCF

#endif
