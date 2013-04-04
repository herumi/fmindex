#pragma once
/**
	@file
	@brief FM-index
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause

	@note original version is http://code.google.com/p/fmindex-plus-plus/
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
*/
#include <map>
#include <vector>
#include <fstream>
#include "sais.hxx"
#include <stdio.h>
#include <limits.h>
#include <time.h>
#ifdef COMPARE_WAVELET
#include "wavelet_matrix.hpp"
struct WaveletMatrix {
	wavelet_matrix::WaveletMatrix wm;
	void init(const std::vector<uint8_t>& v8, int)
	{
		std::vector<uint64_t> v64;
		v64.resize(v8.size());
		for (size_t i = 0; i < v8.size(); i++) {
			v64[i] = v8[i];
		}
		wm.Init(v64);
	}
	uint64_t get(uint64_t pos) const
	{
		return wm.Lookup(pos);
	}
	uint64_t rank(uint32_t val, uint64_t pos) const
	{
		return wm.Rank(val, pos);
	}
	uint64_t rankLt(uint32_t val, uint64_t pos) const
	{
		return wm.RankLessThan(val, pos);
	}
	uint64_t select(uint32_t val, uint64_t rank) const
	{
		return wm.Select(val, rank + 1) - 1;
	}
	size_t size() const { return wm.length(); }
	size_t size(uint32_t val) const
	{
		return rank(val, wm.length());
	}
	void load(std::istream& is) { wm.Load(is); }
	void save(std::ostream& os) const { wm.Save(os); }
};
#else
#include <cybozu/bitvector.hpp>
#include <cybozu/wavelet_matrix.hpp>
typedef cybozu::WaveletMatrix WaveletMatrix;
#endif

namespace cybozu {

class FMindex {
	int percent;
	uint8_t alphaSize;
	std::vector<uint32_t> cf;
	std::map<unsigned char, uint8_t> mapping;
	std::map<uint8_t, unsigned char> rmapping;
	std::vector<uint32_t> sampledSA;
	std::vector<std::vector<int> > cols;
	WaveletMatrix wa;
public:
	FMindex()
	{
	}
	int read(const char* fname, std::vector<uint8_t>& s)
	{
		std::ifstream ifs(fname);
		std::string line;
		mapping['$'] = 0;
		rmapping[0]  = '$';
		alphaSize = 1;

		while (std::getline(ifs, line)) {
			for (size_t i = 0; i < line.size(); i++) {
				if (mapping.find(line[i]) == mapping.end()) {
					mapping[line[i]] = alphaSize;
					rmapping[alphaSize] = line[i];

					if (++alphaSize == 0) std::cerr << "warning: the variety of characters exceeds 255." << std::endl;
				}
				s.push_back(mapping[line[i]]);
			}
		}

		s.push_back(mapping['$']);
		return 0;
	}
	int readQstring(const char* fname, std::vector<std::vector<uint8_t> >& qs)
	{
		std::ifstream ifs(fname);
		std::string line;

		while (std::getline(ifs, line)) {
			if (line.empty()) continue;

			qs.resize(qs.size() + 1);
			std::vector<uint8_t>& tmp = qs[qs.size() - 1];

			for (size_t i = 0; i < line.size(); i++) {
				if (mapping.find(line[i]) == mapping.end()) {
					mapping[line[i]] = alphaSize;
					rmapping[alphaSize]  = line[i];

					if (++alphaSize == 0) std::cerr << "warning: the variety of characters exceeds 255." << std::endl;
				}
				tmp.push_back(mapping[line[i]]);
			}
		}
		return 0;
	}

	int buildSA(const std::vector<uint8_t>& s, std::vector<uint32_t>& sa)
	{
		sa.resize(s.size());

		if (saisxx(s.begin(), sa.begin(), (int)s.size(), 0x100) == -1) {
			std::cerr << "suffix array construction error" << std::endl;
			return 1;
		}
		return 0;
	}

	int buildBWT(const std::vector<uint8_t>& s, const std::vector<uint32_t>& sa, std::vector<uint8_t>& bwt)
	{
		size_t seqLen = s.size();
		bwt.resize(seqLen);

		for (size_t i = 0; i < seqLen; i++) {
			if (sa[i] > 0) {
				bwt[i] = s[sa[i] - 1];
			} else {
				bwt[i] = s[seqLen - 1];
			}
		}
		return 0;
	}

