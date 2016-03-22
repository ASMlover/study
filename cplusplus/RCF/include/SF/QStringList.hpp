
#ifndef INCLUDE_SF_QSTRINGLIST_HPP
#define INCLUDE_SF_QSTRINGLIST_HPP

#include <QStringList>

#include <SF/SerializeStl.hpp>

namespace SF {

    // QStringList
    inline void serialize_vc6(SF::Archive &ar, QStringList &t, const unsigned int)
    {
        serializeStlContainer<PushBackSemantics, NoReserveSemantics>(ar, t);
    }

} // namespace SF

#endif // ! INCLUDE_SF_QSTRINGLIST_HPP
