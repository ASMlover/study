#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2021 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materialsprovided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import argparse
import asyncio
import functools
import json
import os
from typing import List, Tuple

from pymongo import MongoClient
from utils import util


@util.noexcept
def get_mongo_url(config: str) -> Tuple[int, str, str]:
    with open(config, 'rb') as fp:
        json_dict = json.load(fp)
    return json_dict['hostnum'], json_dict['uri'], json_dict['dbname']

async def find_worker(col, query_nids: List[int]) -> List[Tuple[int, str]]:
    found_aids = []
    for doc in col.find({'nid': {'$in': query_nids}}, {'aid':1, 'nid':1, 'name':1}):
        found_aids.append((doc['nid'], doc['aid'].decode('utf-8'), doc['name'].decode('utf-8')))
    return found_aids

async def find_aids(config: str, nid: int = 0, nidfile: str = '', split_count: int = 1000) -> None:
    hostnum, uri, dbname = get_mongo_url(config)
    client = MongoClient(uri)
    col = client[dbname].Avatar

    query_nids, task_list = [], []
    if nidfile and split_count:
        with open(nidfile, 'rb') as fp:
            count = 0
            for nid_str in fp.readlines():
                query_nids.append(int(nid_str))
                count += 1

                if count % split_count == 0:
                    task_list.append(asyncio.create_task(find_worker(col, query_nids)))
                    query_nids = []
        if query_nids:
            task_list.append(asyncio.create_task(find_worker(col, query_nids)))
    else:
        task_list.append(asyncio.create_task(find_worker(col, [nid])))

    results = await asyncio.gather(*task_list)
    for found_aids in results:
        for nid, aid, name in found_aids:
            print(f"nid:{nid} | aid:{aid} | name:{name}")

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', type=str, help='Configure file for MongoDB', default='db.conf')
    parser.add_argument('--nid', type=int, help='Specified Avatar {NID}', default=0)
    parser.add_argument('--nidfile', type=str, help='Specified Avatar {NID} file', default='')
    parser.add_argument('--split-count', type=int, help='{NID} file split count', default=1000)
    args = parser.parse_args()

    if args.nidfile:
        if args.split_count <= 0:
            raise SystemError('Please input valid split count')

    asyncio.run(find_aids(args.config, args.nid, args.nidfile, args.split_count))

if __name__ == '__main__':
    main()
