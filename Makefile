# 컴파일러와 플래그
CC = g++
CFLAGS = -g -Wall `pkg-config opencv4 --cflags --libs` -I/usr/local/include/dynamixel_sdk_cpp -ldxl_x64_cpp

# 파일 설정
SRC_DIR = src
BUILD_DIR = build
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/dxl.cpp
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = lt

# 기본 타겟
all: $(TARGET)

# 실행 파일 빌드
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 오브젝트 파일 빌드
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR) 
	$(CC) $(CFLAGS) -c $< -o $@

# 정리
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
