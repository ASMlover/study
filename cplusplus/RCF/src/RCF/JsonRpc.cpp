
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

#include <RCF/JsonRpc.hpp>

#include <RCF/MemStream.hpp>

namespace RCF {

    JsonRpcRequest::JsonRpcRequest(ByteBuffer message) : mMessageBuffer(message)
    {
        MemIstream is(message.getPtr(), message.getLength());
        bool parsedOk = json_spirit::read_stream(is, mJsonRequest);
        if (!parsedOk)
        {
            RCF_THROW(Exception(_RcfError_ParseJsonRpcRequest()));
        }
        json_spirit::Object obj = mJsonRequest.get_obj();

        for(json_spirit::Object::size_type i=0; i!=obj.size(); ++i)
        {
            const json_spirit::Pair & pair = obj[i];

            const std::string & name  = pair.name_;
            const json_spirit::Value &  value = pair.value_;

            if (name == "method")
            {
                mMethodName = value.get_str();
            }
            else if (name == "id")
            {
                if (value.is_null())
                {
                    mRequestId = 0;
                    mIsNotification = true;
                }
                else
                {
                    mRequestId = value.get_uint64();
                    mIsNotification = false;
                }
            }
            else if (name == "params")
            {
                mJsonParams = value.get_array();
            }
        }

    }

    bool JsonRpcRequest::isNotification() const
    {
        return mIsNotification;
    }

    const std::string & JsonRpcRequest::getMethodName() const
    {
        return mMethodName;
    }

    const json_spirit::Array & JsonRpcRequest::getJsonParams() const
    {
        return mJsonParams;
    }

    boost::uint64_t JsonRpcRequest::getRequestId() const
    {
        return mRequestId;
    }

    JsonRpcResponse::JsonRpcResponse(boost::uint64_t requestId)
    {
        mJsonResponse["id"] = requestId;
        mJsonResponse["result"] = json_spirit::mValue();
        mJsonResponse["error"] = json_spirit::mValue();
    }

    json_spirit::mObject & JsonRpcResponse::getJsonResponse()
    {
        return mJsonResponse;
    }

} // namespace RCF
