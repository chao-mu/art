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
    if len(sys.argv) != 2:
        sys.exit("Expected one and only one argument which is the output path for the midi module config")

    path = sys.argv[1]

    config = {}
    print("## Let's onboard a new midi device! ##")
    print("At any time you may press control+c to quit, like anything in a terminal")

    output_step(1, "Select device")
    port_name = input_port_name()

    port = mido.open_input(port_name)

    output_step(2, "Test connectivity")
    input_test_message(port)

    output_step(3, "Enter regex to match port name")
    config["regex"] = input_port_regex(port_name)

    output_step(4, "Learn controls")
    config["mappings"] = input_mappings(port)

    print()
    print("Finished! Saving...")
    with open(path, "w") as f:
        yaml.dump(config, f, default_flow_style=False)

def output_step(i, desc):
    print()
    print("# Step {}: {}".format(i, desc))

def input_port_regex(name):
    regex_raw = input("Enter a regex or substring that uniquely matches the device's port name: ")
    try:
        regex = re.compile(regex_raw)
    except re.error:
        print("Invalid regular expression. Please try again.")
        return input_port_regex(name)

    if not regex.match(name):
        print("Regex would not match port name. Please try again.")
        return input_port_regex(name)

    return regex_raw

def input_mappings(port):
    print("When prompted, enter a control name. Then, move/press the control on the device that it should be mapped to.")
    mappings = {}
    another = True
    while another:
        name = input_control_name("Name of the next control to map")
        print("Poke it!")
        msg = input_message(port)
        print("Message recceived: {}".format(msg))
        mappings[name] = make_mapping(msg)
        print("Mapping added!")
        another = input_yn("Add another?")

    return mappings

def input_test_message(port):
    print("Press a button or turn a fader/knob to see if we selected the correct device")
    print("If nothing happens, then you may have selected the wrong device. control+c to quit.")
    msg = input_message(port)
    print("Message detected!")
    return msg

def input_message(port):
    # Drain
    while port.poll():
        pass

    for msg in port:
        if msg.type in ("note_on", "control_change"):
            return msg

def input_port_name():
    names = mido.get_input_names()
    print("Detected the following midi device ports")

    for idx, name in enumerate(names):
        print("   {})    {}".format(idx, name))

    idx = input_int("Enter the number of the desired device", range(len(names)))

    print("You chose: " + names[idx])

    return names[idx]

def input_control_name(msg):
    name = input(msg + " (acceptable names are short, all lowercase, and have underscores instead of spaces): ")
    if CONTROL_NAME_RE.match(name) is None:
        print("Sorry, that was not a valid name. Please try again.")
        return input_control_name(msg)

    return name

def input_yn(msg):
    resp = input(msg + " (acceptable: [y], n): ")
    if resp == "y" or resp == "":
        return True
    elif resp == "n":
        return False
    else:
        print("Sorry, that was not a valid response. Please try again.")
        return input_yn(msg)

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

def make_mapping(msg):
    mapping = {}

    if msg.type == "note_on":
        mapping["type"] = "button"
        mapping["function"] = msg.note
    elif msg.type == "control_change":
        mapping["type"] = "fader"
        mapping["function"] = msg.control

    mapping["channel"] = msg.channel
    mapping["low"] = 0
    mapping["high"] = 127

    return mapping

if __name__ == "__main__":
    main()
