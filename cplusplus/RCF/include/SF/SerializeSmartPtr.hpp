
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

#ifndef INCLUDE_SF_SERIALIZESMARTPTR_HPP
#define INCLUDE_SF_SERIALIZESMARTPTR_HPP

#include <SF/Archive.hpp>
#include <SF/Stream.hpp>

namespace SF {

    // 1. Non-ref counted smart pointer. SmartPtr<T> must support reset() and operator->().

    template<typename T, typename SmartPtrT>
    inline void serializeSimpleSmartPtr(SmartPtrT **ppt, SF::Archive &ar)
    {
        if (ar.isRead())
        {
            if (ar.isFlagSet(Archive::POINTER))
            {
                *ppt = new SmartPtrT();
            }
            T *pt = NULL;
            ar & pt;
            (**ppt).reset(pt);
        }
        else if (ar.isWrite())
        {
            T *pt = NULL;
            if (*ppt && (**ppt).get())
            {
                pt = (**ppt).operator->();
            }
            ar & pt;
        }
    }


#define SF_SERIALIZE_SIMPLE_SMARTPTR( SmartPtr )                                        \
    template<typename T>                                                                \
    inline bool invokeCustomSerializer(SmartPtr<T> **ppt, Archive &ar, int)             \
    {                                                                                   \
        serializeSimpleSmartPtr<T>(ppt, ar);                                            \
        return true;                                                                    \
    }

    // 2. Ref counted smart pointer. Must support operator=(), operator->(), and get().

    template<typename T, typename SmartPtrT>
    inline void serializeRefCountedSmartPtr(SmartPtrT **ppt, SF::Archive &ar)
    {
        if (ar.isRead())
        {
            if (ar.isFlagSet(Archive::POINTER))
            {
                *ppt = new SmartPtrT;
            }
            T *pt = NULL;
            ar & pt;

            ContextRead &ctx = ar.getIstream()->getTrackingContext();
            if (!ctx.getEnabled())
            {
                // No pointer tracking.
                **ppt = SmartPtrT(pt);
            }
            else
            {
                // Pointer tracking enabled, so some extra gymnastics involved.
                void *pv = NULL;
                if (pt && ctx.getEnabled() && ctx.query((void *)pt, typeid(SmartPtrT), pv))
                {
                    SmartPtrT *ps_prev = reinterpret_cast<SmartPtrT *>(pv);
                    **ppt = *ps_prev;
                }
                else if (pt)
                {
                    if (ctx.getEnabled())
                    {
                        ctx.add((void *)pt, typeid(SmartPtrT), *ppt);
                    }
                    **ppt = SmartPtrT(pt);
                }
            }
        }
        else /*if (ar.isWrite())*/
        {
            T *pt = NULL;
            if (*ppt)
            {
                pt = (**ppt).get();
            }
            ar & pt;
        }
    }

#define SF_SERIALIZE_REFCOUNTED_SMARTPTR( SmartPtr )                                    \
    template<typename T>                                                                \
    inline bool invokeCustomSerializer(SmartPtr<T> **ppt, Archive &ar, int)             \
    {                                                                                   \
        serializeRefCountedSmartPtr<T>(ppt, ar);                                        \
        return true;                                                                    \
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZERSMARTPTR_HPP
