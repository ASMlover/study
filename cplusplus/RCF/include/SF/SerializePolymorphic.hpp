
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

#ifndef INCLUDE_SF_SERIALIZEPOLYMORPHIC_HPP
#define INCLUDE_SF_SERIALIZEPOLYMORPHIC_HPP

namespace SF {

    class Archive;

    class I_SerializerPolymorphic
    {
    public:
        virtual ~I_SerializerPolymorphic() {}
        virtual bool invoke(void **ppvb, Archive &ar) = 0;
    };

    template<typename Base, typename Derived>
    class SerializerPolymorphic : public I_SerializerPolymorphic
    {
    public:

        SerializerPolymorphic() 
        {}

        virtual bool invoke(void **ppvb, Archive &ar);
    };

}

#include <SF/Archive.hpp>
#include <SF/Serializer.hpp>

namespace SF {

    template<typename Base, typename Derived>
    bool SerializerPolymorphic<Base,Derived>::invoke(void **ppvb, Archive &ar)
    {
        if (ar.isWrite())
        {
            Base *pb = reinterpret_cast<Base *>(*ppvb);
            Derived *pd = static_cast<Derived *>(pb);
            ar & pd;
        }
        else if (ar.isRead())
        {
            if (ar.isFlagSet(Archive::POINTER))
            {
                Derived *pd = NULL;
                ar & pd;
                Base *pb = static_cast<Base *>(pd);
                *ppvb = pb;
            }
            else
            {
                Base *pb = reinterpret_cast<Base *>(*ppvb);
                Derived *pd = static_cast<Derived *>(pb);
                ar & *pd;
            }
        }
        return true;
    }

}

#endif // ! INCLUDE_SF_SERIALIZEPOLYMORPHIC_HPP
