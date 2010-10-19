# Version $Revision: 1.0 $
#
# The makefile for building all versions for all supported platforms
#
# Supports:    Linux , Mac
#
# CC      = cc
# C89     = c89
# GCC     = gcc
# CCS     = /usr/ccs/bin/cc
# NACC    = /opt/ansic/bin/cc
#
#

#CFLAGS  = -DDEBUG -g

all:
	@echo ""
	@echo "You must specify the target.	"
	@echo "	->   linux		(32bit)   <-"
	@echo "	->   macosx		(32bit)   <-"
	@echo ""

clean:
	rm -f wrv.fs
	rm -f *.o
	rm -f linux/*.o
	rm -f macosx/*.o

#
# GNU 'C' compiler Linux build with threads, largefiles, async I/O
#

linux:  iotestlinux.o ioutils.o iomethods.o linux/linuxioutils.o
	$(CC)  -O3 $(LDFLAGS)   iotestlinux.o linux/linuxioutils.o \
				ioutils.o iomethods.o -lrt -o wrv.fs -lpthread
				

macosx:	iotestmacosx.o iomethods.o macosx/macosxioutils.o
	$(CC) 	-O3 	$(LDFLAGS) 	iotestmacosx.o 	macosx/macosxioutils.o \
				ioutils.o iomethods.o -o wrv.fs -lpthread

iotestlinux.o: 	iotest.c linux/linuxioutils.c ioutils.c
	@echo ""
	@echo "Building package for Linux"
	@echo ""
	$(CC) -Wall -c -Dlinux -D_LARGEFILE64_SOURCE $(CFLAGS) iotest.c \
		-DNAME='"linux"' -o iotestlinux.o
	$(CC) -Wall -c -Dlinux -D_LARGEFILE64_SOURCE $(CFLAGS) ioutils.c \
		-DNAME='"linux"' -o ioutils.o
	$(CC) -Wall -c -Dlinux -D_LARGEFILE64_SOURCE $(CFLAGS) iomethods.c \
		-DNAME='"linux"' -o iomethods.o
	$(CC) -Wall -c -Dlinux -D_LARGEFILE64_SOURCE $(CFLAGS) linux/linuxioutils.c \
		-DNAME='"linux"' -o linux/linuxioutils.o


iotestmacosx.o: iotest.c  macosx/macosxioutils.c ioutils.c
	@echo ""
	@echo "Build package for MacOSX"
	@echo ""
	$(CC) -Wall -c -Dmacosx $(CFLAGS) iotest.c \
		-DNAME='"macosx"' -o iotestmacosx.o
	$(CC) -Wall -c -Dmacosx $(CFLAGS) ioutils.c \
		-DNAME='"macosx"' -o ioutils.o
	$(CC) -Wall -c -Dmacosx  $(CFLAGS) iomethods.c \
		-DNAME='"macosx"' -o iomethods.o
	$(CC) -Wall -c -Dmacosx  $(CFLAGS) macosx/macosxioutils.c \
		-DNAME='"macosx"' -o macosx/macosxioutils.o
