# Compiler Selection (Platform-dependent)
ifeq ($(OS),Windows_NT)
  # For Windows (using MinGW)
  CXX = g++ # Change to 'x86_64-w64-mingw32-g++' if you want cross-compile for Windows on Linux
  CXXFLAGS = -O3 -fopenmp -Iinclude -D_WIN32 # Define Windows-specific flags
  CLIENT_TARGET = client.exe
  SERVER_TARGET = server.exe
else
  # For Linux (Default)
  CXX = g++
  CXXFLAGS = -O3 -fopenmp -Iinclude
  CLIENT_TARGET = client
  SERVER_TARGET = server
endif

# Source files
CLIENT_SRC = client.cpp src/message.cpp src/message_handler.cpp src/helper_functions.cpp
SERVER_SRC = server.cpp src/message.cpp src/message_handler.cpp src/helper_functions.cpp

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
