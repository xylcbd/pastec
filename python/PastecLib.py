#!/usr/bin/python3

import urllib.request
import json


class PastecException(Exception):
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return repr(self.msg)


class PastecConnection:

    def __init__(self, pastecHost = "localhost", pastecPort = 4212):
        self.host = pastecHost
        self.port = pastecPort

    def request(self, path, method, data = None):
        req = urllib.request.Request(url="http://" + self.host + ":" + \
                                     str(self.port) + "/" + path, \
                                     data=data, method=method)
        f = urllib.request.urlopen(req)
        ret = f.read().decode()
        return json.loads(ret)

    def indexImageFile(self, imageId, filePath):
        self.indexImageData(imageId, self.loadFileData(filePath))

    def indexImageData(self, imageId, imageData):
        ret = self.request("index/images/" + str(imageId), "PUT", imageData)
        self.raiseExceptionIfNeeded(ret["type"])

    def removeImage(self, imageId):
        ret = self.request("index/images/" + str(imageId), "DELETE", imageData)
        self.raiseExceptionIfNeeded(ret["type"])

    def loadIndex(self, path = ""):
        s = json.dumps({"type" : "LOAD", "index_path" : path})
        ret = self.request("index/io", "POST", bytearray(s, "UTF-8"))
        self.raiseExceptionIfNeeded(ret["type"])

    def writeIndex(self, path = ""):
        s = json.dumps({"type" : "WRITE", "index_path" : path})
        ret = self.request("index/io", "POST", bytearray(s, "UTF-8"))
        self.raiseExceptionIfNeeded(ret["type"])

    def clearIndex(self):
        s = json.dumps({"type" : "CLEAR"})
        ret = self.request("index/io", "POST", bytearray(s, "UTF-8"))
        self.raiseExceptionIfNeeded(ret["type"])

    def imageQueryFile(self, filePath):
        return self.imageQueryData(self.loadFileData(filePath))

    def imageQueryData(self, imageData):
        ret = self.request("index/searcher", "POST", imageData)
        self.raiseExceptionIfNeeded(ret["type"])
        imageIds = ret["image_ids"]
        return imageIds

    def raiseExceptionIfNeeded(self, val):
        if val == "ERROR_GENERIC":
            raise PastecException("Generic error.")
        elif val == "TOO_MANY_CLIENTS":
            raise PastecException("Too many clients connected to the server.")
        elif val == "IMAGE_DATA_TOO_BIG":
            raise PastecException("Image data size too big.")
        elif val == "IMAGE_NOT_INDEXED":
            raise PastecException("Image not indexed.")
        elif val == "IMAGE_SIZE_TOO_BIG":
            raise PastecException("Image dimenssions too big.")
        elif val == "IMAGE_NOT_DECODED":
            raise PastecException("The query image could not be decoded.")
        elif val == "IMAGE_SIZE_TOO_SMALL":
            raise PastecException("Image size too small.")
        elif val == "IMAGE_ALREADY_IN_INDEX":
            raise PastecException("An image with this id already exists in the index.")
        elif val == "IMAGE_NOT_FOUND":
            raise PastecException("Image not found.")

        elif val == "INDEX_NOT_FOUND":
            raise PastecException("Index not found.")
        elif val == "INDEX_NOT_WRITTEN":
            raise PastecException("Index not written.")

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
