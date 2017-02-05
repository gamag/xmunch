/**
 * This file is part of xmunch
 * Copyright (C) 2017 Gabriel Margiani
 *
 * xmunch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xmunch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xmunch.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <iostream>
#include <fstream>


#include "xmunch.h"
#include "word.h"
#include "affix.h"
#include "affix-parser.h"

using namespace xmunch;

void load_wordlist(std::istream& in, WordList& words, Index& index) {
	String l;
	std::getline(in, l);

	int wordcount= 500;
	try {
		wordcount = std::stoi(l) + 1000;
		std::getline(in, l);
	} catch (std::invalid_argument e) {
		std::cerr << "WARNING, dictionary file should contain the number of words in the first line." << std::endl;
	}

	index.reserve(wordcount);

	do {
		words.emplace_front(l);
		index.emplace(words.begin()->getWord(), *(words.begin()));
	} while (std::getline(in, l));
}

void work(std::istream& in, std::ifstream& aff, std::ostream& out) {

	WordList words;
	Index index;

	WordList virtual_stems;
	Index virtual_index;

	AffixGroupList affixes;

	load_wordlist(in, words, index);

	AffixParser afp(aff, affixes);
	afp.parse();

	for (auto& a : affixes) {
		a.print();
	}

	for (auto& a: affixes) {
		a.match(index, virtual_stems, virtual_index);
	}

	for (auto& w : words) {
		if (w.hasStem()) {
			continue;
		}
		w.format(out);
	}
	for (auto& w : virtual_stems) {
		if (w.isStem()) {
			w.format(out);
		}
	}
}

void print_help() {
	std::cerr << "Usage: xmunch wordlist affixes output\n"
		<< "if output or word-list are -, read from/write to standard streams." << std::endl;
}

int main(int argc, char * argv[]) {
	if (argc != 4) {
		print_help();
		return 1;
	}
	
	std::istream* in;
	std::ifstream* aff;
	std::ostream* out;

	std::string a = argv[1];
	if (a == "--help" || a == "-h") {
		print_help();
		return 0;
	} else if (a == "-") {
		in = &std::cin;
	} else {
		in = new std::ifstream(a);
		if (in->fail()) {
			std::cerr << "couldn't open " << a << std::endl;
			return 1;
		}
	}

	a = argv[2];
	if (a == "--help" || a == "-h") {
		print_help();
		return 0;
	} else {
		aff = new std::ifstream(a);
		if (aff->fail()) {
			std::cerr << "couldn't open " << a << std::endl;
			return 1;
		}
	}

	a = argv[3];
	if (a == "--help" || a == "-h") {
		print_help();
		return 0;
	} else if (a == "-") {
		out = &std::cout;
	} else {
		out = new std::ofstream(a);
		if (out->fail()) {
			std::cerr << "couldn't open " << a << std::endl;
			return 1;
		}
	}

	work(*in, *aff, *out);

	if (in != &std::cin) {
		delete in;
	}
	delete aff;
	if (out != &std::cout) {
		delete out;
	}
}

