
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

#include <RCF/ClientTransport.hpp>

#include <RCF/Asio.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/ServerTransport.hpp>

namespace RCF {

    ClientTransport::ClientTransport() :
        mMaxMessageLength(getDefaultMaxMessageLength()),
        mLastRequestSize(0),
        mLastResponseSize(0),
        mRunningTotalBytesSent(0),
        mRunningTotalBytesReceived(0),
        mAsync(false)
    {}

    ClientTransport::ClientTransport(const ClientTransport & rhs) :
        mMaxMessageLength(rhs.mMaxMessageLength),
        mLastRequestSize(),
        mLastResponseSize(0),
        mRunningTotalBytesSent(0),
        mRunningTotalBytesReceived(0),
        mAsync(false)
    {
    }

    bool ClientTransport::isConnected()
    {
        return true;
    }

    void ClientTransport::setMaxMessageLength(std::size_t maxMessageLength)
    {
        setMaxIncomingMessageLength(maxMessageLength);
    }

    std::size_t ClientTransport::getMaxMessageLength() const
    {
        return getMaxIncomingMessageLength();
    }

    void ClientTransport::setMaxIncomingMessageLength(
        std::size_t maxMessageLength)
    {
        mMaxMessageLength = maxMessageLength;
    }

    std::size_t ClientTransport::getMaxIncomingMessageLength() const
    {
        return mMaxMessageLength;
    }

    RcfSessionWeakPtr ClientTransport::getRcfSession()
    {
        return mRcfSessionWeakPtr;
    }

    void ClientTransport::setRcfSession(RcfSessionWeakPtr rcfSessionWeakPtr)
    {
        mRcfSessionWeakPtr = rcfSessionWeakPtr;
    }

    std::size_t ClientTransport::getLastRequestSize()
    {
        return mLastRequestSize;
    }

    std::size_t ClientTransport::getLastResponseSize()
    {
        return mLastResponseSize;
    }

    boost::uint64_t ClientTransport::getRunningTotalBytesSent()
    {
        return mRunningTotalBytesSent;
    }

    boost::uint64_t ClientTransport::getRunningTotalBytesReceived()
    {
        return mRunningTotalBytesReceived;
    }

    void ClientTransport::resetRunningTotals()
    {
        mRunningTotalBytesSent = 0;
        mRunningTotalBytesReceived = 0;
    }

    void ClientTransportCallback::setAsyncDispatcher(RcfServer & server)
    {
        RCF_ASSERT(!mpAsyncDispatcher);
        mpAsyncDispatcher = &server;
    }

    RcfServer * ClientTransportCallback::getAsyncDispatcher()
    {
        return mpAsyncDispatcher;
    }

    void ClientTransport::setAsync(bool async)
    {
        mAsync = async;
    }

    void ClientTransport::associateWithIoService(AsioIoService & ioService)
    {
        RCF_UNUSED_VARIABLE(ioService);
        RCF_ASSERT(0 && "Asynchronous operations not implemented for this transport.");
    }

    bool ClientTransport::isAssociatedWithIoService()
    {
        return false;
    }

    void ClientTransport::cancel()
    {
        RCF_ASSERT(0 && "cancel() not implemented for this transport");
    }

} // namespace RCF
