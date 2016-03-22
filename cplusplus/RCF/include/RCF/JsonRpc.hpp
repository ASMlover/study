
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

#ifndef INCLUDE_RCF_JSONRPC_HPP
#define INCLUDE_RCF_JSONRPC_HPP

#include <RCF/ByteBuffer.hpp>

#include <string>

#include <boost/cstdint.hpp>

#include "json_spirit_reader_template.h"
#include "json_spirit_writer_template.h"

namespace RCF {

    class RCF_EXPORT JsonRpcRequest
    {
    public:
        JsonRpcRequest(ByteBuffer message);
        bool isNotification() const;
        const std::string & getMethodName() const;
        const json_spirit::Array & getJsonParams() const;
        boost::uint64_t getRequestId() const;

    private:

        ByteBuffer mMessageBuffer;

        json_spirit::Value mJsonRequest;
        json_spirit::Array mJsonParams;

        std::string mMethodName;
        bool mIsNotification;
        boost::uint64_t mRequestId;
    };

    class RCF_EXPORT JsonRpcResponse
    {
    public:

        JsonRpcResponse(boost::uint64_t requestId);
        json_spirit::mObject & getJsonResponse();

    private:

        json_spirit::mObject mJsonResponse;

    };

} // namespace RCF

#endif // ! INCLUDE_RCF_JSONRPC_HPP
