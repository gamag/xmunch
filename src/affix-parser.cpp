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

#include "affix-parser.h"
#include "affix.h"

#include <iostream>
#include <cctype>

using namespace xmunch;

AffixParser::AffixParser(std::ifstream& s, AffixGroupList& a) : src(s), affixes(a) {}

AffixParser::~AffixParser() {}

void AffixParser::parse() {
	id_counter = 0;
	skipWhite();

	if (src.get() != 'W') {
		std::cout << "Affix error: expected 'W' and output syntax spcifier." << std::endl;
		return;
	}
	String stem_sep = "";
	String aff_sep = "";
	String virt_mark = "";
	while (src.peek() != 'A' && src && !src.eof()) {
		stem_sep.push_back(src.get());
	}
	src.get(); // A
	while (src.peek() != 'A' && src && !src.eof()) {
		aff_sep.push_back(src.get());
	}
	src.get(); // A
	while (!std::isspace(src.peek()) && src && !src.eof()) {
		virt_mark.push_back(src.get());
	}
	AffixGroup::setMarkers(stem_sep, aff_sep, virt_mark);

	skipWhite();

	while (src && !src.eof()) {
		readGroup();
		skipWhite();
	}
}

void AffixParser::skipWhite(bool nonl) {
	std::string l;
	int c;
	while (src && !src.eof()) {
		c = src.peek();

		if (nonl) {
			if (c == ' ' || c == '\t') {
				src.get();
				continue;
			} else {
				return;
			}
		}

		switch (c) {
			case '#':
				std::getline(src, l);
				continue;
			case ' ':
			case '\t':
			case '\v':
			case '\n':
			case '\r':
			case '\f':
				src.get();
				continue;
			default:
				return;
		}
	}
}

void AffixParser::readGroup() {
	String in;

	String name;

	skipWhite();

	src >> name;

	affixes.emplace_back(id_counter++, name);

	AffixGroup& ag = affixes.back();

	readGroupFlags(ag);

	Char c;
	while (src && !src.eof()) {
		skipWhite();
		c = src.peek();
		if (c == '}') {
			src.get();
			break;
		}
		readAffix(ag);
	}
}

void AffixParser::readGroupFlags(AffixGroup& grp) {
	Char c;
	while (src && !src.eof()) {
		skipWhite();
		c = src.get();
		switch (c) {
			case '(':
				continue;
			case '{':
				return;
			case ')':
				skipWhite();
				c = src.get();
				if (c != '{') {
					std::cerr << "Affix file error: expected '{', found " << 
						src.peek() << " reading header of '" << grp.getName() <<
						"'" << std::endl;
				}
				return;
			case 'v':
			case 'V':
				grp.setVirtualStem(true);
				continue;
		}
		if (std::isdigit(c)) {
			src.putback(c);
			int sc;
			src >> sc;
			c = src.peek();
			if (std::isalpha(c)) {
				src.get();
			} else {
				c = '+';
			}
			grp.addMinScore(sc, c);
		}
	}
}

void AffixParser::readAffix(AffixGroup& grp) {
	StringList endings = readEndings();

	skipWhite();

	String name;
	src >> name;

	skipWhite();

	int score = 1;
	Char score_id = '+';

	if (src.peek() == '(') {
		src.get();

		while (src && !src.eof()) {
			skipWhite();
			Char c = src.peek();
			if (std::isdigit(c)) {
				src >> score;
				c = src.peek();
				if (std::isalpha(c)) {
					score_id = c;
					src.get();
				}
				continue;
			} else if (c == ')') {
				src.get();
				break;
			} else {
				std::cerr << "Affix error: expected ')' found " << c <<
					"while parsing " << name << " in " << grp.getName() << std::endl;
				break;
			}
		}
	}

	if (name[0] == '.') {
		name.erase(0, 1);
		bool autoscore = true;
		for (auto& e : endings) {
			grp.addAffix(e + name, score, score_id, {e}, autoscore);
			autoscore = false;
		}
	} else {
		grp.addAffix(name, score, score_id, endings);
	}
}


StringList AffixParser::readEndings() {
	StringList endings;
	std::string e("");
	Char c;
	while (src && !src.eof()) {
		c = src.get();
		switch (c) {
			case '.':
				if (!endings.empty()) {
					std::cerr << "'.' not allowed here, near " 
						<< endings.front() << std::endl;
				}
				endings.push_back("");
				return endings;
			case ',':
				endings.push_back(e);
				e = "";
				skipWhite();
				continue;
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				skipWhite();
				if (src.peek() == ',') {
					continue;
				} else {
					endings.push_back(e);
					return endings;
				}
			default:
				e.push_back(c);
		}
	}
	return endings;
}

