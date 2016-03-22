
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

#ifndef INCLUDE_RCF_SERVERSTUB_HPP
#define INCLUDE_RCF_SERVERSTUB_HPP

#include <map>
#include <memory>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/Config.hpp>
#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/Token.hpp>

// NB: occurrences of "interface" in this file have been replaced with "interface_", due to obscure bugs with Visual C++

namespace RCF {

    class                                   I_RcfClient;
    typedef boost::shared_ptr<I_RcfClient>  RcfClientPtr;

    template<typename T>
    class I_Deref
    {
    public:
        virtual ~I_Deref() {}
        virtual T &deref() = 0;
    };

    template<typename T>
    class DerefObj : public I_Deref<T>
    {
    public:
        DerefObj(T &t) :
            mT(t)
        {}

        T &deref()
        {
            return mT;
        }

    private:
        T &mT;
    };

    template<typename T>
    class DerefSharedPtr : public I_Deref<T>
    {
    public:
        DerefSharedPtr(boost::shared_ptr<T> tPtr) :
            mTPtr(tPtr)
        {}

        T &deref()
        {
            return *mTPtr;
        }

    private:
        boost::shared_ptr<T> mTPtr;
    };

    RCF_EXPORT void setCurrentCallDesc(
        std::string &                   desc, 
        RCF::MethodInvocationRequest &  request, 
        const char *                    szFunc, 
        const char *                    szArity);

    class StubAccess
    {
    public:

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4702 )
#endif

        template<typename InterfaceT, typename IdT, typename ImplementationT>
        void invoke(
            InterfaceT &                interface_,
            const IdT &                 id,
            RcfSession &                session,
            ImplementationT &           t)
        {
            setCurrentCallDesc(
                session.mCurrentCallDesc, 
                session.mRequest, 
                interface_.getFunctionName(id), 
                interface_.getArity(id));

            RCF_LOG_2() << "RcfServer - begin remote call. " << session.mCurrentCallDesc;
            interface_.invoke(id, session, t);
        }

#ifdef _MSC_VER
#pragma warning( pop )
#endif

        template<typename InterfaceT, typename DerefPtrT>
        void registerInvokeFunctors(
            InterfaceT &                interface_,
            InvokeFunctorMap &          invokeFunctorMap,
            DerefPtrT                   derefPtr)
        {
            interface_.registerInvokeFunctors(invokeFunctorMap, derefPtr);
        }

