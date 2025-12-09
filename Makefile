CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
LDFLAGS = -pthread

TARGET = program
SRCS = main.cpp thread_pool.cpp ipc_manager.cpp process_manager.cpp thread_manager.cpp

OBJS = $(SRCS:.cpp=.o)

all:$(TARGET)

$(TARGET): $(OBJS)
  $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS)
%.o: %.cpp
  $(CXX) $(CXXFLAGS) -c $< -o $@
.PHONY:clean
clean:
  rm -f $(TARGET) $(OBJS)
