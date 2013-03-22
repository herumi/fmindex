
This is a prototype of fmindex by herumi.

The original version is fmindex-plus-plus.
(http://code.google.com/p/fmindex-plus-plus/)

This includes sais.hxx by Yuta Mori

Build
-------------

    mkdir work
    cd work
    git clone git://github.com/herumi/cybozulib.git
    git clone git://github.com/herumi/opti.git
    git clone git://github.com/herumi/fmindex.git
    (cd opti/comp && sh ./build-wavelet-sh)
    cd fmindex
    make -j


A tiny benchmark
-------------
input.txt is 203MB UTF-8 text file.
query.txt is 10KB UTF-8 text file.

    % dir input.txt
    -rw-r--r-- 1 shigeo shigeo 213829088 Mar 21 16:14 input.txt
    % wc query.txt
    1431   442 10389 query.txt
    ; original fmindex
    % time ./fmconstruct input.txt input.dat
    start reading the input-file
    alphabet size:219
    build SA
    calculate statistics
    build BWT
    build WaveletTree
    build sampledSA
    cpu time:61.690000
    75.520u 1.712s 1:17.43 99.7%    0+0k 0+866456io 0pf+0w
    % dir input.dat
    -rw-r--r-- 1 shigeo shigeo 443623615 Mar 21 16:16 input.dat
    % time ./fmsearch input.dat query.txt >a.txt
    exact search mode:
    cpu time: 160.560000
    160.750u 0.704s 2:42.03 99.6%   0+0k 0+448816io 0pf+0w

    ; use wavelet-matrix-cpp
    % time ./construct2 input.txt input.dat
    start reading the input-file
    alphabet size:219
    build SA
    calculate statistics
    build BWT
    build WaveletTree
    build sampledSA
    cpu time:64.540000
    77.716u 1.824s 1:19.74 99.7%    0+0k 0+815496io 0pf+0w
    % dir input.dat
    -rw-r--r-- 1 shigeo shigeo 417532311 Mar 23 05:00 input.dat
    % time ./search2 input.dat ../query.txt > b.txt
    exact search mode:
    cpu time: 71.930000
    71.992u 0.720s 1:12.94 99.6%    0+0k 0+448768io 0pf+0w

    ; forked fmindex with cybozu/wavelet_matrix.hpp
    % time ./construct input.txt input.dat
    start reading the input-file
    alphabet size:219
    build SA
    calculate statistics
    build BWT
    build WaveletTree
    build sampledSA
    cpu time:60.730000
    74.536u 1.404s 1:16.13 99.7%    0+0k 0+923808io 0pf+0w
    % dir input.dat
    -rw-r--r-- 1 shigeo shigeo 472987963 Mar 21 16:16 input.dat
    % time ./search input.dat ../query.txt >c.txt
    exact search mode:
    cpu time: 30.050000
    29.869u 0.744s 0:30.72 99.6%    0+0k 0+448776io 0pf+0w

    a.txt, b.txt and c.txt are all same.

License
--------------------------------------------------------------------
fmindex-plus-plus

Copyright (c) 2010 Yasuo Tabei

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above Copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above Copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. Neither the name of the authors nor the names of its contributors
   may be used to endorse or promote products derived from this
   software without specific prior written permission.

--------------------------------------------------------------------
sais.hxx for sais-lite
Copyright (c) 2008-2009 Yuta Mori All Rights Reserved.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

