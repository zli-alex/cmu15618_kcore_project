CXX ?= c++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude
LDFLAGS :=
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
SDKROOT := $(shell xcrun --sdk macosx --show-sdk-path)
CXXFLAGS += -isysroot $(SDKROOT)
CXXFLAGS += -isystem $(SDKROOT)/usr/include/c++/v1
endif

# Keep OpenMP flags separate for Week 2+ enablement.
OPENMP_CXXFLAGS := -fopenmp
OPENMP_LDFLAGS := -fopenmp

TARGET := seq_runner
SRC := src/sequential/main.cpp
OBJ := $(SRC:.cpp=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDFLAGS)

src/sequential/%.o: src/sequential/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET) --test

clean:
	rm -f $(TARGET) $(OBJ)
