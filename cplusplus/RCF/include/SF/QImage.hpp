
#ifndef INCLUDE_SF_QIMAGE_HPP
#define INCLUDE_SF_QIMAGE_HPP

#include <boost/config.hpp>

#include <SF/Archive.hpp>
#include <SF/Stream.hpp>

#include <QtGui/QImage>
#include <QByteArray>
#include <SF/QByteArray.hpp>


namespace SF {
                
    // QImage
    inline void serialize(SF::Archive & ar, QImage & qobj)
    {
      SERIALIZE_QT_OBJECT
    }    
        
    
} // namespace SF

#endif // ! INCLUDE_SF_QIMAGE_HPP
