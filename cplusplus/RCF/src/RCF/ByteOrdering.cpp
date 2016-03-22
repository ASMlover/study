
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

#include <boost/type_traits/is_same.hpp>

#include <RCF/ByteOrdering.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    enum ByteOrder
    {
        LittleEndian,
        BigEndian
    };

    // Compiler defines tell us what the local byte ordering is.

#if defined(__sparc__) || defined(__sparc)

    const ByteOrder MachineByteOrder = BigEndian;

#elif defined( __ppc__ )

    const ByteOrder MachineByteOrder = BigEndian;

#elif defined(__x86__) || defined(_M_IX86)

    const ByteOrder MachineByteOrder = LittleEndian;

#elif defined(__i386__) || defined(__i386) || defined(i386)

    const ByteOrder MachineByteOrder = LittleEndian;

#elif defined(__x86_64) || defined(__x86_64__)

    const ByteOrder MachineByteOrder = LittleEndian;

#elif defined(__amd64) || defined(__amd64__)

    const ByteOrder MachineByteOrder = LittleEndian;

#elif defined(_M_IA64) || defined(_M_AMD64) || defined(_M_X64)

    const ByteOrder MachineByteOrder = LittleEndian;

#elif defined(__arm__)

    const ByteOrder MachineByteOrder = LittleEndian;

#elif defined(__bfin__)

    const ByteOrder MachineByteOrder = LittleEndian;

#else

    // If you get an error here, uncomment the appropriate line.
    //const ByteOrder MachineByteOrder = BigEndian;
    //const ByteOrder MachineByteOrder = LittleEndian;

#error Unknown byte order on this platform!

#endif

    const ByteOrder NetworkByteOrder = LittleEndian;

    void swapBytes(char *b1, char *b2)
    {
        //*b2 ^= *b1;
        //*b1 ^= *b2;
        //*b2 ^= *b1;

        char temp = *b1;
        *b1 = *b2;
        *b2 = temp;
    }

    void reverseByteOrder(void *buffer, int width, int count)
    {
        RCF_ASSERT_GT(width , 0);
        RCF_ASSERT_GT(count , 0);
        if (width == 1) return;

        BOOST_STATIC_ASSERT( sizeof(char) == 1 );   
        char *chBuffer = static_cast<char *>(buffer);
        for (int i=0; i<count; i++)
        {
            for (int j=0;j<width/2;j++)
            {
                swapBytes(
                    chBuffer + i*width + j,
                    chBuffer + i*width + width - j - 1 );
            }
        }
    }

    void machineToNetworkOrder(void *buffer, int width, int count)
    {
        if (MachineByteOrder != NetworkByteOrder)
        {
            reverseByteOrder(buffer, width, count);
        }
    }

    void networkToMachineOrder(void *buffer, int width, int count)
    {
        if (MachineByteOrder != NetworkByteOrder)
        {
            reverseByteOrder(buffer, width, count);
        }
    }

    bool machineOrderEqualsNetworkOrder()
    {
        return MachineByteOrder == NetworkByteOrder;
    }

    bool isPlatformLittleEndian()
    {
        return MachineByteOrder == LittleEndian;
    }

} // namespace RCF
