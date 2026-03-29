all: fput

CXX=g++
OUTPUT=fput

C-SRCS= main.c
H-SRCS= ../toollib/common.h ../toollib/crc16.h main.h ../toollib/serial.h ../toollib/spkt.h ../toollib/lib.h ../toollib/utils.h ../toollib/types.h ../toollib/pretty_units.h
INCLUDES= -I. -I../../spod_library/src -I../toollib\
          -I$(TARGET_DIR)/usr/include

$(OUTPUT): $(C-SRCS) $(H-SRCS) ../toollib/toollib.a
	$(CXX) -g  $(INCLUDES) -o $(OUTPUT) $(C-SRCS) ../toollib/toollib.a -L$(TARGET_DIR)/usr/lib/

clean:
	rm -f $(OUTPUT)
