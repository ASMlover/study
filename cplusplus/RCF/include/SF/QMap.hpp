
#ifndef INCLUDE_SF_QMAP_HPP
#define INCLUDE_SF_QMAP_HPP

#include <QMap>

#include <SF/SerializeStl.hpp>

namespace SF {

    // QMap
    template<typename K, typename T>
    inline void serialize_vc6(Archive &ar, QMap<K,T> &t, const unsigned int)
    {
        typedef typename QMap<K,T>::iterator Iterator;
        typedef typename QMap<K,T>::key_type Key;
        typedef typename QMap<K,T>::mapped_type Value;

        if (ar.isRead())
        {
          t.clear();
          boost::uint32_t count = 0;
          ar & count;

          for (boost::uint32_t i=0; i<count; i++)
          {
            Key key;
            ar & key;
            Value value;
            ar & value;
            t.insert(key, value);
          }
        }
        else if (ar.isWrite())
        {
          boost::uint32_t count = static_cast<boost::uint32_t>(t.size());
          ar & count;
          Iterator it = t.begin();
          for (boost::uint32_t i=0; i<count; i++)
          {
            ar & it.key();
            ar & it.value();
            it++;
          }
        }
    }

}

#endif // ! INCLUDE_SF_QMAP_HPP
