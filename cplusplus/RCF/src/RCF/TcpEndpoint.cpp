
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

#include <RCF/TcpEndpoint.hpp>

#include <boost/config.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/SerializationProtocol.hpp>

#include <RCF/TcpServerTransport.hpp>
#include <RCF/TcpClientTransport.hpp>

namespace RCF {

    TcpEndpoint::TcpEndpoint()
    {}

    TcpEndpoint::TcpEndpoint(int port) :
        mIpAddress("127.0.0.1", port)
    {}

    TcpEndpoint::TcpEndpoint(const std::string &ip, int port) :
        mIpAddress(ip, port)
    {}

    TcpEndpoint::TcpEndpoint(const IpAddress & ipAddress) :
        mIpAddress(ipAddress)
    {}

    TcpEndpoint::TcpEndpoint(const TcpEndpoint &rhs) :
        mIpAddress(rhs.mIpAddress)
    {}

    EndpointPtr TcpEndpoint::clone() const
    {
        return EndpointPtr(new TcpEndpoint(*this));
    }

    std::string TcpEndpoint::getIp() const
    {
        return mIpAddress.getIp();
    }

    int TcpEndpoint::getPort() const
    {
        return mIpAddress.getPort();
    }

    std::string TcpEndpoint::asString() const
    {
        MemOstream os;
        std::string ip = getIp();
        if (ip.empty())
        {
            ip = "127.0.0.1";
        }
        os << "tcp://" << ip << ":" << getPort();
        return os.string();
    }

    IpAddress TcpEndpoint::getIpAddress() const
    {
        return mIpAddress;
    }

    std::auto_ptr<ServerTransport> TcpEndpoint::createServerTransport() const
    {
        return std::auto_ptr<ServerTransport>(
            new RCF::TcpServerTransport(mIpAddress));
    }

    std::auto_ptr<ClientTransport> TcpEndpoint::createClientTransport() const
    {
        return std::auto_ptr<ClientTransport>(
            new RCF::TcpClientTransport(mIpAddress));
    }

} // namespace RCF
