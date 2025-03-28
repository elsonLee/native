CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

# 添加 Boost 和 Polygon 相关设置
INCLUDES = -I/usr/include/boost/polygon/ -I/usr/include/gtest/
LDFLAGS =
LIBS = -lgtest -pthread

# 目标文件和源文件
TARGET = main
TEST_TARGET = test_runner
#SRCS = $(filter-out main.cpp, $(wildcard *.cpp))
SRCS = $(wildcard *.cpp)
TEST_SRCS = $(wildcard *.test.cpp)
OBJS = $(SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接目标文件，添加库链接
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) $^ -o $(TARGET)

# 添加测试目标
$(TEST_TARGET): $(TEST_OBJS) $(filter-out main.o, $(OBJS))
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

# 编译源文件，添加包含路径
%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
# 清理编译产物
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET)

# 防止与同名文件冲突
.PHONY: all clean test
