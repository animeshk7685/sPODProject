CXX=g++
TARGET = toollib.a

O-SRCS= board.o dab.o setbaud.o serial.o crc16.o crc32.o common.o utils.o spkt.o pretty_units.o
H-SRCS= common.h crc16.h serial.h spkt.h lib.h utils.h types.h pretty_units.h
INCLUDES= -I. -I../../libraries/spod_library/src\
          -I$(TARGET_DIR)/usr/include

$(TARGET): $(O-SRCS)
	ar rcs $@ $^

board.o: board.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
dab.o: dab.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
setbaud.o: setbaud.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
serial.o: serial.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
crc16.o: crc16.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
crc32.o: crc32.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
common.o: common.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
utils.o: utils.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
spkt.o: spkt.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
 
pretty_units.o: pretty_units.c $(H-SRCS)
	$(CXX) -g $(INCLUDES) -c $< -o $@
     

clean:
	rm -f *.o $(TARGET)
