#!/usr/bin/env python3

#
# Install directions:
# pip3 install mido python-rtmidi
#

import re
import time
import yaml
import sys
import os.path
import mido

CONTROL_NAME_RE = re.compile("^[a-z_\d]+$")

def main():
    print("Select device")
    port_name = input_port_name()

    port = mido.open_input(port_name)

    while True:
        while port.poll():
            pass

        for msg in port:
            print(repr(msg))


def input_port_name():
    names = mido.get_input_names()
    print("Detected the following midi device ports")

    for idx, name in enumerate(names):
        print("   {})    {}".format(idx, name))

    idx = input_int("Enter the number of the desired device", range(len(names)))

    print("You chose: " + names[idx])

    return names[idx]


def input_int(msg, acceptable=None):
    acceptable_s =  ", ".join(str(v) for v in acceptable)
    num_raw = input("{} (acceptable: {}): ".format(msg, acceptable_s))
    try:
        num = int(num_raw)
    except ValueError:
        print("Input not a number! Try again.")
        return input_int(msg)

    if acceptable is not None and num not in acceptable:
        print("Input not an acceptable number! Try again.")
        return input_int(msg, acceptable)

    return num


if __name__ == "__main__":
    main()
