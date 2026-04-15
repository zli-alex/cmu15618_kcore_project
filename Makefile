CXX ?= c++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude
LDFLAGS :=
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
SDKROOT := $(shell xcrun --sdk macosx --show-sdk-path)
CXXFLAGS += -isysroot $(SDKROOT)
CXXFLAGS += -isystem $(SDKROOT)/usr/include/c++/v1
endif

TARGET := seq_runner
TRACE_TARGET := seq_trace_runner
PAR_TRACE_TARGET := par_trace_runner

CORE_PKG := $(wildcard src/core/*.cpp)
SEQ_PKG := $(filter-out src/sequential/main.cpp src/sequential/trace_runner.cpp,$(wildcard src/sequential/*.cpp))
COMMON_PKG := $(CORE_PKG) $(SEQ_PKG)
PAR_IMPL_SRC := src/parallel/parallel_batch_engine.cpp
PAR_TRACE_SRC := src/parallel/trace_runner.cpp

SEQ_MAIN_SRC := src/sequential/main.cpp
TRACE_MAIN_SRC := src/sequential/trace_runner.cpp

COMMON_OBJ := $(COMMON_PKG:.cpp=.o)
SEQ_MAIN_OBJ := $(SEQ_MAIN_SRC:.cpp=.o)
SEQ_TRACE_OBJ := $(TRACE_MAIN_SRC:.cpp=.o)
PAR_IMPL_OBJ := $(PAR_IMPL_SRC:.cpp=.o)
PAR_TRACE_OBJ := $(PAR_TRACE_SRC:.cpp=.o)

SEQ_OBJ := $(COMMON_OBJ) $(SEQ_MAIN_OBJ)
TRACE_OBJ := $(COMMON_OBJ) $(SEQ_TRACE_OBJ)
PAR_TRACE_OBJ_ALL := $(COMMON_OBJ) $(PAR_IMPL_OBJ) $(PAR_TRACE_OBJ)

TEST_SRC := $(wildcard tests/sequential/*.cpp)
TEST_BINS := $(patsubst tests/sequential/%.cpp,tests/sequential/bin/%,$(TEST_SRC))

.PHONY: all clean test

all: $(TARGET) $(TRACE_TARGET) $(PAR_TRACE_TARGET)

$(TARGET): $(SEQ_OBJ)
	$(CXX) $(CXXFLAGS) $(SEQ_OBJ) -o $@ $(LDFLAGS)

$(TRACE_TARGET): $(TRACE_OBJ)
	$(CXX) $(CXXFLAGS) $(TRACE_OBJ) -o $@ $(LDFLAGS)

$(PAR_TRACE_TARGET): $(PAR_TRACE_OBJ_ALL)
	$(CXX) $(CXXFLAGS) $(PAR_TRACE_OBJ_ALL) -o $@ $(LDFLAGS)

src/core/%.o: src/core/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/sequential/%.o: src/sequential/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/parallel/%.o: src/parallel/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests/sequential/bin/%: tests/sequential/%.cpp $(COMMON_PKG) | tests/sequential/bin
	$(CXX) $(CXXFLAGS) $< $(COMMON_PKG) -o $@ $(LDFLAGS)

tests/sequential/bin:
	mkdir -p tests/sequential/bin

test: $(TEST_BINS)
	for t in $(TEST_BINS); do echo "[test] $$t"; $$t || exit 1; done

clean:
	rm -f $(TARGET) $(TRACE_TARGET) $(PAR_TRACE_TARGET)
	rm -f $(SEQ_OBJ) $(TRACE_OBJ) $(PAR_TRACE_OBJ_ALL)