        template<typename RcfClientT>
        ClientStubPtr getClientStubPtr(const RcfClientT &rcfClient)
        {
            return rcfClient.mClientStubPtr;
        }

    };

    template<typename InterfaceT, typename ImplementationT>
    inline void invoke(
        InterfaceT &                    interface_,
        ImplementationT &               t,
        int                             fnId,
        RcfSession  &                   session)
    {
        switch (fnId) {
        case   0: StubAccess().invoke(interface_, boost::mpl::int_<  0>(), session, t); break;
        case   1: StubAccess().invoke(interface_, boost::mpl::int_<  1>(), session, t); break;
        case   2: StubAccess().invoke(interface_, boost::mpl::int_<  2>(), session, t); break;
        case   3: StubAccess().invoke(interface_, boost::mpl::int_<  3>(), session, t); break;
        case   4: StubAccess().invoke(interface_, boost::mpl::int_<  4>(), session, t); break;
        case   5: StubAccess().invoke(interface_, boost::mpl::int_<  5>(), session, t); break;
        case   6: StubAccess().invoke(interface_, boost::mpl::int_<  6>(), session, t); break;
        case   7: StubAccess().invoke(interface_, boost::mpl::int_<  7>(), session, t); break;
        case   8: StubAccess().invoke(interface_, boost::mpl::int_<  8>(), session, t); break;
        case   9: StubAccess().invoke(interface_, boost::mpl::int_<  9>(), session, t); break;
        case  10: StubAccess().invoke(interface_, boost::mpl::int_< 10>(), session, t); break;
        case  11: StubAccess().invoke(interface_, boost::mpl::int_< 11>(), session, t); break;
        case  12: StubAccess().invoke(interface_, boost::mpl::int_< 12>(), session, t); break;
        case  13: StubAccess().invoke(interface_, boost::mpl::int_< 13>(), session, t); break;
        case  14: StubAccess().invoke(interface_, boost::mpl::int_< 14>(), session, t); break;
        case  15: StubAccess().invoke(interface_, boost::mpl::int_< 15>(), session, t); break;
        case  16: StubAccess().invoke(interface_, boost::mpl::int_< 16>(), session, t); break;
        case  17: StubAccess().invoke(interface_, boost::mpl::int_< 17>(), session, t); break;
        case  18: StubAccess().invoke(interface_, boost::mpl::int_< 18>(), session, t); break;
        case  19: StubAccess().invoke(interface_, boost::mpl::int_< 19>(), session, t); break;
        case  20: StubAccess().invoke(interface_, boost::mpl::int_< 20>(), session, t); break;
        case  21: StubAccess().invoke(interface_, boost::mpl::int_< 21>(), session, t); break;
        case  22: StubAccess().invoke(interface_, boost::mpl::int_< 22>(), session, t); break;
        case  23: StubAccess().invoke(interface_, boost::mpl::int_< 23>(), session, t); break;
        case  24: StubAccess().invoke(interface_, boost::mpl::int_< 24>(), session, t); break;
        case  25: StubAccess().invoke(interface_, boost::mpl::int_< 25>(), session, t); break;
        case  26: StubAccess().invoke(interface_, boost::mpl::int_< 26>(), session, t); break;
        case  27: StubAccess().invoke(interface_, boost::mpl::int_< 27>(), session, t); break;
        case  28: StubAccess().invoke(interface_, boost::mpl::int_< 28>(), session, t); break;
        case  29: StubAccess().invoke(interface_, boost::mpl::int_< 29>(), session, t); break;
        case  30: StubAccess().invoke(interface_, boost::mpl::int_< 30>(), session, t); break;
        case  31: StubAccess().invoke(interface_, boost::mpl::int_< 31>(), session, t); break;
        case  32: StubAccess().invoke(interface_, boost::mpl::int_< 32>(), session, t); break;
        case  33: StubAccess().invoke(interface_, boost::mpl::int_< 33>(), session, t); break;
        case  34: StubAccess().invoke(interface_, boost::mpl::int_< 34>(), session, t); break;
        case  35: StubAccess().invoke(interface_, boost::mpl::int_< 35>(), session, t); break;
        case  36: StubAccess().invoke(interface_, boost::mpl::int_< 36>(), session, t); break;
        case  37: StubAccess().invoke(interface_, boost::mpl::int_< 37>(), session, t); break;
        case  38: StubAccess().invoke(interface_, boost::mpl::int_< 38>(), session, t); break;
        case  39: StubAccess().invoke(interface_, boost::mpl::int_< 39>(), session, t); break;
        case  40: StubAccess().invoke(interface_, boost::mpl::int_< 40>(), session, t); break;
        case  41: StubAccess().invoke(interface_, boost::mpl::int_< 41>(), session, t); break;
        case  42: StubAccess().invoke(interface_, boost::mpl::int_< 42>(), session, t); break;
        case  43: StubAccess().invoke(interface_, boost::mpl::int_< 43>(), session, t); break;
        case  44: StubAccess().invoke(interface_, boost::mpl::int_< 44>(), session, t); break;
        case  45: StubAccess().invoke(interface_, boost::mpl::int_< 45>(), session, t); break;
        case  46: StubAccess().invoke(interface_, boost::mpl::int_< 46>(), session, t); break;
        case  47: StubAccess().invoke(interface_, boost::mpl::int_< 47>(), session, t); break;
        case  48: StubAccess().invoke(interface_, boost::mpl::int_< 48>(), session, t); break;
        case  49: StubAccess().invoke(interface_, boost::mpl::int_< 49>(), session, t); break;
        case  50: StubAccess().invoke(interface_, boost::mpl::int_< 50>(), session, t); break;
        case  51: StubAccess().invoke(interface_, boost::mpl::int_< 51>(), session, t); break;
        case  52: StubAccess().invoke(interface_, boost::mpl::int_< 52>(), session, t); break;
        case  53: StubAccess().invoke(interface_, boost::mpl::int_< 53>(), session, t); break;
        case  54: StubAccess().invoke(interface_, boost::mpl::int_< 54>(), session, t); break;
        case  55: StubAccess().invoke(interface_, boost::mpl::int_< 55>(), session, t); break;
        case  56: StubAccess().invoke(interface_, boost::mpl::int_< 56>(), session, t); break;
        case  57: StubAccess().invoke(interface_, boost::mpl::int_< 57>(), session, t); break;
        case  58: StubAccess().invoke(interface_, boost::mpl::int_< 58>(), session, t); break;
        case  59: StubAccess().invoke(interface_, boost::mpl::int_< 59>(), session, t); break;
        case  60: StubAccess().invoke(interface_, boost::mpl::int_< 60>(), session, t); break;
        case  61: StubAccess().invoke(interface_, boost::mpl::int_< 61>(), session, t); break;
        case  62: StubAccess().invoke(interface_, boost::mpl::int_< 62>(), session, t); break;
        case  63: StubAccess().invoke(interface_, boost::mpl::int_< 63>(), session, t); break;
        case  64: StubAccess().invoke(interface_, boost::mpl::int_< 64>(), session, t); break;
        case  65: StubAccess().invoke(interface_, boost::mpl::int_< 65>(), session, t); break;
        case  66: StubAccess().invoke(interface_, boost::mpl::int_< 66>(), session, t); break;
        case  67: StubAccess().invoke(interface_, boost::mpl::int_< 67>(), session, t); break;
        case  68: StubAccess().invoke(interface_, boost::mpl::int_< 68>(), session, t); break;
        case  69: StubAccess().invoke(interface_, boost::mpl::int_< 69>(), session, t); break;
        case  70: StubAccess().invoke(interface_, boost::mpl::int_< 70>(), session, t); break;
        case  71: StubAccess().invoke(interface_, boost::mpl::int_< 71>(), session, t); break;
        case  72: StubAccess().invoke(interface_, boost::mpl::int_< 72>(), session, t); break;
        case  73: StubAccess().invoke(interface_, boost::mpl::int_< 73>(), session, t); break;
        case  74: StubAccess().invoke(interface_, boost::mpl::int_< 74>(), session, t); break;
        case  75: StubAccess().invoke(interface_, boost::mpl::int_< 75>(), session, t); break;
        case  76: StubAccess().invoke(interface_, boost::mpl::int_< 76>(), session, t); break;
        case  77: StubAccess().invoke(interface_, boost::mpl::int_< 77>(), session, t); break;
        case  78: StubAccess().invoke(interface_, boost::mpl::int_< 78>(), session, t); break;
        case  79: StubAccess().invoke(interface_, boost::mpl::int_< 79>(), session, t); break;
        case  80: StubAccess().invoke(interface_, boost::mpl::int_< 80>(), session, t); break;
        case  81: StubAccess().invoke(interface_, boost::mpl::int_< 81>(), session, t); break;
        case  82: StubAccess().invoke(interface_, boost::mpl::int_< 82>(), session, t); break;
        case  83: StubAccess().invoke(interface_, boost::mpl::int_< 83>(), session, t); break;
        case  84: StubAccess().invoke(interface_, boost::mpl::int_< 84>(), session, t); break;
        case  85: StubAccess().invoke(interface_, boost::mpl::int_< 85>(), session, t); break;
        case  86: StubAccess().invoke(interface_, boost::mpl::int_< 86>(), session, t); break;
        case  87: StubAccess().invoke(interface_, boost::mpl::int_< 87>(), session, t); break;
        case  88: StubAccess().invoke(interface_, boost::mpl::int_< 88>(), session, t); break;
        case  89: StubAccess().invoke(interface_, boost::mpl::int_< 89>(), session, t); break;
        case  90: StubAccess().invoke(interface_, boost::mpl::int_< 90>(), session, t); break;
        case  91: StubAccess().invoke(interface_, boost::mpl::int_< 91>(), session, t); break;
        case  92: StubAccess().invoke(interface_, boost::mpl::int_< 92>(), session, t); break;
        case  93: StubAccess().invoke(interface_, boost::mpl::int_< 93>(), session, t); break;
        case  94: StubAccess().invoke(interface_, boost::mpl::int_< 94>(), session, t); break;
        case  95: StubAccess().invoke(interface_, boost::mpl::int_< 95>(), session, t); break;
        case  96: StubAccess().invoke(interface_, boost::mpl::int_< 96>(), session, t); break;
        case  97: StubAccess().invoke(interface_, boost::mpl::int_< 97>(), session, t); break;
        case  98: StubAccess().invoke(interface_, boost::mpl::int_< 98>(), session, t); break;
        case  99: StubAccess().invoke(interface_, boost::mpl::int_< 99>(), session, t); break;
        case 100: StubAccess().invoke(interface_, boost::mpl::int_<100>(), session, t); break;
        case 101: StubAccess().invoke(interface_, boost::mpl::int_<101>(), session, t); break;
        case 102: StubAccess().invoke(interface_, boost::mpl::int_<102>(), session, t); break;
        case 103: StubAccess().invoke(interface_, boost::mpl::int_<103>(), session, t); break;
        case 104: StubAccess().invoke(interface_, boost::mpl::int_<104>(), session, t); break;
        case 105: StubAccess().invoke(interface_, boost::mpl::int_<105>(), session, t); break;
        case 106: StubAccess().invoke(interface_, boost::mpl::int_<106>(), session, t); break;
        case 107: StubAccess().invoke(interface_, boost::mpl::int_<107>(), session, t); break;
        case 108: StubAccess().invoke(interface_, boost::mpl::int_<108>(), session, t); break;
        case 109: StubAccess().invoke(interface_, boost::mpl::int_<109>(), session, t); break;
        case 110: StubAccess().invoke(interface_, boost::mpl::int_<110>(), session, t); break;
        case 111: StubAccess().invoke(interface_, boost::mpl::int_<111>(), session, t); break;
        case 112: StubAccess().invoke(interface_, boost::mpl::int_<112>(), session, t); break;
        case 113: StubAccess().invoke(interface_, boost::mpl::int_<113>(), session, t); break;
        case 114: StubAccess().invoke(interface_, boost::mpl::int_<114>(), session, t); break;
        case 115: StubAccess().invoke(interface_, boost::mpl::int_<115>(), session, t); break;
        case 116: StubAccess().invoke(interface_, boost::mpl::int_<116>(), session, t); break;
        case 117: StubAccess().invoke(interface_, boost::mpl::int_<117>(), session, t); break;
        case 118: StubAccess().invoke(interface_, boost::mpl::int_<118>(), session, t); break;
        case 119: StubAccess().invoke(interface_, boost::mpl::int_<119>(), session, t); break;
        case 120: StubAccess().invoke(interface_, boost::mpl::int_<120>(), session, t); break;
        case 121: StubAccess().invoke(interface_, boost::mpl::int_<121>(), session, t); break;
        case 122: StubAccess().invoke(interface_, boost::mpl::int_<122>(), session, t); break;
        case 123: StubAccess().invoke(interface_, boost::mpl::int_<123>(), session, t); break;
        case 124: StubAccess().invoke(interface_, boost::mpl::int_<124>(), session, t); break;
        case 125: StubAccess().invoke(interface_, boost::mpl::int_<125>(), session, t); break;
        case 126: StubAccess().invoke(interface_, boost::mpl::int_<126>(), session, t); break;
        case 127: StubAccess().invoke(interface_, boost::mpl::int_<127>(), session, t); break;
        case 128: StubAccess().invoke(interface_, boost::mpl::int_<128>(), session, t); break;
        case 129: StubAccess().invoke(interface_, boost::mpl::int_<129>(), session, t); break;
        case 130: StubAccess().invoke(interface_, boost::mpl::int_<130>(), session, t); break;
        case 131: StubAccess().invoke(interface_, boost::mpl::int_<131>(), session, t); break;
        case 132: StubAccess().invoke(interface_, boost::mpl::int_<132>(), session, t); break;
        case 133: StubAccess().invoke(interface_, boost::mpl::int_<133>(), session, t); break;
        case 134: StubAccess().invoke(interface_, boost::mpl::int_<134>(), session, t); break;
        case 135: StubAccess().invoke(interface_, boost::mpl::int_<135>(), session, t); break;
        case 136: StubAccess().invoke(interface_, boost::mpl::int_<136>(), session, t); break;
        case 137: StubAccess().invoke(interface_, boost::mpl::int_<137>(), session, t); break;
        case 138: StubAccess().invoke(interface_, boost::mpl::int_<138>(), session, t); break;
        case 139: StubAccess().invoke(interface_, boost::mpl::int_<139>(), session, t); break;
        case 140: StubAccess().invoke(interface_, boost::mpl::int_<140>(), session, t); break;
        case 141: StubAccess().invoke(interface_, boost::mpl::int_<141>(), session, t); break;
        case 142: StubAccess().invoke(interface_, boost::mpl::int_<142>(), session, t); break;
        case 143: StubAccess().invoke(interface_, boost::mpl::int_<143>(), session, t); break;
        case 144: StubAccess().invoke(interface_, boost::mpl::int_<144>(), session, t); break;
        case 145: StubAccess().invoke(interface_, boost::mpl::int_<145>(), session, t); break;
        case 146: StubAccess().invoke(interface_, boost::mpl::int_<146>(), session, t); break;
        case 147: StubAccess().invoke(interface_, boost::mpl::int_<147>(), session, t); break;
        case 148: StubAccess().invoke(interface_, boost::mpl::int_<148>(), session, t); break;
        case 149: StubAccess().invoke(interface_, boost::mpl::int_<149>(), session, t); break;
        case 150: StubAccess().invoke(interface_, boost::mpl::int_<150>(), session, t); break;
        case 151: StubAccess().invoke(interface_, boost::mpl::int_<151>(), session, t); break;
        case 152: StubAccess().invoke(interface_, boost::mpl::int_<152>(), session, t); break;
        case 153: StubAccess().invoke(interface_, boost::mpl::int_<153>(), session, t); break;
        case 154: StubAccess().invoke(interface_, boost::mpl::int_<154>(), session, t); break;
        case 155: StubAccess().invoke(interface_, boost::mpl::int_<155>(), session, t); break;
        case 156: StubAccess().invoke(interface_, boost::mpl::int_<156>(), session, t); break;
        case 157: StubAccess().invoke(interface_, boost::mpl::int_<157>(), session, t); break;
        case 158: StubAccess().invoke(interface_, boost::mpl::int_<158>(), session, t); break;
        case 159: StubAccess().invoke(interface_, boost::mpl::int_<159>(), session, t); break;
        case 160: StubAccess().invoke(interface_, boost::mpl::int_<160>(), session, t); break;
        case 161: StubAccess().invoke(interface_, boost::mpl::int_<161>(), session, t); break;
        case 162: StubAccess().invoke(interface_, boost::mpl::int_<162>(), session, t); break;
        case 163: StubAccess().invoke(interface_, boost::mpl::int_<163>(), session, t); break;
        case 164: StubAccess().invoke(interface_, boost::mpl::int_<164>(), session, t); break;
        case 165: StubAccess().invoke(interface_, boost::mpl::int_<165>(), session, t); break;
        case 166: StubAccess().invoke(interface_, boost::mpl::int_<166>(), session, t); break;
        case 167: StubAccess().invoke(interface_, boost::mpl::int_<167>(), session, t); break;
        case 168: StubAccess().invoke(interface_, boost::mpl::int_<168>(), session, t); break;
        case 169: StubAccess().invoke(interface_, boost::mpl::int_<169>(), session, t); break;
        case 170: StubAccess().invoke(interface_, boost::mpl::int_<170>(), session, t); break;
        case 171: StubAccess().invoke(interface_, boost::mpl::int_<171>(), session, t); break;
        case 172: StubAccess().invoke(interface_, boost::mpl::int_<172>(), session, t); break;
        case 173: StubAccess().invoke(interface_, boost::mpl::int_<173>(), session, t); break;
        case 174: StubAccess().invoke(interface_, boost::mpl::int_<174>(), session, t); break;
        case 175: StubAccess().invoke(interface_, boost::mpl::int_<175>(), session, t); break;
        case 176: StubAccess().invoke(interface_, boost::mpl::int_<176>(), session, t); break;
        case 177: StubAccess().invoke(interface_, boost::mpl::int_<177>(), session, t); break;
        case 178: StubAccess().invoke(interface_, boost::mpl::int_<178>(), session, t); break;
        case 179: StubAccess().invoke(interface_, boost::mpl::int_<179>(), session, t); break;
        case 180: StubAccess().invoke(interface_, boost::mpl::int_<180>(), session, t); break;
        case 181: StubAccess().invoke(interface_, boost::mpl::int_<181>(), session, t); break;
        case 182: StubAccess().invoke(interface_, boost::mpl::int_<182>(), session, t); break;
        case 183: StubAccess().invoke(interface_, boost::mpl::int_<183>(), session, t); break;
        case 184: StubAccess().invoke(interface_, boost::mpl::int_<184>(), session, t); break;
        case 185: StubAccess().invoke(interface_, boost::mpl::int_<185>(), session, t); break;
        case 186: StubAccess().invoke(interface_, boost::mpl::int_<186>(), session, t); break;
        case 187: StubAccess().invoke(interface_, boost::mpl::int_<187>(), session, t); break;
        case 188: StubAccess().invoke(interface_, boost::mpl::int_<188>(), session, t); break;
        case 189: StubAccess().invoke(interface_, boost::mpl::int_<189>(), session, t); break;
        case 190: StubAccess().invoke(interface_, boost::mpl::int_<190>(), session, t); break;
        case 191: StubAccess().invoke(interface_, boost::mpl::int_<191>(), session, t); break;
        case 192: StubAccess().invoke(interface_, boost::mpl::int_<192>(), session, t); break;
        case 193: StubAccess().invoke(interface_, boost::mpl::int_<193>(), session, t); break;
        case 194: StubAccess().invoke(interface_, boost::mpl::int_<194>(), session, t); break;
        case 195: StubAccess().invoke(interface_, boost::mpl::int_<195>(), session, t); break;
        case 196: StubAccess().invoke(interface_, boost::mpl::int_<196>(), session, t); break;
        case 197: StubAccess().invoke(interface_, boost::mpl::int_<197>(), session, t); break;
        case 198: StubAccess().invoke(interface_, boost::mpl::int_<198>(), session, t); break;
        case 199: StubAccess().invoke(interface_, boost::mpl::int_<199>(), session, t); break;
        case 200: StubAccess().invoke(interface_, boost::mpl::int_<200>(), session, t); break;

        default: RCF_THROW(Exception(_RcfError_FnId(fnId)));
        }
    }

    template<typename InterfaceT, typename DerefPtrT>
    class InterfaceInvocator
    {
    public:
        InterfaceInvocator(InterfaceT &interface_, DerefPtrT derefPtr) :
            mInterface(interface_),
            mDerefPtr(derefPtr)
        {}

        void operator()(
            int                         fnId,
            RcfSession &                session)
        {
            invoke<InterfaceT>(mInterface, mDerefPtr->deref(), fnId, session);
        }

    private:
        InterfaceT &    mInterface;
        DerefPtrT       mDerefPtr;
    };

    template<typename InterfaceT, typename DerefPtrT>
    void registerInvokeFunctors(
        InterfaceT &                    interface_,
        InvokeFunctorMap &              invokeFunctorMap,
        DerefPtrT                       derefPtr)
    {
        // NB: same interface may occur more than once in the inheritance hierarchy of another interface, and in
        // that case, via overwriting, only one InterfaceInvocator is registered, so only the functions in one of the interfaces will ever be called.
        // But it doesn't matter, since even if an interface occurs several times in the inheritance hierarchy, each occurrence
        // of the interface will be bound to derefPtr in exactly the same way.

        typedef typename InterfaceT::Interface Interface;
        std::string interfaceName = ::RCF::getInterfaceName( (Interface *) NULL);

        invokeFunctorMap[ interfaceName ] =
            InterfaceInvocator<InterfaceT, DerefPtrT>(interface_, derefPtr);
    }

    class ServerBinding;

    typedef boost::shared_ptr<ServerBinding> ServerBindingPtr;

    class RCF_EXPORT ServerBinding
    {
    public:

        typedef boost::function<bool(int)> CbAccessControl;

        void setAccessControl(CbAccessControl cbAccessControl);

        template<typename InterfaceT, typename DerefPtrT>
        void registerInvokeFunctors(InterfaceT &interface_, DerefPtrT derefPtr)
        {
            StubAccess access;
            access.registerInvokeFunctors(
                interface_,
                mInvokeFunctorMap,
                derefPtr);
        }

    private:

        friend class RcfServer;
        friend class RcfSession;

        void invoke(
            const std::string &         subInterface,
            int                         fnId,
            RcfSession &                session);

    private:

        Mutex                           mMutex;

        // TODO: too much overhead per server stub?
        InvokeFunctorMap                mInvokeFunctorMap;

        CbAccessControl                 mCbAccessControl;
    };

    template<typename InterfaceT, typename ImplementationT, typename ImplementationPtrT>
    RcfClientPtr createServerStub(
        InterfaceT *,
        ImplementationT *,
        ImplementationPtrT px)
    {
        typedef typename InterfaceT::RcfClientT RcfClientT;
        return RcfClientPtr( new RcfClientT(
            ServerBindingPtr(new ServerBinding()),
            px,
            (boost::mpl::true_ *) NULL));
    }
    
} // namespace RCF

#endif // ! INCLUDE_RCF_SERVERSTUB_HPP
