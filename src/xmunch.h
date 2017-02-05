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

#ifndef _XMUNCH_XMUNCH_H_
#define _XMUNCH_XMUNCH_H_ 

#include <string>
#include <list>
#include <map>
#include <unordered_map>

namespace xmunch {
	class Word;
	class Affix;
	class AffixedWord;
	class AffixGroup;

	typedef char Char;
	typedef std::string String;

	typedef std::unordered_map<String, Word&> Index;
	typedef std::list<Word> WordList;

	typedef std::list<String> StringList;

	typedef std::list<AffixGroup> AffixGroupList;

	typedef std::list<AffixedWord> AffixedWordList;
}

#endif /* ifndef _XMUNCH_XMUNCH_H_ */
