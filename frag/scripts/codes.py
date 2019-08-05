#!/usr/bin/env python3

import sys

print(repr([int(x) / 255.0 for x in sys.argv[1:]]))
