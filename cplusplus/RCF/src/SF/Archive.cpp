
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

#include <SF/Archive.hpp>

#include <SF/Stream.hpp>

namespace SF {

    Archive::Archive(Direction dir, IStream *stream) :
        mDir(dir),
        mIstream(stream),
        mOstream(),
        mLabel(),
        mFlags()
    {
    }

    Archive::Archive(Direction dir, OStream *stream) :
        mDir(dir),
        mIstream(),
        mOstream(stream),
        mLabel(),
        mFlags()
    {
    }

    Archive & Archive::operator&(Flag flag)
    {
        setFlag(flag);
        return *this;
    }

    bool Archive::isRead() const
    {
        return mDir == READ;
    }

    bool Archive::isWrite() const
    {
        return mDir == WRITE;
    }

    IStream *Archive::getIstream() const
    {
        return mIstream;
    }

    OStream *Archive::getOstream() const
    {
        return mOstream;
    }

    bool Archive::isFlagSet(Flag flag) const
    {
        return mFlags & flag ? true : false;
    }

    void Archive::setFlag(Flag flag, bool bEnable)
    {
        if (bEnable)
        {
            mFlags |= flag;
        }
        else
        {
            mFlags &= ~flag;
        }
    }

    void Archive::clearFlag(Flag flag)
    {
        setFlag(flag, false);
    }

    void Archive::clearState()
    {
        mLabel = "";
        mFlags = 0;
    }

    DataPtr & Archive::getLabel()
    {
        return mLabel;
    }

    bool Archive::verifyAgainstArchiveSize(std::size_t bytesToRead)
    {
        if (mIstream)
        {
            return mIstream->verifyAgainstArchiveSize(bytesToRead);
        }
        else
        {
            return false;
        }
    }

    int Archive::getRuntimeVersion()
    {
        if (mIstream)
        {
            return mIstream->getRuntimeVersion();
        }
        else
        {
            return mOstream->getRuntimeVersion();
        }
    }

    int Archive::getArchiveVersion()
    {
        if (mIstream)
        {
            return mIstream->getArchiveVersion();
        }
        else
        {
            return mOstream->getArchiveVersion();
        }
    }

} // namespace SF
