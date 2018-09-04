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

#ifndef _XMUNCH_PREMUNCHED_LOADER_H_
#define _XMUNCH_PREMUNCHED_LOADER_H_ 

#include "xmunch.h"

#include <fstream>

namespace xmunch {
	class PremunchedLoader {
		std::ifstream& src;

		WordList& words;
		Index& index;

		WordList& vwords;
		Index& vindex;

		AffixGroupList& affixes;

		int id_counter;

		public:

			PremunchedLoader(
					std::ifstream& input,
					AffixGroupList& a,
					WordList& w,
					Index& wi,
					WordList& virtual_w,
					Index& virtual_wi
				);

			~PremunchedLoader();

			void load();

		protected:

			void skipWhite(bool no_newline = false);

			Word* loadWord();

			String readWord();

			void loadDerivedList(Word& stem);
	};

	// Implemented in affix_parser.cpp
	void skip_over_whitespace(std::istream& s, bool no_newline /* = false */);
}

#endif /* ifndef _XMUNCH_PREMUNCHED_LOADER_H_ */
