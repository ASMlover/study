/*    Copyright 2012 10gen Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * Implementation of the AtomicIntrinsics<T>::* operations for IA-32 and AMD64 systems using a
 * GCC-compatible compiler toolchain.
 */

#pragma once

#include <boost/utility.hpp>

namespace mongo {

    
    /**
     * Instantiation of AtomicIntrinsics<> for all word types T where sizeof<T> <= sizeof(void *).
     *
     * On 32-bit systems, this handles 8-, 16- and 32-bit word types.  On 64-bit systems,
     * it handles 8-, 16, 32- and 64-bit types.
     */
    template <typename T, typename IsTLarge=void>
    class AtomicIntrinsics {
    public:

        static T compareAndSwap(volatile T* dest, T expected, T newValue){
            
            T result = *dest;
            if ( result == expected)
                 *dest = newValue;
            return result;
        }

        static T swap(volatile T* dest, T newValue) {

            T result = * dest;
            *dest = newValue;
            return result;
        }

        static T load(volatile const T* value) {
            return *value;
        }

        static T loadRelaxed(volatile const T* value) {
            return *value;
        }

        static void store(volatile T* dest, T newValue) {
            *dest = newValue;
        }

        static T fetchAndAdd(volatile T* dest, T increment) {
            T result = * dest;
            * dest += increment;
            return  result;
        }

    private:
        AtomicIntrinsics();
        ~AtomicIntrinsics();
    };

}  // namespace mongo
