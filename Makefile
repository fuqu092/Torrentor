# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -O3 -fopenmp -Iinclude

# Target executables
CLIENT_TARGET = client
SERVER_TARGET = server

# Source files
CLIENT_SRC = client.cpp src/message.cpp src/message_handler.cpp
SERVER_SRC = server.cpp src/message.cpp src/message_handler.cpp

# Object files
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)

# Default rule
all: $(CLIENT_TARGET) $(SERVER_TARGET)

# Link object files to create the client executable
$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Link object files to create the server executable
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove compiled files
clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(CLIENT_TARGET) $(SERVER_TARGET)