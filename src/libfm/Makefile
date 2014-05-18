BIN_DIR := ../../bin/

OBJECTS := \
	libfm.o \
	tools/transpose.o \
	tools/convert.o \
	tools/parse_vw.o \

all: libFM transpose convert parse_vw

libFM: libfm.o
	g++ -O3 -Wall libfm.o -o $(BIN_DIR)libFM

%.o: %.cpp
	g++ -g -Wall -c $< -o $@

clean:	clean_lib
	rm -f $(BIN_DIR)libFM $(BIN_DIR)convert $(BIN_DIR)transpose

clean_lib:
	rm -f $(OBJECTS)


transpose: tools/transpose.o
	g++ -O3 tools/transpose.o -o $(BIN_DIR)transpose

convert: tools/convert.o
	g++ -O3 tools/convert.o -o $(BIN_DIR)convert

parse_vw: tools/parse_vw.o
	g++ -g tools/parse_vw.o -o $(BIN_DIR)parse_vw

