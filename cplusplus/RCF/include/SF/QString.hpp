
#ifndef INCLUDE_SF_QSTRING_HPP
#define INCLUDE_SF_QSTRING_HPP

#include <QString>

#include <boost/config.hpp>

#include <SF/Archive.hpp>
#include <SF/Stream.hpp>

#include <SF/QByteArray.hpp>

namespace SF {

    // QString    
    inline void serialize(SF::Archive & ar, QString & qobj)
    {
        SERIALIZE_QT_OBJECT
    }    

} // namespace SF

#endif // ! INCLUDE_SF_QSTRING_HPP
