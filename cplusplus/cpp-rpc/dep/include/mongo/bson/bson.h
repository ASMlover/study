/** @file bson.h

    Main bson include file for mongodb c++ clients. MongoDB includes ../db/jsobj.h instead. 
    This file, however, pulls in much less code / dependencies.

    @see bsondemo
*/

/*
 *    Copyright 2009 10gen Inc.
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
   Main include file for C++ BSON module when using standalone (sans MongoDB client).

   "BSON" stands for "binary JSON" -- ie a binary way to represent objects that would be
   represented in JSON (plus a few extensions useful for databases & other languages).

   http://www.bsonspec.org/
*/

#pragma once

#if defined(MONGO_EXPOSE_MACROS)
#error this header is for client programs, not the mongo database itself. include jsobj.h instead.
/* because we define simplistic assert helpers here that don't pull in a bunch of util -- so that
   BSON can be used header only.
   */
#endif

#include <cstdlib>
#include <memory>
#include <iostream>
#include <sstream>




#include "mongo/bson/bson-assert.h"
#include "mongo/bson/bsonelement.h"
#include "mongo/bson/bsonobj.h"
#include "mongo/bson/bsonobjbuilder.h"
#include "mongo/bson/bsonobjiterator.h"
#include "mongo/bson/bsontypes.h"
#include "mongo/bson/bson-inl.h"
#include "mongo/bson/oid.h"
#include "mongo/bson/util/builder.h"
