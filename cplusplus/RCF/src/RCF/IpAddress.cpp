
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

#include <RCF/IpAddress.hpp>

#include <RCF/ByteOrdering.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Tools.hpp>

#include <sstream>

#ifdef BOOST_WINDOWS
#include <WS2tcpip.h> // getaddrinfo()
#endif

namespace RCF {

#ifdef BOOST_WINDOWS

    // Windows only has inet_pton on later versions, so use WSAStringToAddress() instead.
    bool resolveNumericIp(const std::string & ip, int addrFamily)
    {
        SockAddrStorage storage;
        int storageLen = sizeof(storage);

        INT ret = WSAStringToAddressA(
            (LPSTR) ip.c_str(), 
            addrFamily, 
            NULL, 
            (LPSOCKADDR) &storage, 
            &storageLen);

        if (ret == 0)
        {
            return true;
        }

        return false;
    }

#else

    bool resolveNumericIp(const std::string & ip, int addrFamily)
    {
        SockAddrStorage storage;

        int rc = inet_pton(addrFamily, ip.c_str(), &storage);

        if (rc == 1)
        {
            return true;
        }

        return false;
    }

#endif

    IpAddress::IpAddress() :
        mType(V4_or_V6),
        mResolved(false),
        mIp(),
        mPort(0)
    {
        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));
    }

    IpAddress::IpAddress(Type restrictTo) :
        mType(restrictTo),
        mResolved(false),
        mIp(),
        mPort(0)
    {
        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));
    }

    IpAddress::IpAddress(const std::string & ip) :
        mType(V4_or_V6),
        mResolved(false),
        mIp(ip),
        mPort(0)
    {
        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));
    }

    IpAddress::IpAddress(const std::string & ip, int port) :
        mType(V4_or_V6),
        mResolved(false),
        mIp(ip),
        mPort(port)
    {
        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));
    }

    IpAddress::IpAddress(const std::string & ip, int port, Type restrictTo) :
        mType(restrictTo),
        mResolved(false),
        mIp(ip),
        mPort(port)
    {
        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));
    }

    IpAddress::IpAddress(const sockaddr_in &addr) :
        mType(V4),
        mResolved(true),
        mIp(),
        mPort()
    {
        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));
        memcpy(&mAddrV4, &addr, sizeof(mAddrV4));

        extractIpAndPort();
    }

    IpAddress::IpAddress(const SockAddrIn6 &addr) :
        mType(V6),
        mResolved(true),
        mIp(),
        mPort()
    {
        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));
        memcpy(&mAddrV6, &addr, sizeof(mAddrV6));

        extractIpAndPort();
    }

    IpAddress::IpAddress(
        const sockaddr &addr, 
        std::size_t addrLen, 
        Type type) :
            mType(type),
            mResolved(true),
            mIp(),
            mPort()
    {
        init(addr, addrLen, type);        
    }

    void IpAddress::init(const sockaddr &addr, std::size_t addrLen, Type type)
    {
        RCF_UNUSED_VARIABLE(addrLen);

        mType = type;
        mResolved = true;
        mIp.clear();
        mPort = 0;

        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));

        if (type == V4)
        {
            RCF_ASSERT_EQ(addrLen , sizeof(sockaddr_in));
            sockaddr_in * pAddrV4 = (sockaddr_in *) &addr;
            memcpy(&mAddrV4, pAddrV4, sizeof(mAddrV4));
        }
        else if (type == V6)
        {
            RCF_ASSERT_EQ(addrLen , sizeof(SockAddrIn6));
            SockAddrIn6 * pAddrV6 = (SockAddrIn6 *) &addr;
            memcpy(&mAddrV6, pAddrV6, sizeof(mAddrV6));
        }
        else
        {
            RCF_ASSERT(0);
        }

        extractIpAndPort();
    }

    IpAddress::IpAddress(int fd, Type type) :
        mType(type),
        mResolved(true),
        mIp(),
        mPort()
    {
        memset(&mAddrV4, 0, sizeof(mAddrV4));
        memset(&mAddrV6, 0, sizeof(mAddrV6));

        SockAddrStorage addr;
        Platform::OS::BsdSockets::socklen_t addrLen = sizeof(addr);
        int ret = getsockname(fd, (sockaddr *) &addr, &addrLen);
        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(ret == 0, Exception(_RcfError_GetSockName(), err));

        if (type == V4)
        {
            RCF_ASSERT_EQ(addrLen , sizeof(sockaddr_in));
            sockaddr_in * pAddrV4 = (sockaddr_in *) &addr;
            memcpy(&mAddrV4, pAddrV4, sizeof(mAddrV4));
        }
        else if (type == V6)
        {
            RCF_ASSERT_EQ(addrLen , sizeof(SockAddrIn6));
            SockAddrIn6 * pAddrV6 = (SockAddrIn6 *) &addr;
            memcpy(&mAddrV6, pAddrV6, sizeof(mAddrV6));
        }
        else
        {
            RCF_ASSERT(0);
        }

        extractIpAndPort();
    }


    int IpAddress::createSocket(int socketType, int protocol) const
    {
        RCF_ASSERT(mResolved);

        int family = (mType == V4) ? PF_INET : PF_INET6 ;

        int fd = static_cast<int>( ::socket(family, socketType, protocol) );
        int err = Platform::OS::BsdSockets::GetLastError();

        RCF_VERIFY(
            fd != -1,
            Exception(
            _RcfError_Socket("socket()"), err, RcfSubsystem_Os));

        return fd;
    }

    void IpAddress::getSockAddr(sockaddr *& pSockAddr, Platform::OS::BsdSockets::socklen_t & sockAddrSize) const
    {
        RCF_ASSERT(mResolved);
        if (mType == V4)
        {
            pSockAddr = (sockaddr *) &mAddrV4;
            sockAddrSize = sizeof(mAddrV4);
        }
        else
        {
            pSockAddr = (sockaddr *) &mAddrV6;
            sockAddrSize = sizeof(mAddrV6);
        }
    }

    std::string IpAddress::getIp() const
    {
        return mIp;
    }

    int IpAddress::getPort() const
    {
        return mPort;
    }

    IpAddress::Type IpAddress::getType()
    {
        return mType;
    }

    void IpAddress::resolve() const
    {
        ExceptionPtr e;
        resolve(e);
        RCF_VERIFY(!e, *e);
    }

    IpAddress::Type gDefaultResolveTo = IpAddress::V4_or_V6;

    void IpAddress::setPreferredResolveProtocol(Type type)
    {
        gDefaultResolveTo = type;
    }

    IpAddress::Type IpAddress::getPreferredResolveProtocol()
    {
        return gDefaultResolveTo;
    }

