#!/usr/bin/python3

import PastecLib
import sys

if len(sys.argv) != 2:
    print("Usage: startSearch serverHost")

pastec = PastecLib.PastecConnection()

pastec.connect(sys.argv[1])
pastec.initSearch()

pastec.close()

