#!/usr/bin/python3

import PastecLib

pastec = PastecLib.PastecConnection()

pastec.connect()
pastec.initBuildForwardIndex()
pastec.indexImageFile(42, "test.jpg")

pastec.buildBackwardIndex()

pastec.close()

