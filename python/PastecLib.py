#!/usr/bin/python3

import socket
import struct

from PastecLibMessages import Reply, Query


class PastecException(Exception):
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return repr(self.msg)


class PastecConnection:

    def sendData(self, data):
        totalsent = 0
        while totalsent < len(data):
            sent = self.sock.send(data[totalsent:])
            if sent == 0:
                raise RuntimeError("Socket connection broken")
            totalsent = totalsent + sent

    def connect(self, host="localhost", port=4212):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))

    def close(self):
        self.sock.close()

    def waitForReply(self):
        d = self.sock.recv(1024)
        if len(d) > 1:
            raise PastecException("Received message too long.")
        val = struct.unpack("B", d)[0]
        return val

    def setIndexMode(self):
        d = struct.pack("B", Query.INIT_BUILD_FORWARD_INDEX)
        self.sendData(d)
        val = self.waitForReply()
        if val != Reply.OK:
            raise PastecException("Could not start index mode.") 

    def indexImageFile(self, imageId, filePath):
        fd = open(filePath, "rb")

        imageData = b""
        counter = 0
        while 1:
            buf = fd.read(1024)
            # End of file
            if not buf:
                break
            imageData += buf
            counter += 1
            if counter == 1024:
                raise PastecException("Image file too big.")
                return

        d = struct.pack("B", Query.INDEX_IMAGE)
        d += struct.pack("II", imageId, len(imageData))
        d += imageData

        self.sendData(d)

        val = self.waitForReply()
        if val != Reply.OK:
            raise PastecException("Image not indexed.")  

