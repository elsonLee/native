CPP = g++
CFLAGS = -g -std=c++17 -Wall -I./
LDFLAG = -lpthread
SOURCES = Acceptor.cpp \
          Buffer.cpp \
	  Channel.cpp \
	  EventLoop.cpp \
	  EventLoopThread.cpp \
	  InetAddress.cpp \
	  Poller.cpp \
	  Socket.cpp \
	  SocketOps.cpp \
	  TcpConnection.cpp \
	  Connector.cpp \
	  TcpServer.cpp \
	  Timer.cpp \
	  TimerQueue.cpp

OBJ_DIR = ./objs
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(SOURCES:.cpp=.o)))
TARGET = main

TEST_DIR = ./test
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINARY = $(TEST_SOURCES:$(TEST_DIR)/%.cpp=%)
TEST_OBJ_DIR = $(OBJ_DIR)/test
TEST_OBJ = $(TEST_BINARY:%=$(TEST_OBJ_DIR)/%.o)

.PHONY: all test make_dir clean

default: make_dir $(OBJECTS) $(TARGET)
all: make_dir $(OBJECTS) $(TEST_OBJ) $(TARGET) $(TEST_BINARY)
test: make_dir $(TEST_OBJ) $(TEST_BINARY)

make_dir: $(OBJ_DIR) $(TEST_OBJ_DIR)

$(TARGET): $(OBJECTS) main.cpp
	$(CPP) $^ $(LDFLAG) -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

# unit tests
$(TEST_BINARY): %: $(OBJECTS) $(TEST_OBJ_DIR)/%.o
	$(CPP) $^ $(LDFLAG) -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $@

$(TEST_OBJ_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(OBJ_DIR) $(TARGET) $(TEST_BINARY)

