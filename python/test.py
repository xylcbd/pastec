#!/usr/bin/python3

import PastecLib

pastec = PastecLib.PastecConnection()

pastec.connect()
pastec.setIndexMode()
pastec.indexImageFile(42, "test.jpg")

pastec.close()

