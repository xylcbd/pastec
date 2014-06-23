#!/usr/bin/python3

import socket
import struct

from PastecLibMessages import Reply, Query

MAX_IMAGE_SIZE = 1024 * 1024


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
        self.sock.setblocking(True)
        self.sock.connect((host, port))
        val = self.waitForReply()
        self.raiseExceptionIfNeeded(val)

    def close(self):
        self.sock.close()

    def waitForReply(self):
        d = self.sock.recv(1024)
        if len(d) > 4:
            raise PastecException("Received message too long.")
        val = struct.unpack("I", d)[0]
        return val

    def indexImageFile(self, imageId, filePath):
        self.indexImageData(imageId, self.loadFileData(filePath))

    def indexImageData(self, imageId, imageData):
        if len(imageData) > MAX_IMAGE_SIZE:
            raise PastecException("Image data size too big.")

        d = struct.pack("III", Query.INDEX_IMAGE, \
                        imageId, len(imageData))
        d += imageData

        self.sendData(d)

        val = self.waitForReply()
        self.raiseExceptionIfNeeded(val)

    def removeImage(self, imageId):
        d = struct.pack("II", Query.REMOVE_IMAGE, imageId)
        self.sendData(d)
        val = self.waitForReply()
        self.raiseExceptionIfNeeded(val)

    def writeIndex(self):
        d = struct.pack("I", Query.WRITE_INDEX)
        self.sendData(d)
        val = self.waitForReply()
        self.raiseExceptionIfNeeded(val)

    def clearIndex(self):
        d = struct.pack("I", Query.CLEAR_INDEX)
        self.sendData(d)
        val = self.waitForReply()
        self.raiseExceptionIfNeeded(val)

    def imageQueryFile(self, filePath):
        return self.imageQueryData(self.loadFileData(filePath))

    def imageQueryData(self, imageData):
        if len(imageData) > MAX_IMAGE_SIZE:
            raise PastecException("Image data size too big.")

        d = struct.pack("II", Query.SEARCH, len(imageData))
        d += imageData

        self.sendData(d)

        msg = b""
        while len(msg) < 4:
            msg += self.sock.recv(1024)

        # Get the message code.
        val = struct.unpack("I", msg[:4])[0]
        self.raiseExceptionIfNeeded(val)

        # We get a list of images.

        while len(msg) < 8:
            msg += self.sock.recv(1024)

        # Get the number of images.
        nbImages = struct.unpack("I", msg[4:8])[0]

        # Receive all the message containing the ids of the images.
        while len(msg) < 5 + nbImages * 4:
            msg += s.recv(1024)

        # Extract the image ids.
        imageIds = []
        for i in range(nbImages):
            imageIds += [struct.unpack("I", msg[8 + 4 * i : 8 + 4 * (i + 1)])[0]]

        return imageIds

    def raiseExceptionIfNeeded(self, val):
        if val == Reply.OK:
            return
        elif val == Reply.ERROR_GENERIC:
            raise PastecException("Generic error.")
        elif val == Reply.TOO_MANY_CLIENTS:
            raise PastecException("Too many clients connected to the server.")
        if val == Reply.IMAGE_DATA_TOO_BIG:
            raise PastecException("Image data size too big.")
        elif val == Reply.IMAGE_SIZE_TOO_BIG:
            raise PastecException("Image dimenssions too big.")
        elif val == Reply.IMAGE_SIZE_TOO_SMALL:
            raise PastecException("Image size too small.")
        elif val == Reply.IMAGE_NOT_DECODED:
            raise PastecException("The query image could not be decoded.")
        elif val == Reply.IMAGE_ALREADY_IN_INDEX:
            raise PastecException("An image with this id already exists in the index.")
        else:
            raise PastecException("Unkown error code: %#010x" % val)

    def loadFileData(self, filePath):
        fd = open(filePath, "rb")
        imageData = b""
        while 1:
            buf = fd.read(1024)
            # End of file
            if not buf:
                break
            imageData += buf
        return imageData
