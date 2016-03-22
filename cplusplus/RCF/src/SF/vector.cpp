
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

#include <SF/vector.hpp>

#include <boost/cstdint.hpp>
#include <boost/mpl/assert.hpp>

#include <RCF/Export.hpp>
#include <SF/Stream.hpp>
#include <SF/Tools.hpp>
#include <SF/bitset.hpp>

namespace SF {

    void serializeVectorFastImpl(
        SF::Archive &           ar,
        I_VecWrapper &          vec)
    {
        if (ar.isRead())
        {
            boost::uint32_t count = 0;
            ar & count;

            if (count)
            {
                SF::IStream &is = *ar.getIstream();

                vec.resize(0);

                std::size_t minSerializedLength = vec.sizeofElement();
                if (ar.verifyAgainstArchiveSize(count*minSerializedLength))
                {
                    // Size field is verified, so read everything in one go.
                    vec.resize(count);

                    boost::uint32_t bytesToRead = count * vec.sizeofElement();

                    boost::uint32_t bytesActuallyRead = is.read( 
                        vec.addressOfElement(0),
                        bytesToRead);

                    RCF_VERIFY(
                        bytesActuallyRead == bytesToRead,
                        RCF::Exception(RCF::_SfError_ReadFailure()))
                        (bytesActuallyRead)(bytesToRead)(count);

                    // Byte ordering.
                    if (ar.getRuntimeVersion() >= 8)
                    {
                        RCF::networkToMachineOrder(
                            vec.addressOfElement(0), 
                            static_cast<int>(vec.sizeofElement()), 
                            static_cast<int>(vec.size()));
                    }
                }
                else
                {
                    // Size field not verified, so read in chunks.
                    boost::uint32_t elementsRemaining = count;
                    
                    while (elementsRemaining)
                    {
                        const boost::uint32_t ElementsMax = 50*1024;
                        boost::uint32_t elementsRead = count - elementsRemaining;
                        boost::uint32_t elementsToRead = RCF_MIN(ElementsMax, elementsRemaining);
                        boost::uint32_t bytesToRead = elementsToRead*vec.sizeofElement();
                        vec.resize( vec.size() + elementsToRead);

                        boost::uint32_t bytesRead = is.read( 
                            vec.addressOfElement(elementsRead), 
                            bytesToRead);

                        RCF_VERIFY(
                            bytesRead == bytesToRead,
                            RCF::Exception(RCF::_SfError_ReadFailure()))
                            (bytesRead)(bytesToRead)(ElementsMax)(count);

                        elementsRemaining -= elementsToRead;
                    }

                    // Byte ordering.
                    if (ar.getRuntimeVersion() >= 8)
                    {
                        RCF::networkToMachineOrder(
                            vec.addressOfElement(0), 
                            static_cast<int>(vec.sizeofElement()), 
                            static_cast<int>(vec.size()));
                    }
                }
            }
        }
        else if (ar.isWrite())
        {
            boost::uint32_t count = static_cast<boost::uint32_t>(vec.size());
            ar & count;
            if (count)
            {
                boost::uint32_t totalBytesToWrite = count * vec.sizeofElement();

                if (RCF::machineOrderEqualsNetworkOrder())
                {
                    // Don't need reordering, so write everything in one go.
                    ar.getOstream()->writeRaw(vec.addressOfElement(0), totalBytesToWrite);
                }
                else if (ar.getRuntimeVersion() < 8)
                {
                    // Don't need reordering, so write everything in one go.
                    ar.getOstream()->writeRaw(vec.addressOfElement(0), totalBytesToWrite);
                }
                else
                {
                    // Reordering needed, so we go through a temporary buffer.
                    boost::uint32_t elementsRemaining = count;
                    const boost::uint32_t BufferSize = 100*1024;

                    const boost::uint32_t ElementsMax = BufferSize / vec.sizeofElement();

                    char Buffer[BufferSize];
                    while (elementsRemaining)
                    {
                        boost::uint32_t elementsWritten = count - elementsRemaining;
                        boost::uint32_t elementsToWrite = RCF_MIN(ElementsMax, elementsRemaining);
                        boost::uint32_t bytesToWrite = elementsToWrite*vec.sizeofElement();
                        
                        memcpy( (char *) &Buffer[0], vec.addressOfElement(elementsWritten), bytesToWrite);
                        RCF::machineToNetworkOrder( &Buffer[0], vec.sizeofElement(), elementsToWrite);
                        ar.getOstream()->writeRaw( (char *) &Buffer[0], bytesToWrite);
                        elementsRemaining -= elementsToWrite;
                    }
                }
            }
        }
    }


    class VectorBoolWrapper : public I_BitsetWrapper
    {
    public:
        VectorBoolWrapper(std::vector<bool> & bits) : mBits(bits)
        {
        }

        virtual std::size_t size()
        {
            return mBits.size();
        }

        virtual void resize(std::size_t newSize)
        {
            mBits.resize(newSize);
        }

        virtual void setBit(std::size_t idx, bool newValue)
        {
            mBits[idx] = newValue;
        }

        virtual bool getBit(std::size_t idx)
        {
            return mBits[idx];
        }

    private:

        std::vector<bool> & mBits;
    };

    RCF_EXPORT void serialize(SF::Archive & ar, std::vector<bool> & bits)
    {
        VectorBoolWrapper wrapper(bits);
        serializeBitset(ar, wrapper);
    }

} // namespace SF
