# Copyright (c) 2014 Cesanta Software
# All rights reserved


PUB_DIR:=/home/lscm/codebase
ILP_LIB:=$(PUB_DIR)/ilplib/include/
IZENE_LIB:=$(PUB_DIR)/izenelib/include/
JSON_INC:=../include/3third-lib/jsoncpp/include/
JSON_LIB:=../include/3third-lib/jsoncpp/lib/


PROG = http-server
CFLAGS = -W -Wall -I../.. -pthread -g -O0 -DMONGOOSE_ENABLE_THREADS $(CFLAGS_EXTRA)
SOURCES = $(PROG).cc mongoose.c indexEngine.cpp recommendEngine.cpp $(PUB_DIR)/ilplib/source/knlp/normalize.cpp 

CC=g++

LDFLAGS=-lboost_system -lglog -ljson -lboost_filesystem
$(PROG): $(SOURCES)
	$(CC) -o $(PROG) $(SOURCES) $(CFLAGS) $(LDFLAGS) -I $(ILP_LIB) -I $(IZENE_LIB) -I $(JSON_INC) -L $(JSON_LIB)


clean:
	rm -rf $(PROG) *.exe *.dSYM *.obj *.exp .*o *.lib
