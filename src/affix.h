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

	enum class StemType : char {
		VIRTUAL = 'v', // Stem must not exist in word list.
		OPTIONAL = 'o', // If stem is missing, use a virtual one.
		CREATE = 'c', // If stem is missing, create it.
		NORMAL = 'n',
		UNDEFINED = 'u' // Only to be used in word objects
	};

	class Affix {
		AffixGroup& group;	

		String suffix;
		String prefix;

		int score;
		Char score_id;

		StemType stem_type;

		StringList stem_beginnings;
		StringList stem_endings;

		public:
			Affix(
					AffixGroup& grp,
					String pref,
					String suff,
					StringList preplace,
					StringList sreplace,
					int sco,
					Char scoid,
					StemType stype
				);

			void setStemEndings(StringList e);
			void setStemBeginnings(StringList b);

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

		StemType stem_type;
		
		std::list<Affix> affixes;

		std::map<Word*, std::map<Char, int> > match_scores;

		static String stem_separator;
		static String name_separator;
		static String virtual_marker;

		public:

			AffixGroup(int i, String n);

			void addAffix(
					String prefix,
					String suffix,
					StringList preplace,
					StringList sreplace,
					int score,
					Char score_id,
					bool autoscore = true // Allow auto score.
					);
			void setStemType(StemType t);
			void addMinScore(int s, Char n);

			static void setMarkers(
					const String& ss,
					const String& ns,
					const String& vm
					);

			const String& getName() { return name; };
			StemType getStemType()   { return stem_type; }
			static const String& getStemSep()  { return stem_separator; };
			static const String& getAffSep()   { return name_separator; };
			static const String& getVirtMark() { return virtual_marker; };

			void match(Index& words, WordList& vstems, Index& vindex);
			void countMatch(Word& stem, int score, Char score_id);
			void confirmStem(Word& stem);

			bool isMatchingStemType(StemType tword);
			StemType getNewStemType(StemType told);

			void print();
	};
}

#endif /* ifndef _XMUNCH_AFFIX_H_ */
