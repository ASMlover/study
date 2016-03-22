
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

#ifndef INCLUDE_RCF_UDPCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_UDPCLIENTTRANSPORT_HPP

#include <RCF/ClientTransport.hpp>
#include <RCF/Export.hpp>
#include <RCF/IpAddress.hpp>
#include <RCF/IpClientTransport.hpp>

namespace RCF {

    class UdpClientTransport;

    typedef boost::shared_ptr<UdpClientTransport> UdpClientTransportPtr;
   
    class RCF_EXPORT UdpClientTransport : 
        public ClientTransport, 
        public IpClientTransport
    {
    private:
        IpAddress                                   mSrcIp;
        IpAddress                                   mDestIp;
        IpAddress                                   mFromIp;
        ReallocBufferPtr                            mReadVecPtr;
        ReallocBufferPtr                            mWriteVecPtr;
        bool                                        mAsync;
        int                                         mSock;

    public:
        UdpClientTransport(const IpAddress & ipAddress);
        UdpClientTransport(const UdpClientTransport &rhs);
        ~UdpClientTransport();

         TransportType getTransportType();

        ClientTransportAutoPtr 
                        clone() const;

        EndpointPtr     getEndpointPtr() const;

        void            connect(
                            ClientTransportCallback &clientStub, 
                            unsigned int timeoutMs);

        void            disconnect(unsigned int timeoutMs);

        int             send(
                            ClientTransportCallback &clientStub, 
                            const std::vector<ByteBuffer> &data, 
                            unsigned int timeoutMs);

        int             receive(
                            ClientTransportCallback &clientStub, 
                            ByteBuffer &byteBuffer, 
                            unsigned int timeoutMs);

        void            close();
        bool            isConnected();

        void            setTransportFilters(
                            const std::vector<FilterPtr> &filters);

        void            getTransportFilters(
                            std::vector<FilterPtr> &filters);

        int             getNativeHandle() const;

        void            setTimer(boost::uint32_t timeoutMs, ClientTransportCallback *pClientStub);

        bool            supportsTransportFilters()
        {
            return false;
        }
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_UDPCLIENTTRANSPORT_HPP
