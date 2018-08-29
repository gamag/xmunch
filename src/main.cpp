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

void work(std::istream& in, std::ifstream& aff, std::ostream& out, bool print_tree, bool no_compression) {

	WordList words;
	Index index;

	WordList virtual_stems;
	Index virtual_index;

	AffixGroupList affixes;

	load_wordlist(in, words, index);

	AffixParser afp(aff, affixes);
	afp.parse();

	if (print_tree) {
		for (auto& a : affixes) {
			a.print();
		}
	}

	for (auto& a: affixes) {
		a.match(index, virtual_stems, virtual_index);
	}

	for (auto& w : words) {
		if (w.hasStem()) {
			continue;
		}
		if (no_compression) {
			w.format_uncompressed(out);
		} else {
			w.format(out);
		}
	}
	for (auto& w : virtual_stems) {
		if (!w.isStem()) {
			continue;
		}
		if (no_compression) {
			w.format_uncompressed(out);
		} else {
			w.format(out);
		}
	}
}

void print_help() {
	std::cerr << "Usage: xmunch wordlist affixes output [options]\n"
		<< "if output or word-list are -, read from/write to standard streams.\n"
		<< "--print-tree to print the parsed affix definitions to stderr\n"
		<< "--no-compression to do no affix compression, output derivatives grouped with their stems\n" << std::endl;
}

int main(int argc, char * argv[]) {
	bool print_tree = false;
	bool no_compression = false;

	std::istream* in = nullptr;
	std::ifstream* aff = nullptr;
	std::ostream* out = nullptr;

	// parse arguments
	int fi = 0;
	for (int i = 1; i < argc; i++) {
		std::string a(argv[i]);

		if (a == "--help" || a == "-h") {
			print_help();
			return 0;
		} else if (a == "--print-tree") {
			print_tree = true;
			continue;
		} else if (a == "--no-compression") {
			no_compression = true;
			continue;
		}

		switch (fi) {
			case 0: // word list
				if (a == "-") {
					in = &std::cin;
				} else {
					in = new std::ifstream(a);
					if (in->fail()) {
						std::cerr << "couldn't open word list: " << a << std::endl;
						return 1;
					}
				}
				break;
			case 1: // affix definitions
				aff = new std::ifstream(a);
				if (aff->fail()) {
					std::cerr << "couldn't open affix definition file: " << a << std::endl;
					return 1;
				}
				break;
			case 2: // output
				if (a == "-") {
					out = &std::cout;
				} else {
					out = new std::ofstream(a);
					if (out->fail()) {
						std::cerr << "couldn't open output file: " << a << std::endl;
						return 1;
					}
				}
				break;
			default:
				std::cout << "Too many arguments." << std::endl;
				print_help();
				return 1;
		}
		fi++;
	}

	// do the work
	work(*in, *aff, *out, print_tree, no_compression);

	// clean up
	if (in && in != &std::cin) {
		delete in;
	}
	if (aff) {
		delete aff;
	}
	if (out && out != &std::cout) {
		delete out;
	}
}