	int buildSampledSA(const std::vector<uint8_t>& s, const std::vector<uint32_t>& sa)
	{
		size_t seqLen = s.size();
		size_t divSeq = size_t(seqLen * (percent / 100.));
		divSeq = seqLen / divSeq;

		for (size_t i = 0; i < seqLen; i++) {
			if (i % divSeq == 0) sampledSA.push_back(sa[i]);
		}
		return 0;
	}

	void calculateStatistics(const std::vector<uint8_t>& s)
	{
		size_t seqLen = s.size();
		std::vector<uint32_t> tmpCf(255 + 1);

		for (size_t i = 0; i < seqLen; i++) {
			tmpCf[s[i]]++;
		}

		cf.resize(255 + 1);
		cf[0] = 0;

		for (size_t i = 1; i <= 255; i++) {
			cf[i] = tmpCf[i - 1];
			tmpCf[i] += tmpCf[i - 1];
		}
	}

	size_t locate(uint64_t i)
	{
		size_t bsize = wa.size();
		size_t divSeq = size_t(bsize * (percent / 100.));
		divSeq = bsize / divSeq;
		size_t j = i;
		size_t t = 0;

		while (j % divSeq != 0) {
			size_t c = wa.get(j);
			j = cf[c] + wa.rank(c, j + 1) - 1;
			t++;
		}

		if (sampledSA[j / divSeq] + t >= bsize) {
			return (size_t)sampledSA[j / divSeq] + t - bsize;
		}

		return (size_t)sampledSA[j / divSeq] + t;
	}

	int save(std::ostream& os)
	{
		os.write((const char*)(&percent), sizeof(percent));
		os.write((const char*)(&alphaSize), sizeof(alphaSize));
		size_t cfSize = cf.size();
		os.write((const char*)(&cfSize), sizeof(cfSize));
		size_t sampledSASize = sampledSA.size();
		os.write((const char*)(&sampledSASize), sizeof(sampledSASize));
		os.write((const char*)(&cf[0]), sizeof(cf[0])*cf.size());
		os.write((const char*)(&sampledSA[0]), sizeof(sampledSA[0])*sampledSA.size());
		uint32_t count = 0;
		std::vector<unsigned char> key;
		std::vector<uint8_t> val;

		for (std::map<unsigned char, uint8_t>::iterator it = mapping.begin(); it != mapping.end(); it++) {
			key.push_back(it->first);
			val.push_back(it->second);
			count++;
		}

		os.write((const char*)(&count), sizeof(uint32_t));
		os.write((const char*)(&key[0]), sizeof(key[0])*count);
		os.write((const char*)(&val[0]), sizeof(val[0])*count);
		wa.save(os);
		return 0;
	}

	int load(std::istream& is)
	{
		is.read((char*)(&percent), sizeof(percent));
		is.read((char*)(&alphaSize), sizeof(alphaSize));
		size_t cfSize;
		is.read((char*)(&cfSize), sizeof(cfSize));
		size_t sampledSASize;
		is.read((char*)(&sampledSASize), sizeof(sampledSASize));
		cf.resize(cfSize);
		is.read((char*)(&cf[0]), sizeof(cf[0])*cfSize);
		sampledSA.resize(sampledSASize);
		is.read((char*)(&sampledSA[0]), sizeof(sampledSA[0])*sampledSASize);
		uint32_t count;
		std::vector<unsigned char> key;
		std::vector<uint8_t>       val;
		is.read((char*)(&count), sizeof(count));
		key.resize(count);
		is.read((char*)(&key[0]), sizeof(key[0])*count);
		val.resize(count);
		is.read((char*)(&val[0]), sizeof(val[0])*count);

		for (uint32_t i = 0; i < count; i++) {
			mapping[key[i]]  = val[i];
			rmapping[val[i]] = key[i];
		}

		wa.load(is);
		return 0;
	}

	int buildWaveletTree(const std::vector<uint8_t>& bwt)
	{
		wa.init(bwt, 8);
		return 0;
	}