#if RCF_FEATURE_IPV6==1

    void IpAddress::resolve(RCF::ExceptionPtr & e) const
    {
        if ( mResolved )
        {
            return;
        }

        addrinfo hints = {0};

        hints.ai_family = AF_UNSPEC;

        if (mType == V4)
        {
            hints.ai_family = AF_INET;          
        }
        else if (mType == V6)
        {
            hints.ai_family = AF_INET6;
        }

        // Don't do name resolution for numeric IP's.
        if (resolveNumericIp(mIp, AF_INET))
        {
            // Valid IPv4 numeric address.
            hints.ai_flags |= AI_NUMERICHOST;
        }
        else if (resolveNumericIp(mIp, AF_INET6))
        {
            // Valid IPv6 numeric address.
            hints.ai_flags |= AI_NUMERICHOST;
        }

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4995) // 'sprintf': name was marked as #pragma deprecated
#pragma warning(disable: 4996) // 'sprintf': This function or variable may be unsafe.
#endif

        char szPort[16] = {0};
        sprintf(szPort, "%d", mPort);

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

        addrinfo * pAddrInfoRet = NULL;
        int ret = getaddrinfo(mIp.c_str(), szPort, &hints, &pAddrInfoRet);
        int dwErr = Platform::OS::BsdSockets::GetLastError();

        if (ret != 0)
        {
            e.reset( new Exception(_RcfError_DnsLookup(mIp), dwErr) );
            return;
        }


        using namespace boost::multi_index::detail;
        scope_guard freeAddrInfoGuard = make_guard(&freeaddrinfo, pAddrInfoRet);
        RCF_UNUSED_VARIABLE(freeAddrInfoGuard);

        // Find first V4 and first V6 address that getaddrinfo() returned. 
        addrinfo * addrinfoVec[2] = {0};
        std::size_t addrinfoVecPos = 0;

        addrinfo * paddrinfoV4 = NULL;
        addrinfo * paddrinfoV6 = NULL;
        
        for(addrinfo * ptr=pAddrInfoRet; ptr != NULL ;ptr=ptr->ai_next) 
        {
            if (ptr->ai_family == AF_INET && !paddrinfoV4)
            {
                paddrinfoV4 = ptr;
                addrinfoVec[addrinfoVecPos++] = paddrinfoV4;
            }
            else if (ptr->ai_family == AF_INET6 && !paddrinfoV6)
            {
                paddrinfoV6 = ptr;
                addrinfoVec[addrinfoVecPos++] = paddrinfoV6;
            }
        }

        // Determine whether to go with V4 or V6.
        addrinfo * paddrinfo = NULL;

        if (paddrinfoV4 && !paddrinfoV6 )
        {
            paddrinfo = paddrinfoV4;
        }
        else if (!paddrinfoV4 && paddrinfoV6)
        {
            paddrinfo = paddrinfoV6;
        }
        else if (paddrinfoV4 && paddrinfoV6)
        {
            IpAddress::Type type = getPreferredResolveProtocol();
            if (type == V4)
            {
                paddrinfo = paddrinfoV4;
            }
            else if (type == V6)
            {
                paddrinfo = paddrinfoV6;
            }
            else
            {
                RCF_ASSERT(addrinfoVecPos > 0);
                paddrinfo = addrinfoVec[0];
            }
        }

        RCF_ASSERT(paddrinfo);

        if (paddrinfo->ai_family == AF_INET)
        {
            sockaddr_in * sockaddr_ipv4 = (sockaddr_in *) paddrinfo->ai_addr;
            mType = V4;
            memcpy(&mAddrV4, sockaddr_ipv4, sizeof(mAddrV4));
            mResolved = true;
        }
        else if (paddrinfo->ai_family == AF_INET6)
        {
            SockAddrIn6 * sockaddr_ipv6 = (SockAddrIn6 *) paddrinfo->ai_addr;
            mType = V6;
            memcpy(&mAddrV6, sockaddr_ipv6, sizeof(mAddrV6));
            mResolved = true;
        }
    }

    void IpAddress::extractIpAndPort()
    {
        RCF_ASSERT(mResolved);
        RCF_ASSERT( mType == V4 || mType == V6 );

        sockaddr * pSockAddr = NULL;
        Platform::OS::BsdSockets::socklen_t sockAddrSize = 0;
        getSockAddr(pSockAddr, sockAddrSize);

        const std::size_t BufferSize = 512;
        char Buffer[BufferSize];
        int ret = getnameinfo(
            pSockAddr,
            sockAddrSize, 
            Buffer, 
            BufferSize, 
            NULL, 
            0, 
            NI_NUMERICHOST);

        int err = Platform::OS::BsdSockets::GetLastError();

        RCF_VERIFY(ret == 0, Exception(_RcfError_ParseSockAddr(), err));

        mIp = Buffer;

        if (mType == V4)
        {
            mPort = ntohs(mAddrV4.sin_port);
        }
        else
        {
            mPort = ntohs(mAddrV6.sin6_port);
        }
    }

