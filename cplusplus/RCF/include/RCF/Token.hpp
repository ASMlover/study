
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

#ifndef INCLUDE_RCF_TOKEN_HPP
#define INCLUDE_RCF_TOKEN_HPP

#include <vector>

#include <boost/noncopyable.hpp>

#include <RCF/Export.hpp>
#include <RCF/Config.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/TypeTraits.hpp>

namespace SF {

    class Archive;

}

namespace RCF {

    class MemOstream;

    class RCF_EXPORT Token
    {
    public:
        Token();
        Token(int id);
        int getId() const;
        friend bool operator<(const Token &lhs, const Token &rhs);
        friend bool operator==(const Token &lhs, const Token &rhs);
        friend bool operator!=(const Token &lhs, const Token &rhs);

#if RCF_FEATURE_SF==1

        void serialize(SF::Archive &ar);

#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1

        template<typename Archive> 
        void serialize(Archive &ar, const unsigned int)
        {
            ar & boost::serialization::make_nvp("Id", mId);
        }

#endif
       
        friend RCF_EXPORT RCF::MemOstream &operator<<(RCF::MemOstream &os, const Token &token);

    private:
        int mId;
    };

    class TokenFactory : boost::noncopyable
    {
    public:
                                    TokenFactory(int tokenCount);

        bool                        requestToken(Token &token);
        void                        returnToken(const Token &token);
        const std::vector<Token> &  getTokenSpace();
        std::size_t                 getAvailableTokenCount();
        bool                        isAvailable(const Token & token);

    private:
        std::vector<Token>          mTokenSpace;
        std::vector<Token>          mAvailableTokens;
        ReadWriteMutex              mMutex;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_TOKEN_HPP
