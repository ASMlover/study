//
//  bson-assert.h
//  bson
//
//  Created by yemingjiang on 13-5-16.
//
//

#pragma once

#include <cstdlib>
#include <memory>
#include <iostream>
#include <sstream>

#include "mongo/platform/compiler.h"

namespace bson {
    
    using std::string;
    using std::stringstream;
    
    class assertion : public std::exception {
    public:
        assertion( unsigned u , const std::string& s )
        : id( u ) , msg( s ) {
            std::stringstream ss;
            ss << "BsonAssertion id: " << u << " " << s;
            full = ss.str();
        }
        
        virtual ~assertion() throw() {}
        
        virtual const char* what() const throw() { return full.c_str(); }
        
        unsigned id;
        std::string msg;
        std::string full;
    };
}

namespace mongo {
#if !defined(verify)
    inline void verify(bool expr) {
        if(!expr) {
            throw bson::assertion( 0 , "assertion failure in bson library" );
        }
    }
#endif
#if !defined(uassert)
    MONGO_COMPILER_NORETURN inline void uasserted(int msgid, const std::string &s) {
        throw bson::assertion( msgid , s );
    }
    
    inline void uassert(unsigned msgid, const std::string& msg, bool expr) {
        if( !expr )
            uasserted( msgid , msg );
    }
    MONGO_COMPILER_NORETURN inline void msgasserted(int msgid, const char *msg) {
        throw bson::assertion( msgid , msg );
    }
    MONGO_COMPILER_NORETURN inline void msgasserted(int msgid, const std::string &msg) {
        msgasserted(msgid, msg.c_str());
    }
    inline void massert(int msgid, const std::string& msg, bool expr) {
        if(!expr) {
            std::cout << "assertion failure in bson library: " << msgid << ' ' << msg << std::endl;
            throw bson::assertion( msgid , msg );
        }
    }
#endif
}

