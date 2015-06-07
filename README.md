Pastec core windows port
======

Introduction
------------

### Presentation

[Pastec](http://www.pastec.io) is an open source index and search engine for image recognition based on [OpenCV](http://www.opencv.org/). It can recognize flat objects such as covers, packaged goods or artworks. It has however not been designed to recognize faces, 3D objects, barcodes and QR codes.

Pastec can be, for example, used to recognize DVD covers in a mobile app or detect near duplicate images in a big database.

Pastec do not store the pixels of the images in its database. It stores a signature of each image thanks to the technique of the [visual words](http://en.wikipedia.org/wiki/Visual_Word).

Pastec offers a tiny HTTP API using JSON to add, remove and search for images in the index.

### Intellectual property

Pastec is developped by [Visualink](http://www.visualink.io) and licenced under the [GNU LGPL v3.0](http://www.gnu.org/licenses/lgpl.html).
It is only based on the free packages of [OpenCV](http://www.opencv.org/) that are available for commercial purposes. You should therefore be free to use Pastec without paying any patent licence.

More precisely, Pastec uses the [patent-free ORB descriptor](https://www.willowgarage.com/sites/default/files/orb_final.pdf) and not the well-known SIFT and SURF descriptors that are patented.

Setup
-----

See [here](http://pastec.io/doc#setup).

API
---

See [here](http://pastec.io/doc#api)

### Windows port

This repo is windows port of pastec core.

