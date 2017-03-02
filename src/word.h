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

#ifndef _XMUNCH_WORD_H_
#define _XMUNCH_WORD_H_

#include "xmunch.h"
#include "affix.h"
#include <map>
#include <list>
#include <fstream>

namespace xmunch {

	class Affix;

	struct AffixedWord {
		Word& word;
		Affix& affix;

		AffixedWord(Word& w, Affix& f) : word(w), affix(f) {}
	};

	class Word {

		String word;

		bool has_stem;

		std::map<AffixGroup*, AffixedWordList> affixes;

		std::list<AffixGroup*> stem_of;

		public:
			Word(String w) : word(w) {};

			~Word() {};

			String& getWord() { return word; }

			bool isStem() { return !stem_of.empty(); }
			bool hasStem() { return has_stem; }
			bool matchable() { return stem_of.empty() && !has_stem; }

			void addAffix(AffixGroup& g, Affix& f, Word& w) { affixes[&g].emplace_back(w, f); }

			bool hasAffixOfGroup(AffixGroup& group) { return affixes.count(&group) > 1; }
			AffixedWordList& getAffixesByGroup(AffixGroup& group) { return affixes.at(&group); }

			void setStemFor(AffixGroup& affix) { stem_of.push_back(&affix); }
			void setHasStem(bool hs) { has_stem = hs; }

			void format(std::ostream& out) {
				out << word;
				if (stem_of.empty()) {
					out << std::endl; 
					return;
				}
				out << AffixGroup::getStemSep();
				bool first = true;
				for (auto a : stem_of) {
					out << (first ? String("") : AffixGroup::getAffSep()) << a->getName();
					first = false;
				}
				if (stem_of.front()->hasVirtualStem()) {
					out << (first ? String("") : AffixGroup::getAffSep()) << AffixGroup::getVirtMark();
				}
				out << std::endl;
			}
	};
}

#endif /* ifndef _XMUNCH_WORD_H_ */
