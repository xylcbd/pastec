#!/usr/bin/python3

# Echo client program
import socket
import sys

HOST = sys.argv[1]    # The remote host
PORT = 4212              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

d = bytearray('\x03', 'UTF-8')
s.send(d)

s.close()