#else

    void IpAddress::resolve(ExceptionPtr & e) const
    {
        if ( mResolved )
        {
            return;
        }

        // Try resolution of numeric addresses first.
        unsigned long test = inet_addr(mIp.c_str());
        if (test != INADDR_NONE)
        {
            mAddrV4.sin_addr.s_addr = test;
            mAddrV4.sin_family = AF_INET;
            mAddrV4.sin_port = htons( static_cast<u_short>(mPort) );
            mResolved = true;
            mType = V4;
            return;
        }

        // Then try regular name resolution.
        hostent *hostDesc = ::gethostbyname( mIp.c_str() );
        if (hostDesc)
        {
            char *szIp = ::inet_ntoa( * (in_addr*) hostDesc->h_addr_list[0]);
            if (szIp == NULL)
            {
                e.reset( new Exception(_RcfError_DnsLookup(mIp) ));
            }
            mAddrV4.sin_addr.s_addr = ::inet_addr(szIp);
            mAddrV4.sin_family = AF_INET;
            mAddrV4.sin_port = htons( static_cast<u_short>(mPort) );
            mResolved = true;
            mType = V4;
        }
        else
        {
            e.reset( new Exception(_RcfError_DnsLookup(mIp) ));
        }
    }

    void IpAddress::extractIpAndPort()
    {
        RCF_ASSERT(mResolved);
        RCF_ASSERT_EQ( mType , V4 );

        char *szIp = ::inet_ntoa(mAddrV4.sin_addr);
        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(szIp, Exception(_RcfError_ParseSockAddr(), err));

        mIp = szIp;
        mPort = ntohs(mAddrV4.sin_port);
    }

