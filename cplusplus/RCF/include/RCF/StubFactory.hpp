
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

#ifndef INCLUDE_RCF_STUBFACTORY_HPP
#define INCLUDE_RCF_STUBFACTORY_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/RcfClient.hpp>
#include <RCF/ServerStub.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    class I_RcfClient;
    typedef boost::shared_ptr<I_RcfClient> RcfClientPtr;

    class StubFactory
    {
    public:
        virtual ~StubFactory()
        {}

        virtual RcfClientPtr makeServerStub() = 0;
    };

    template<typename T, typename I1>
    class StubFactory_1 : public StubFactory
    {
    public:
        RcfClientPtr makeServerStub()
        {
            boost::shared_ptr<T> tPtr( new T );

            boost::shared_ptr< RCF::I_Deref<T> > derefPtr(
                new RCF::DerefSharedPtr<T>(tPtr) );

            RcfClientPtr rcfClientPtr =
                createServerStub( (I1 *) NULL, (T *) NULL, derefPtr);

            return rcfClientPtr;
        }
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_STUBFACTORY_HPP
