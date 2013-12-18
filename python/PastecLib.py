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
        val = self.waitForReply()
        if val == Reply.TOO_MANY_CLIENTS:
            raise PastecException("Too many clients connected to the server.")
        elif val != Reply.OK:
            raise PastecException("Unkown returned code.")

    def close(self):
        self.sock.close()

    def waitForReply(self):
        d = self.sock.recv(1024)
        if len(d) > 1:
            raise PastecException("Received message too long.")
        val = struct.unpack("B", d)[0]
        return val

    def initBuildForwardIndex(self):
        d = struct.pack("B", Query.INIT_BUILD_FORWARD_INDEX)
        self.sendData(d)
        val = self.waitForReply()
        if val != Reply.OK:
            raise PastecException("Could not start forward index building.")
            
    def buildBackwardIndex(self):
        d = struct.pack("B", Query.BUILD_BACKWARD_INDEX)
        self.sendData(d)
        val = self.waitForReply()
        if val != Reply.OK:
            raise PastecException("Could not build backward index.")

    def initSearch(self):
        d = struct.pack("B", Query.INIT_SEARCH)
        self.sendData(d)
        val = self.waitForReply()
        if val != Reply.OK:
            raise PastecException("Could not start the searching mode.")

    def stopServer(self):
        d = struct.pack("B", Query.STOP)
        self.sendData(d)

    def indexImageFile(self, imageId, filePath):
        fd = open(filePath, "rb")
        imageData = b""
        while 1:
            buf = fd.read(1024)
            # End of file
            if not buf:
                break
            imageData += buf

        self.indexImageData(imageId, imageData)

    def indexImageData(self, imageId, imageData):
        if len(imageData) > 1024 * 1024:
            raise PastecException("Image file too big.")

        d = struct.pack("B", Query.INDEX_IMAGE)
        d += struct.pack("II", imageId, len(imageData))
        d += imageData

        self.sendData(d)

        val = self.waitForReply()
        if val == Reply.IMAGE_DATA_TOO_BIG:
            raise PastecException("Image data too big.")
        elif val == Reply.IMAGE_SIZE_TOO_BIG:
            raise PastecException("Image size too big.")
        elif val == Reply.IMAGE_NOT_DECODED:
            raise PastecException("The query image could not be decoded.")
        elif val != Reply.OK:
            raise PastecException("Unkown returned code.")

    def imageQuery(self, imageData):
        d = struct.pack("B", Query.SEARCH)
        d += struct.pack("I", len(data))
        d += data

        self.sendData(d)

        msg = b""
        while len(msg) < 1:
            msg += self.sock.recv(1024)

        # Get the message code.
        val = int.from_bytes(struct.unpack("c", msg[:1])[0], byteorder='little')
        if val == Reply.IMAGE_DATA_TOO_BIG:
            raise PastecException("Image data too big.")
        elif val == Reply.IMAGE_SIZE_TOO_BIG:
            raise PastecException("Image size too big.")
        elif val == Reply.IMAGE_NOT_DECODED:
            raise PastecException("The query image could not be decoded.")
        elif val != Reply.OK:
            raise PastecException("Unkown returned code.")

        # code == 1: We get a list of images.

        while len(msg) < 5:
            msg += self.sock.recv(1024)

        # Get the number of images.
        nbImages = struct.unpack("I", msg[1:5])[0]
        print("Got back " + str(nbImages) + " images.", file=sys.stderr)

        # Receive all the message containing the ids of the images.
        while len(msg) < 5 + nbImages * 4:
            msg += s.recv(1024)

        # Extract the image ids.
        imageIds = []
        for i in range(nbImages):
            imageIds += [struct.unpack("I", msg[5 + 4 * i : 5 + 4 * (i + 1)])[0]]

        return imageIds
