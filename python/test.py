#!/usr/bin/python3

import PastecLib

c = PastecLib.PastecConnection()
c.connect()
#c.indexImageFile(1, "/home/magsoft/Bureau/testPastec/starTrek.jpg")
#c.indexImageFile(2, "/home/magsoft/Bureau/testPastec/test.jpg")
fd = open("/home/magsoft/Bureau/testPastec/test.jpg", "rb")
data = fd.read(1024*1024)
print(c.imageQuery(data))

c.writeIndex()
