#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2024 ASMlover. All rights reserved.
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
import os
from typing import List

PREAMBLE = """// Generated automatically from `{0}`. Do not edit.
static const char* {1}ModuleSource =
{2};
"""

def wren_to_c_string(input_path: str, wren_source_lines: List[str], module: str) -> str:
	wren_source = ""
	for line in wren_source_lines:
		line = line.decode("utf-8")
		line = line.replace('"', "\\\"")
		line = line.replace("\n", "\\n\"")
		if wren_source:
			wren_source += "\n"
		wren_source += '"' + line

	return PREAMBLE.format(input_path, module, wren_source)

def main() -> None:
	parser = argparse.ArgumentParser(description="Convert a Wren library to a C string literal.")
	parser.add_argument("-o", "--output", help="The output file to write.")
	parser.add_argument("-i", "--input", help="The source wren file")
	parser.add_argument("--module", help="The module name to convert, default by filename,", default='')

	args = parser.parse_args()

	with open(args.input, "rb") as fp:
		wren_source_lines = fp.readlines()

	if args.module:
		module = args.module
	else:
		module = os.path.splitext(os.path.basename(args.input))[0]
	c_source = wren_to_c_string(args.input, wren_source_lines, module)

	with open(args.output, "w") as fp:
		fp.write(c_source)

if __name__ == "__main__":
	main()