	int buildFmIndex(const char* fname, int _percent)
	{
		percent = _percent;
		std::cerr << "start reading the input-file" << std::endl;
		std::vector<uint8_t> s;
		read(fname, s);
		std::cerr << "alphabet size:" << (int)alphaSize << std::endl;
		double sTime = clock();
		std::vector<uint32_t> sa;
		std::cerr << "build SA" << std::endl;
		buildSA(s, sa);
		std::cerr << "calculate statistics" << std::endl;
		calculateStatistics(s);
		std::vector<uint8_t> bwt;
		std::cerr << "build BWT" << std::endl;
		buildBWT(s, sa, bwt);
		std::cerr << "build WaveletTree" << std::endl;
		buildWaveletTree(bwt);
		std::cerr << "build sampledSA" << std::endl;
		buildSampledSA(s, sa);
		double eTime = clock();
		fprintf(stderr, "cpu time:%f\n", (eTime - sTime) / CLOCKS_PER_SEC);
		return 0;
	}

	void search(const std::vector<uint8_t>& qs, std::pair<uint64_t, uint64_t>& res)
	{
		size_t sp = 1, ep = wa.size();
		uint8_t c;
		int i = (int)qs.size() - 1;

		while ((sp < ep) && (i >= 0)) {
			c  = qs[i]; i--;
			sp = cf[c] + wa.rank(c, sp);
			ep = cf[c] + wa.rank(c, ep);
		}

		if (sp < ep) {
			res = std::make_pair(sp, ep - 1);
		} else {
			res = std::make_pair((size_t) - 1, (size_t) - 1);
		}
	}
	void searchHamming(const std::vector<uint8_t>& qs, int dist, std::vector<std::pair<uint64_t, uint64_t> >& res)
	{
		res.clear();
		searchHamming(qs, (int)qs.size() - 1, 1, wa.size(), 0, dist, res);
	}

	void searchHamming(const std::vector<uint8_t>& qs, int i, size_t sp, size_t ep, int d, int dist, std::vector<std::pair<uint64_t, uint64_t> >& res)
	{
		if (sp >= ep) {
			return;
		}

		if (dist < d) {
			return;
		}

		if (i < 0) {
			res.push_back(std::make_pair(sp, ep - 1));
			return;
		}

		uint8_t s = qs[i];
		int     newd;
		uint8_t m;

		for (std::map<unsigned char, uint8_t>::iterator it = mapping.begin(); it != mapping.end(); it++) {
			m = it->second;

			if (m == 0) continue;

			newd = d;

			if (m != s) newd = d + 1;

			searchHamming(qs, i - 1, cf[m] + wa.rank(m, sp), cf[m] + wa.rank(m, ep), newd, dist, res);
		}
	}
	inline int min(int a, int b, int c)
	{
#if 1
		return std::min(a, std::min(b, c));
#else
		if (a <= b) {
			if (a <= c)
				return a;
			else if (b <= c)
				return b;
			else
				return c;

		} else {
			if (b <= c)
				return b;
			else if (a <= c)
				return a;
			else
				return c;
		}
#endif
	}

	void searchEdit(const std::vector<uint8_t>& qs, int dist, std::vector<std::pair<uint64_t, uint64_t> >& res)
	{
		res.clear();
		cols.clear();
		cols.resize(qs.size() + 1);

		for (size_t i = 0; i < cols.size(); i++) {
			cols[i].resize(qs.size() + 1);
		}

		for (size_t i = 0; i < cols.size(); i++) {
			cols[0][i] = (int)i;
			cols[i][0] = (int)i;
		}

		searchEdit(qs, 1, 1, wa.size(), dist, res);
	}

	void searchEdit(const std::vector<uint8_t>& qs, int count, int sp, int ep, int dist, std::vector<std::pair<uint64_t, uint64_t> >& res)
	{
		if (sp >= ep) return;

		if (count > (int)qs.size()) {
			if (cols[qs.size()][qs.size()] <= dist) res.push_back(std::make_pair(sp, ep - 1));
			return;
		}

		int minval, diag;
		uint8_t m;

		for (std::map<unsigned char, uint8_t>::iterator it = mapping.begin(); it != mapping.end(); it++) {
			m = it->second;

			if (it->second == 0)
				continue;

			cols[count][0] = count - 1;
			minval = INT_MAX;

			for (size_t k = 1; k <= qs.size(); k++) {
				if (qs[qs.size() - k] == it->second)
					diag = 0;
				else
					diag = 1;

				cols[count][k] = min(cols[count - 1][k] + 1, cols[count][k - 1] + 1, cols[count - 1][k - 1] + diag);

				if (cols[count][k] < minval) minval = cols[count][k];
			}

			if (minval <= dist)
				searchEdit(qs, count + 1, cf[m] + wa.rank(m, sp), cf[m] + wa.rank(m, ep), dist, res);
		}
	}
};

} // cybozu

