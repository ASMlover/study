
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

#ifndef INCLUDE_RCF_CLIENTTRANSPORT_HPP
#define INCLUDE_RCF_CLIENTTRANSPORT_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/weak_ptr.hpp>

#include <RCF/AsioFwd.hpp>
#include <RCF/Enums.hpp>
#include <RCF/Filter.hpp>
#include <RCF/ByteBuffer.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    class Endpoint;
    typedef boost::shared_ptr<Endpoint> EndpointPtr;

    class RcfServer;

    class OverlappedAmi;
    typedef boost::shared_ptr<OverlappedAmi> OverlappedAmiPtr;

    class ClientStub;
    class RcfSession;
    typedef boost::weak_ptr<RcfSession> RcfSessionWeakPtr;

    class RCF_EXPORT ClientTransportCallback
    {
    public:
        ClientTransportCallback() : mpAsyncDispatcher() {}
        virtual ~ClientTransportCallback() {}
        virtual void onConnectCompleted(bool alreadyConnected = false) = 0;
        virtual void onSendCompleted() = 0;
        virtual void onReceiveCompleted() = 0;
        virtual void onTimerExpired() = 0;
        virtual void onError(const std::exception &e) = 0;

        void setAsyncDispatcher(RcfServer & server);
        RcfServer * getAsyncDispatcher();

        virtual bool isClientStub() const { return false; }

    private:
        RcfServer * mpAsyncDispatcher;
    };

    class ClientStub;

    /// Base class for all client transports.
    class RCF_EXPORT ClientTransport
    {
    public:
        ClientTransport();
        ClientTransport(const ClientTransport & rhs);

        virtual ~ClientTransport()
        {}

        // *** SWIG BEGIN ***
        
        /// Returns the transport type of this client transport.
        virtual TransportType getTransportType() = 0;

        /// Sets maximum incoming message length. Incoming messages that are larger
        /// than this size will be dropped.
        void setMaxIncomingMessageLength(std::size_t maxMessageLength);

        /// Returns maximum incoming message length.
        std::size_t getMaxIncomingMessageLength() const;

        /// Returns the byte size of the last request sent on the client transport.
        std::size_t getLastRequestSize();

        /// Returns the byte size of the last response received on the client transport.
        std::size_t getLastResponseSize();

        /// Returns the running total of bytes sent on the client transport.
        boost::uint64_t getRunningTotalBytesSent();

        /// Returns the running total of bytes received on the client transport.
        boost::uint64_t getRunningTotalBytesReceived();

        /// Resets the bytes-sent and bytes-received running totals to zero.
        void resetRunningTotals();

        // *** SWIG END ***

        

        virtual 
        std::auto_ptr<ClientTransport> clone() const = 0;

        virtual 
        EndpointPtr getEndpointPtr() const = 0;
       
        virtual 
        int send(
            ClientTransportCallback &     clientStub, 
            const std::vector<ByteBuffer> & data, 
            unsigned int                    timeoutMs) = 0;

        virtual 
        int receive(
            ClientTransportCallback &     clientStub, 
            ByteBuffer &                    byteBuffer, 
            unsigned int                    timeoutMs) = 0;

        virtual 
        bool isConnected() = 0;

        virtual 
        void connect(
            ClientTransportCallback &     clientStub, 
            unsigned int                    timeoutMs) = 0;

        virtual 
        void disconnect(
            unsigned int                    timeoutMs = 0) = 0;

        virtual 
        void setTransportFilters(
            const std::vector<FilterPtr> &  filters) = 0;
       
        virtual 
        void getTransportFilters(
            std::vector<FilterPtr> &        filters) = 0;

        // Deprecated - use setMaxIncomingMessageLength()/getMaxIncomingMessageLength() instead.
        void setMaxMessageLength(std::size_t maxMessageLength);
        std::size_t getMaxMessageLength() const;


        RcfSessionWeakPtr getRcfSession();
        void setRcfSession(RcfSessionWeakPtr rcfSessionWeakPtr);


        void setAsync(bool async);

        virtual void cancel();

        virtual void setTimer(
            boost::uint32_t timeoutMs,
            ClientTransportCallback * pClientStub = NULL) = 0;

        virtual void associateWithIoService(AsioIoService & ioService);
        virtual bool isAssociatedWithIoService();

        virtual bool supportsTransportFilters()
        {
            return true;
        }

    private:
        std::size_t mMaxMessageLength;
        RcfSessionWeakPtr mRcfSessionWeakPtr;

    protected:
        std::size_t mLastRequestSize;
        std::size_t mLastResponseSize;

        boost::uint64_t mRunningTotalBytesSent;
        boost::uint64_t mRunningTotalBytesReceived;

        bool mAsync;

        friend class ClientStub;
    };

    typedef boost::shared_ptr<ClientTransport> ClientTransportPtr;

    typedef std::auto_ptr<ClientTransport> ClientTransportAutoPtr;

    typedef boost::shared_ptr< ClientTransportAutoPtr > ClientTransportAutoPtrPtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_CLIENTTRANSPORT_HPP
