
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

#ifndef INCLUDE_SF_I_STREAM_HPP
#define INCLUDE_SF_I_STREAM_HPP

#include <RCF/Export.hpp>

#include <SF/PortableTypes.hpp>
#include <SF/Tools.hpp>

namespace SF {

    //*************************************************************************
    // Stream interfaces

    class DataPtr;
    class Node;

    typedef std::pair<void *, const std::type_info *> ObjectId;

    //--------------------------------------------------------------------------
    // I_Encoding

    class RCF_EXPORT I_Encoding
    {
    public:
        virtual ~I_Encoding() 
        {}

#define SF_DECLARE_GETCOUNT_PV(T)                   \
        virtual UInt32 getCount(                    \
            DataPtr &               data,           \
            T *                         ) = 0;

#define SF_DECLARE_TODATA_PV(T)                     \
        virtual void toData(                        \
            DataPtr &               data,           \
            T *                     pObj,           \
            int                     nCount) = 0;

#define SF_DECLARE_TOOBJECT_PV(T)                   \
        virtual void toObject(                      \
            DataPtr &               data,           \
            T *                     pObj,           \
            int                     nCount) = 0;

        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_GETCOUNT_PV )
        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_TODATA_PV )
        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_TOOBJECT_PV )

#undef SF_DECLARE_GETCOUNT_PV
#undef SF_DECLARE_TODATA_PV
#undef SF_DECLARE_TOOBJECT_PV

    };

    //--------------------------------------------------------------------------
    // EncodingTest, EncodingBinaryNative, EncodingBinaryPortable

#define SF_DECLARE_GETCOUNT(T)                      \
        virtual UInt32 getCount(                    \
            DataPtr &               data,           \
            T *                         );

#define SF_DECLARE_TODATA(T)                        \
        virtual void toData(                        \
            DataPtr &               data,           \
            T *                     pObj,           \
            int                     nCount);

#define SF_DECLARE_TOOBJECT(T)                      \
        virtual void toObject(                      \
            DataPtr &               data,           \
            T *                     pObj,           \
            int                     nCount);


    class RCF_EXPORT EncodingText : public I_Encoding
    {
    public:

        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_GETCOUNT )
        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_TODATA )
        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_TOOBJECT )

    };

    class RCF_EXPORT EncodingBinaryNative : public I_Encoding
    {
    public:

        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_GETCOUNT )
        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_TODATA )
        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_TOOBJECT )

    };

    class RCF_EXPORT EncodingBinaryPortable : public I_Encoding
    {
    public:

        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_GETCOUNT )
        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_TODATA )
        SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_DECLARE_TOOBJECT )

    };

#undef SF_DECLARE_GETCOUNT
#undef SF_DECLARE_TODATA
#undef SF_DECLARE_TOOBJECT

}

#endif // ! INCLUDE_SF_I_STREAM_HPP
