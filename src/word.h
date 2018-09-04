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
#include <set>
#include <fstream>

namespace xmunch {

	struct AffixedWord {
		Word& word;
		const Affix& affix;

		AffixedWord(Word& w, const Affix& f) : word(w), affix(f) {}
	};

	class Word {

		String word;

		bool has_stem;

		std::map<AffixGroup*, AffixedWordList> affixes;

		std::set<AffixGroup*> stem_of;

		StemType is_type;

		public:
			Word(String w) : word(w), has_stem(false), is_type(StemType::NORMAL) {};

			~Word() {};

			const String& getWord() const { return word; }

			bool isStem() const { return !stem_of.empty(); }
			bool isStemOf(AffixGroup& group) const { return stem_of.count(&group) == 1; }
			bool hasStem() const { return has_stem; }
			bool matchable() const { return stem_of.empty() && !has_stem; }

			void addAffix(AffixGroup& g, const Affix& f, Word& w) { affixes[&g].emplace_back(w, f); }

			bool hasAffixOfGroup(AffixGroup& group) const { return affixes.count(&group) == 1 && affixes.at(&group).size() != 0; }
			AffixedWordList& getAffixesByGroup(AffixGroup& group) { return affixes[&group]; }

			void setStemFor(AffixGroup& affix) { stem_of.insert(&affix); }
			void setHasStem(bool hs) { has_stem = hs; }

			void setStemType(StemType t) { is_type = t; }
			StemType getStemType() const { return is_type; }

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
				if (is_type == StemType::VIRTUAL || is_type == StemType::OPTIONAL) {
					out << (first ? String("") : AffixGroup::getAffSep()) << AffixGroup::getVirtMark();
				}
				out << std::endl;
			}

			void format_uncompressed(std::ostream& out) {
				out << word;
				if (stem_of.empty()) {
					out << ";" << std::endl;
					return;
				}

				if (is_type == StemType::VIRTUAL) {
					out << "@V";
				} else if (is_type == StemType::OPTIONAL) {
					out << "@O";
				} else if (is_type == StemType::CREATE) {
					out << "@C";
				}
				out << " {" << std::endl;

				for (auto ag : stem_of) {
					out << "\t" << ag->getName() << " {" << std::endl;
					for (auto a : getAffixesByGroup(*ag)) {
						if (a.word.isStem()) {
							continue;
						}

						out << "\t\t" << a.word.word << std::endl;
					}
					out << "\t}" << std::endl;
				}

				out << "};" << std::endl;
			}
	};
}

#endif /* ifndef _XMUNCH_WORD_H_ */