#endif

    void IpAddress::setPort(int port)
    {
        mPort = port;
        if (mResolved)
        {
            if (mType == V4)
            {
                mAddrV4.sin_port = htons( static_cast<u_short>(port) );
            }
            else
            {
                mAddrV6.sin6_port = htons( static_cast<u_short>(port) );
            }
        }
    }

    std::string IpAddress::string() const
    {
        MemOstream os;
        os << mIp << ":" << mPort;
        return os.string();
    }

    bool IpAddress::empty() const
    {
        return *this == IpAddress();
    }

    bool IpAddress::isResolved() const
    {
        return mResolved;
    }

    bool IpAddress::isBroadcast() const
    {
        RCF_ASSERT(mResolved);

        switch(mType)
        {
        case V4:

            return (ntohl(mAddrV4.sin_addr.s_addr) & 0x000000FF) == 0x000000FF;

        case V6:

            // IPV6 does not do broadcast.
            return false;

        default:
            RCF_ASSERT(0);
            return false;
        }
    }

    bool IpAddress::isMulticast() const
    {
        RCF_ASSERT(mResolved);

        switch(mType)
        {
        case V4:

            return (ntohl(mAddrV4.sin_addr.s_addr) & 0xE0000000) == 0xE0000000;

#if RCF_FEATURE_IPV6==1
        case V6:
            return mAddrV6.sin6_addr.s6_addr[15] == 0xFF;
#endif


        default:
            RCF_ASSERT(0);
            return false;
        }
    }

    bool IpAddress::isLoopback() const
    {
        RCF_ASSERT(mResolved);

        switch(mType)
        {
        case V4:

            {
                RCF::IpAddress loopBackV4("127.0.0.1");
                loopBackV4.resolve();
                return *this == loopBackV4;
            }

#if RCF_FEATURE_IPV6==1
        case V6:

            {
                RCF::IpAddress loopBackV6("::1");
                loopBackV6.resolve();
                return *this == loopBackV6;
            }
#endif


        default:
            RCF_ASSERT(0);
            return false;
        }
    }

    bool IpAddress::matches(const IpAddress & rhs, std::size_t bits) const
    {
        resolve();
        rhs.resolve();

        if (mType == rhs.mType)
        {
            sockaddr * pSockAddr = NULL;
            Platform::OS::BsdSockets::socklen_t sockAddrSize = 0;
            getSockAddr(pSockAddr, sockAddrSize);

            sockaddr * pSockAddrRhs = NULL;
            Platform::OS::BsdSockets::socklen_t sockAddrSizeRhs = 0;
            rhs.getSockAddr(pSockAddrRhs, sockAddrSizeRhs);

            if (mType == V4)
            {
                sockaddr_in * pAddrV4 = (sockaddr_in *) pSockAddr;
                sockaddr_in * pAddrRhsV4 = (sockaddr_in *) pSockAddrRhs;

                if (bits > 32)
                {
                    bits = 32;
                }

                boost::uint32_t mask = 0;
                if (bits == 0)
                {
                    mask = 0;
                }
                else
                {
                    if (isPlatformLittleEndian())
                    {
                        mask = boost::uint32_t(-1) >> (32-bits);
                    }
                    else
                    {
                        mask = boost::uint32_t(-1) << (32-bits);
                    }
                }

                if (    (pAddrV4->sin_addr.s_addr & mask)
                    ==  (pAddrRhsV4->sin_addr.s_addr & mask))
                {
                    return true;
                }
            }
            else if (mType == V6)
            {
#if RCF_FEATURE_IPV6==1
                SockAddrIn6 * pAddrV6 = (SockAddrIn6 *) pSockAddr;
                SockAddrIn6 * pAddrRhsV6 = (SockAddrIn6 *) pSockAddrRhs;

                if (bits > 128)
                {
                    bits = 128;
                }

                RCF_ASSERT(bits % 8 == 0);

                u_char * pLhs = NULL;
                u_char * pRhs = NULL;
                if (isPlatformLittleEndian())
                {
                    pLhs = pAddrV6->sin6_addr.s6_addr + (128-bits)/8;
                    pRhs = pAddrRhsV6->sin6_addr.s6_addr + (128-bits)/8;
                }
                else
                {
                    pLhs = pAddrV6->sin6_addr.s6_addr;
                    pRhs = pAddrRhsV6->sin6_addr.s6_addr;
                }
                std::size_t bytesToCompare = bits / 8;

                if (0 == memcmp(pLhs, pRhs, bytesToCompare))
                {
                    return true;
                }
#else
                RCF_ASSERT(0);
                return true;
#endif
            }
        }

        return false;
    }

    bool IpAddress::operator==(const IpAddress & rhs) const
    {
        // Compare the members that must be the same.
        if (    mType == rhs.mType
            &&  mResolved == rhs.mResolved
            &&  mIp == rhs.mIp
            &&  mPort == rhs.mPort)
        {
            // Compare relevant V4/V6 addresses.
            if (mType == V4_or_V6)
            {
                RCF_ASSERT(!mResolved);
                return true;
            }
            else if (mType == V4)
            {
                int ret = memcmp(&mAddrV4, &rhs.mAddrV4, sizeof(mAddrV4));
                return ret == 0;
            }
            else if (mType == V6)
            {
                int ret = memcmp(&mAddrV6, &rhs.mAddrV6, sizeof(mAddrV6));
                return ret == 0;
            }
        }

        return false;
    }

    bool IpAddress::operator!=(const IpAddress & rhs) const
    {
        return !(*this == rhs);
    }

    bool IpAddress::operator<(const IpAddress &rhs) const
    {
        if (    mIp < rhs.mIp
            ||  (   mIp == rhs.mIp
                &&  mPort < rhs.mPort))
        {
            return true;
        }

        return false;
    }

} // namespace RCF

