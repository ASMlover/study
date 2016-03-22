
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

#include <RCF/ServerStub.hpp>

#include <iterator>

#include <RCF/RcfClient.hpp>

namespace RCF {

    void ServerBinding::setAccessControl(CbAccessControl cbAccessControl)
    {
        Lock lock(mMutex);
        mCbAccessControl = cbAccessControl;
    }


    void ServerBinding::invoke(
        const std::string &         subInterface,
        int                         fnId,
        RcfSession &                session)
    {
        // Check access control.

        {
            Lock lock(mMutex);
            if (mCbAccessControl)
            {
                bool ok = mCbAccessControl(fnId);
                if (!ok)
                {
                    RCF_THROW( RCF::Exception(_RcfError_ServerStubAccessDenied()));
                }
            }
        }

        // No mutex here, since there is never anyone writing to mInvokeFunctorMap.

        RCF_VERIFY(
            mInvokeFunctorMap.find(subInterface) != mInvokeFunctorMap.end(),
            Exception(_RcfError_UnknownInterface(subInterface)));

        mInvokeFunctorMap[subInterface](fnId, session);
    }

} // namespace RCF
