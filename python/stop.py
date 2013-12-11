#!/usr/bin/python3

import PastecLib

pastec = PastecLib.PastecConnection()

pastec.connect()
pastec.stopServer()

pastec.close()
