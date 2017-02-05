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

#ifndef _XMUNCH_AFFIX_H_
#define _XMUNCH_AFFIX_H_ 

#include "xmunch.h"

#include <list>

namespace xmunch {

	class Affix {
		AffixGroup& group;	

		String value;

		int score;
		Char score_id;

		bool has_virtual_stem;

		StringList stem_endings;

		public:
			Affix(AffixGroup& g, String v, int sc, Char sn, bool sv, StringList e);

			void setStemEndings(StringList e);

			void match(Index& words, WordList& vstems, Index& vindex, Word& word);

			void print();

		protected: 
			void handleMatch(
					Index& words,
					WordList& vstems,
					Index& vindex,
					const String& stem,
					Word& w
					);
	};

	class AffixGroup {
		int id;

		String name;
		
		bool auto_score;
		std::map<Char, int> min_affix_score;

		bool has_virtual_stem;
		
		std::list<Affix> affixes;

		std::map<Word*, std::map<Char, int> > match_scores;

		static String stem_separator;
		static String name_separator;
		static String virtual_marker;

		public:

			AffixGroup(int i, String n);

			void addAffix(
					String value,
					int score,
					Char score_id,
					StringList endings,
					bool autoscore = true // Allow auto score.
					);
			void setVirtualStem(bool v);
			void addMinScore(int s, Char n);

			static void setMarkers(
					const String& ss,
					const String& ns,
					const String& vm
					);

			const String& getName() { return name; };
			bool hasVirtualStem()   { return has_virtual_stem; }
			static const String& getStemSep()  { return stem_separator; };
			static const String& getAffSep()   { return name_separator; };
			static const String& getVirtMark() { return virtual_marker; };

			void match(Index& words, WordList& vstems, Index& vindex);
			void countMatch(Word& stem, int score, Char score_id);
			void confirmStem(Word& stem);

			void print();
	};
}

#endif /* ifndef _XMUNCH_AFFIX_H_ */
