
#ifndef INCLUDE_SF_QVARIANT_HPP
#define INCLUDE_SF_QVARIANT_HPP

#include <boost/config.hpp>

#include <SF/Archive.hpp>
#include <SF/Stream.hpp>

#include <QVariant>
#include <QByteArray>
#include <SF/QByteArray.hpp>


namespace SF {
                
    // QVariant
    inline void serialize(SF::Archive & ar, QVariant & qobj)
    {
      SERIALIZE_QT_OBJECT
    }    
        
    
} // namespace SF

#endif // ! INCLUDE_SF_QVARIANT_HPP
