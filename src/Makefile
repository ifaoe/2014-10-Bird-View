### Makefile --- 
## Author:  alex.weidauer@huckfinn.de
## Version: $Id: Makefile,v 0.1 2013/10/02 14:41:26 huckfinn Exp $

LPATH   = -L/usr/lib -L/usr/local/lib -L.
IPATH   = -I/usr/include/ -I/usr/local/include
CFLAGS  = -std=c99 -pedantic
LLINALG = -lgsl -lblas
LGDAL   = -lgdal

all:	build/geo-pix build/gpc-trfm build/geo-cut
	cp build/geo-pix  ~/tools
	cp build/gpc-trfm ~/tools
	cp build/geo-cut  ~/tools

clean:
	rm build/*

build/geo-pix: geo-pix.c alg.o
	gcc $(IPATH) $(LPATH) $(LLINALG) $(LGDAL) $(CFLAGS) -o build/geo-pix  geo-pix.c alg.o
	
build/geo-cut: geo-cut.c alg.o
	gcc $(IPATH) $(LPATH) $(LLINALG) $(LGDAL) $(CFLAGS) -o build/geo-cut  geo-cut.c alg.o 

build/gpc-trfm: gpc-trfm.c alg.c
	gcc  $(IPATH) $(LPATH) $(LLINALG) $(CFLAGS) -o build/gpc-trfm gpc-trfm.c alg.c

alg.o: alg.c
	gcc $(IPATH) $(LPATH) $(LLINALG)  $(CFLAGS) -c alg.c 

test:
	build/gpc-trfm -d -i -s "|" -p 1 0 10 0 -p 0 1 0 10 -p 1 1 10 10 -t 1 1 -t 1 0
	build/gpc-trfm -s "|" -p 1 0 10 0 -p 0 1 0 10 -p 1 1 10 10 -t 1 1 -t 1 0




