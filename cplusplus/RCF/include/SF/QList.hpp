
#ifndef INCLUDE_SF_QLIST_HPP
#define INCLUDE_SF_QLIST_HPP

#include <QList>

#include <SF/SerializeStl.hpp>

namespace SF {

    // QList
    template<typename T>
    inline void serialize_vc6(SF::Archive &ar, QList<T> &t, const unsigned int)
    {
        serializeStlContainer<PushBackSemantics, NoReserveSemantics>(ar, t);
    }

} // namespace SF

#endif // ! INCLUDE_SF_QLIST_HPP
