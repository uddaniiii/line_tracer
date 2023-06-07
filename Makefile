CC = g++
CFLAGS = -g -Wall
CFLAGS += `pkg-config opencv4 --cflags --libs`
CFLAGS += -I/usr/local/include/dynamixel_sdk_cpp #헤더파일경로 지정
CFLAGS += -ldxl_x64_cpp #라이브러리파일 지정,+=는 이전 설정값에 추가하라는 의미
SRCS = main.cpp
TARGET = lt
OBJS = main.o dxl.o
$(TARGET):$(SRCS)
	   $(CC) -o $(TARGET) $(SRCS) $(CFLAGS)
$(TARGET) : $(OBJS)
	   $(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(CFLAGS)
main.o : main.cpp
	   $(CC) $(CFLAGS) -c main.cpp $(CFLAGS)
dxl.o : dxl.hpp dxl.cpp
	   $(CC) $(CFLAGS) -c dxl.cpp $(CFLAGS)    
.PHONY: all clean
all: $(TARGET)
clean:
	   rm -f $(OBJS) $(TARGET) 
	   rm -rf $(TARGET) $(OBJS)
