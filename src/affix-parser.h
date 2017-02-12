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

#ifndef _XMUNCH_AFFIX_PARSER_H_
#define _XMUNCH_AFFIX_PARSER_H_ 

#include "xmunch.h"

#include <fstream>

namespace xmunch {
	class AffixParser {
		std::ifstream& src;

		AffixGroupList& affixes;

		int id_counter;

		public:

			AffixParser(std::ifstream& s, AffixGroupList& a);
			~AffixParser();

			void parse();

		protected:

			void skipWhite(bool no_newline = false);

			// suffix shouldn't contain .
			// . in prefix will be handled.
			void handleAddPrefix(
					AffixGroup& grp,
					String prefix,
					const String& suffix,
					int score,
					Char score_id,
					const std::list<String>& beginnings,
					const std::list<String>& endings,
					bool auto_score
				);

			String readAffixString();
			void readGroup();
			void readGroupFlags(AffixGroup& grp);

			void readAffix(AffixGroup& grp);

			StringList readEndings();

	};
}

#endif /* ifndef _XMUNCH_AFFIX_PARSER_H_ */
