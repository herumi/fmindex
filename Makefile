OPT=-O3 -fomit-frame-pointer -DNDEBUG -mtune=native

TARGET=search construct search2 construct2

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

WAVELET_CFLAGS+=-DCOMPARE_WAVELET -I../opti/comp/wavelet-matrix-cpp/src
WAVELET_LDFLAGS+=-lwavelet_matrix -L../opti/comp/lib
WAVELET_LIB=../opti/comp/lib/libwavelet_matrix.a

HEADER=include/cybozu/fmindex.hpp ../cybozulib/include/cybozu/wavelet_matrix.hpp ../cybozulib/include/cybozu/sucvector.hpp
search.o: search.cpp $(HEADER)
construct.o: construct.cpp $(HEADER)

search2.o: search.cpp $(HEADER)
	$(CXX) -c $< -o $@ $(CFLAGS) $(WAVELET_CFLAGS)
construct2.o: construct.cpp $(HEADER)
	$(CXX) -c $< -o $@ $(CFLAGS) $(WAVELET_CFLAGS)
search2: search2.o
	$(CXX) $(LDFLAGS) $< -o $@ $(WAVELET_LIB)

construct2: construct2.o
	$(CXX) $(LDFLAGS) $< -o $@ $(WAVELET_LIB)

