/**
 * This file is part of xmunch
 * Copyright (C) 2018 Gabriel Margiani
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

#include "premunched-loader.h"

#include "word.h"
#include "affix.h"

#include <iostream>
#include <algorithm>


using namespace xmunch;

PremunchedLoader::PremunchedLoader(
					std::ifstream& input,
					AffixGroupList& a,
					WordList& w,
					Index& wi,
					WordList& virtual_w,
					Index& virtual_wi
				):
src(input), words(w), index(wi), vwords(virtual_w), vindex(virtual_wi), affixes(a) {}

PremunchedLoader::~PremunchedLoader() {}

void  PremunchedLoader::load() {
	skipWhite();
	while (src && !src.eof()) {
		Word* w = loadWord();
		skipWhite();

	 	Char c = src.get();

		if (c == '{') {
			loadDerivedList(*w);
			skipWhite();
			c = src.get();
		}

		if (c != ';') {
			std::cerr << "Error in premunched input, expected ';' got '" <<
				c << "' near '" << w->getWord() << "'" << std::endl;
		}
		skipWhite();
	}
}

Word* PremunchedLoader::loadWord() {
	String s = readWord();

	bool virt = false;
	StemType type = StemType::NORMAL;
	if (src.peek() == '@') {
		src.get();
		switch (src.get()) {
			case 'c':
			case 'C':
				type = StemType::CREATE;
				break;
			case 'o':
			case 'O':
				type = StemType::OPTIONAL;
				break;
			case 'v':
			case 'V':
				type = StemType::VIRTUAL;
				virt = true;
				break;
			case 'n':
			case 'N':
				type = StemType::NORMAL;
				break;
			default:
				std::cerr << "Error in premunched input, expected @v, @o or @c near '" << s << "'" << std::endl;

		}
	}

	Word* ret = nullptr;
	if (index.count(s) > 0) {
		if (virt) {
			// Remove - this word has to be virtual
			Word& tmp = index.at(s);
			index.erase(s);
			words.erase(
					std::find_if(
						words.begin(), 
						words.end(), 
						[tmp] (Word& a) { return a.getWord() == tmp.getWord(); }
					)
				);
		} else {
			ret = &index.at(s);
			if (type == StemType::OPTIONAL || type == StemType::CREATE) {
				type = StemType::NORMAL;
			}
		}
	} else if (type == StemType::OPTIONAL) {
		// Since the word is not in the word list, it is virtual...
		// Note that in the affix matching process, CREATE is handled the same way.
		virt = true;
	}

	if (ret == nullptr) {
		if (virt) {
			vwords.emplace_front(s);
			ret = &(*vwords.begin());
			vindex.emplace(ret->getWord(), *ret);
		} else {
			words.emplace_front(s);
			ret = &(*words.begin());
			index.emplace(ret->getWord(), *ret);
		}
	}

	ret->setStemType(type);

	return ret;
}

String PremunchedLoader::readWord() {
	String ret("");
	Char c = 0;

	while (src && !src.eof()) {
		c = src.peek();

		if (c < 31 /* control chars */ ||
				c == ' ' || c == '#' ||
				c == ';' || c == ',' ||
				c == '@' || c == ':' ||
				c == '{' || c == '}' ||
				c == '"' || c == '"'
				) {
			// There are of course more non-word characters, but we only care
			// about functional ones in our syntax.
			break;
		}

		ret.push_back(src.get());
	}

	return ret;
}

void PremunchedLoader::loadDerivedList(Word& stem) {
	skipWhite();
	while (src && !src.eof() && src.peek() != '}') {
		String a = readWord();
		auto i = std::find_if(
				affixes.begin(),
				affixes.end(),
				[a](const AffixGroup& g) { return g.getName() == a; }
			);
		if (i == affixes.end()) {
			std::cerr << "Error in premunched input, invalid affix group name: '" << a << "'" << std::endl;
			String tmp;
			std::getline(src, tmp, '}');
			std::cerr << "The following content has been ignored: " << std::endl << tmp << std::endl;
			continue;
		}

		AffixGroup& g = *i;

		stem.setStemFor(g);

		skipWhite();
		if (src.get() != '{') {
			std::cerr << "Error in premunched input, expected { after " << a << std::endl;
		}

		skipWhite();
		while (src && !src.eof() && src.peek() != '}') {
			String w = readWord();
			Word* derived;
			if (index.count(w) == 0) {
				words.emplace_front(w);
				derived = &*words.begin();
				index.emplace(derived->getWord(), *derived);
			} else {
				derived = &index.at(w);
			}

			derived->setHasStem(true);
			stem.addAffix(
				g, 
				Affix(g, "", "", {}, {}, 0, 0, g.getStemType()), 
				*derived
			);

			skipWhite();
		}

		src.get(); // }
		skipWhite();
	}
	src.get(); // }
}

void PremunchedLoader::skipWhite(bool neof) {
	skip_over_whitespace(src, neof);
}
