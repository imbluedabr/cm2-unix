#!/bin/env python3

import sys
import base64, zlib

def list_to_huge_string(data):
    data = bytearray(data)

    while len(data) % 4 != 0:
        data.append(0)

    word = bytearray()

    for i in range(0, len(data)):
        word.append(data[i])
        word.append(0)

    compressed = zlib.compress(word, level=2, wbits=-15)

    encoded = base64.b64encode(compressed).decode('utf-8').strip('=')

    return encoded


with open(sys.argv[1], 'rb') as binary:
    mem = list_to_huge_string(binary.read())
    print(mem)
    print('\n')
