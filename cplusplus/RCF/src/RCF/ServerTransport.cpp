
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

#include <RCF/ServerTransport.hpp>

#include <RCF/Service.hpp>

namespace RCF {

    ServerTransport::ServerTransport() :
        mRpcProtocol(Rp_Rcf),
        mCustomFraming(false),
        mReadWriteMutex(WriterPriority),
        mMaxMessageLength(getDefaultMaxMessageLength()),
        mConnectionLimit(0),
        mInitialNumberOfConnections(1)
    {}

    ServerTransport & ServerTransport::setMaxMessageLength(std::size_t maxMessageLength)
    {
        return setMaxIncomingMessageLength(maxMessageLength);
    }

    std::size_t ServerTransport::getMaxMessageLength() const
    {
        return getMaxIncomingMessageLength();
    }

    ServerTransport & ServerTransport::setMaxIncomingMessageLength(std::size_t maxMessageLength)
    {
        WriteLock writeLock(mReadWriteMutex);
        mMaxMessageLength = maxMessageLength;
        return *this;
    }

    std::size_t ServerTransport::getMaxIncomingMessageLength() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mMaxMessageLength;
    }

    std::size_t ServerTransport::getConnectionLimit() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mConnectionLimit;
    }

    ServerTransport & ServerTransport::setConnectionLimit(
        std::size_t connectionLimit)
    {
        WriteLock writeLock(mReadWriteMutex);
        mConnectionLimit = connectionLimit;

        return *this;
    }

    std::size_t ServerTransport::getInitialNumberOfConnections() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mInitialNumberOfConnections;
    }

    ServerTransport & ServerTransport::setInitialNumberOfConnections(
        std::size_t initialNumberOfConnections)
    {
        WriteLock writeLock(mReadWriteMutex);
        mInitialNumberOfConnections = initialNumberOfConnections;

        return *this;
    }

    void ServerTransport::setRpcProtocol(RpcProtocol rpcProtocol)
    {
        mRpcProtocol = rpcProtocol;

        // For JSON-RPC over HTTP/S, enable HTTP framing.
        if (rpcProtocol == Rp_JsonRpc)
        {
            TransportType tt = getTransportType();
            if (tt == Tt_Http || tt == Tt_Https)
            {
                mCustomFraming = true;
            }
        }
    }

    RpcProtocol ServerTransport::getRpcProtocol() const
    {
        return mRpcProtocol;
    }

    ServerTransport & ServerTransport::setThreadPool(
        ThreadPoolPtr threadPoolPtr)
    {
        I_Service & svc = dynamic_cast<I_Service &>(*this);
        svc.setThreadPool(threadPoolPtr);
        return *this;
    }

    ServerTransport & ServerTransport::setSupportedProtocols(const std::vector<TransportProtocol> & protocols)
    {
        mSupportedProtocols = protocols;
        return *this;
    }

    const std::vector<TransportProtocol> & ServerTransport::getSupportedProtocols() const
    {
        return mSupportedProtocols;
    }

    std::size_t gDefaultMaxMessageLength = 1024*1024; // 1 Mb

    std::size_t getDefaultMaxMessageLength()
    {
        return gDefaultMaxMessageLength;
    }

    void setDefaultMaxMessageLength(std::size_t maxMessageLength)
    {
        gDefaultMaxMessageLength = maxMessageLength;
    }

    NetworkSession::NetworkSession() :
        mEnableReconnect(true),
        mBytesReceivedCounter(0),
        mBytesSentCounter(0),
        mLastActivityTimestampMs(0)
    {
    }

    NetworkSession::~NetworkSession()
    {
    }

    void NetworkSession::setEnableReconnect(bool enableReconnect)
    {
        mEnableReconnect = enableReconnect;
    }

    bool NetworkSession::getEnableReconnect()
    {
        return mEnableReconnect;
    }

    boost::uint64_t NetworkSession::getTotalBytesReceived() const
    {
        return mBytesReceivedCounter;
    }

    boost::uint64_t NetworkSession::getTotalBytesSent() const
    {
        return mBytesSentCounter;
    }

    SessionPtr NetworkSession::getSessionPtr() const
    {
        return mRcfSessionPtr;
    }

    boost::uint32_t NetworkSession::getLastActivityTimestamp() const
    {
        return mLastActivityTimestampMs;
    }

    void NetworkSession::setLastActivityTimestamp()
    {
        mLastActivityTimestampMs = RCF::getCurrentTimeMs();
    }

} // namespace RCF
