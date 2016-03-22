
#ifndef INCLUDE_SF_QBYTEARRAY_HPP
#define INCLUDE_SF_QBYTEARRAY_HPP

#include <QDataStream>
#include <QByteArray>

#include <boost/config.hpp>

#include <SF/Archive.hpp>
#include <SF/Stream.hpp>

namespace SF {

    // QByteArray    
    inline void serializeQByteArray(SF::Archive & ar, QByteArray & qba)
    {
        if (ar.isRead())
        {
            boost::uint32_t count = 0;
            ar & count;
                
            qba.resize(count);
                        
            if (count) 
            {
                SF::IStream &is = *ar.getIstream();            

                // Size field is verified, so read everything in one go. 
                RCF_VERIFY(
                    is.read((char *)qba.data(), count) == count,
                    RCF::Exception(RCF::_SfError_ReadFailure()))
                    (count);
            }
        }
        else if (ar.isWrite())
        {
            boost::uint32_t count = static_cast<boost::uint32_t >(qba.size());
            ar & count;
            ar.getOstream()->writeRaw(qba.constData(), count);
        }

    }

    // QByteArray
    inline void serialize_vc6(SF::Archive & ar, QByteArray & qba, const unsigned int)
    {
        serializeQByteArray(ar, qba);
    }
    
    
    #define SERIALIZE_QT_OBJECT                         \
      QByteArray data;                                  \
      if (ar.isRead())                                  \
      {                                                 \
        serializeQByteArray(ar, data);                  \
        QDataStream qdsi(data); /*QIODevice::ReadOnly*/ \
        qdsi >> qobj;                                   \
      }                                                 \
      else if (ar.isWrite())                            \
      {                                                 \
        QDataStream qdso(&data, QIODevice::ReadWrite);  \
        qdso << qobj;                                   \
        serializeQByteArray(ar, data);                  \
      }



} // namespace SF

#endif // ! INCLUDE_SF_QBYTEARRAY_HPP
