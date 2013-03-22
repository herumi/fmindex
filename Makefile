OPT=-O3 -fomit-frame-pointer -DNDEBUG -mtune=native

TARGET=search construct

# ----------------------------------------------------------------
CFLAGS+= -g3 -msse4 -I../cybozulib/include -Iinclude $(OPT)
CFLAGS_WARN=-Wall -Wextra -Wformat=2 -Wcast-qual -Wwrite-strings -Wfloat-equal -Wpointer-arith
CFLAGS+=$(CFLAGS_WARN)
# ----------------------------------------------------------------
all:$(TARGET)

.SUFFIXES: .cpp

search: search.o
	$(CXX) $(LDFLAGS) $< -o $@

construct: construct.o
	$(CXX) $(LDFLAGS) $< -o $@

.cpp.o:
	$(CXX) -c $< -o $@ $(CFLAGS)

.cc.o:
	$(CXX) -c $< -o $@ $(CFLAGS)

.c.o:
	$(CXX) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) *.o $(TARGET)


HEADER=include/cybozu/fmindex.hpp ../cybozulib/include/cybozu/wavelet_matrix.hpp ../cybozulib/include/cybozu/sucvector.hpp
search.o: search.cpp $(HEADER)
construct.o: construct.cpp $(HEADER)

