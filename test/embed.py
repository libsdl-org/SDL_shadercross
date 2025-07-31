#!/usr/bin/env python

import argparse
import itertools
import pathlib
import re
import sys


def main():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument("-i", type=pathlib.Path, dest="input",  help="Input file")
    parser.add_argument("-o", type=pathlib.Path, dest="output", help="Output file")
    parser.add_argument("-z", action="store_true", dest="is_text", help="Input is text")
    parser.add_argument("-u", dest="identifier", help="Identifier (optional)")
    args = parser.parse_args()

    if args.identifier:
        varname = args.identifier
    elif args.input:
        varname, _ = re.subn("[ .]", "_", args.input.name)
    else:
        parser.error("-u is required when using stdin")

    if not varname.isidentifier():
        parser.error(f"Invalid identifier: {varname}. Change input file or use/fix -u.")

    with (args.input.open("rb") if args.input else sys.stdin.buffer) as f_in:
        data_in = f_in.read()

    with (args.output.open("w", encoding="UTF-8", newline="") if args.output else sys.stdout) as f_out:
        f_out.write("#pragma once\n")
        f_out.write("\n")
        f_out.write("// This file is auto-generated. Manual modifications will be lost.\n")
        f_out.write("\n")
        f_out.write("#include <SDL3/SDL_stdinc.h>\n")
        f_out.write("\n")
        f_out.write(f"static const Uint8 {varname}[{len(data_in)+1}] = {{\n")
        for batch in itertools.batched(data_in, 16):
            f_out.write("  ")
            f_out.write(" ".join(f"0x{b:02x},"for b in batch))
            f_out.write("\n")
        if args.is_text:
            f_out.write("  0x00, /* null-terminator */\n")
        f_out.write(f"}};\n")


if __name__ == "__main__":
    raise SystemExit(main())
