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

#include "affix.h"
#include "word.h"

#include <iostream>
#include <algorithm>

using namespace xmunch;

/** Affix **/

/* Setup */

Affix::Affix(
		AffixGroup& grp,
		String pref,
		String suff,
		StringList preplace,
		StringList sreplace,
		int sco,
		Char scoid,
		bool virt
	) : group(grp), suffix(suff), prefix(pref), score(sco), score_id(scoid),
		has_virtual_stem(virt) {
	if (preplace.empty()) {
		stem_beginnings = {""};
	} else {
		stem_beginnings = preplace;
	}
	if (sreplace.empty()) {
		stem_endings = {""};
	} else {
		stem_endings = sreplace;
	}
}

void Affix::setStemEndings(StringList e) {
	if (e.empty()) {
		stem_endings = {""};
	} else {
		stem_endings = e;
	}
}
void Affix::setStemBeginnings(StringList b) {
	if (b.empty()) {
		stem_beginnings = {""};
	} else {
		stem_beginnings = b;
	}
}

/* Core */

void Affix::match(Index& words, WordList& vstems, Index& vindex, Word& w) {
	if (!w.matchable()) {
		return;
	}

	const String& s = w.getWord();
	auto begin = s.begin();
	auto end = s.end();

	if (!suffix.empty()) {
		auto m = std::mismatch(
				suffix.rbegin(),
				suffix.rend(),
				s.rbegin(),
				s.rend()
				);

		if (m.first != suffix.rend()) { // No match
			return;
		}

		end = m.second.base();
	}

	if (!prefix.empty()) {
		auto m = std::mismatch(
				prefix.begin(),
				prefix.end(),
				s.begin(),
				s.end()
				);

		if (m.first != prefix.end()) { // No match
			return;
		}

		begin = m.second;
	}

	if (begin >= end-1) { // Empty match or overlap
		return;
	}

	String stem(begin, end);
	for (auto& e : stem_endings) {
		for (auto& b : stem_beginnings) {
			handleMatch(words, vstems, vindex, b + stem + e, w);
		}
	}
}

void Affix::handleMatch(
				Index& words,
				WordList& vstems,
				Index& vindex,
				const String& stem,
				Word& w
) {
	Word * s;
	if (words.count(stem) == 1) {
		if (has_virtual_stem) { // don't "virtualize" a word from the word list.
			return;
		}
		s = &words.at(stem);
	} else if (has_virtual_stem) {
		if (vindex.count(stem) == 0) {
			vstems.emplace_back(stem);
			vindex.emplace(stem, vstems.back());
		}
		s = &vindex.at(stem);
	} else {
		return;
	}
	s->addAffix(group, *this, w);
	group.countMatch(*s, score, score_id);
}


/* Extra */

void Affix::print() {
	std::cerr << "AFF " << prefix << ":" << suffix 
		<< " (" << score_id << score << ") [";
	for (auto& b : stem_beginnings) {
		std::cerr << b << ',';
	}
	std::cerr << ":";
	for (auto& e : stem_endings) {
		std::cerr << e << ',';
	}
	std::cerr << "] " << (has_virtual_stem ? 'V' : ' ') << std::endl; 
}


/** AffixGroup **/


/* Setup */

AffixGroup::AffixGroup(int i, String n) 
	: id(i), name(n), auto_score(true), has_virtual_stem(false) {
		min_affix_score['*'] = 0;
}

String AffixGroup::stem_separator("/");
String AffixGroup::name_separator("");
String AffixGroup::virtual_marker("!");
void AffixGroup::setMarkers(const String& ss, const String& ns, const String& vm) {
	stem_separator = ss;
	name_separator = ns;
	virtual_marker = vm;
}


void AffixGroup::setVirtualStem(bool v) {
	has_virtual_stem = v;
}

void AffixGroup::addMinScore(int s, Char n) {
	if (s != 0) {
		auto_score = false;
	}
	min_affix_score[n] = s;
}

void AffixGroup::addAffix(
					String prefix,
					String suffix,
					StringList preplace,
					StringList sreplace,
					int score,
					Char score_id,
					bool as
	) {
	affixes.emplace_back(
			*this,
			prefix,
			suffix,
			preplace,
			sreplace,
			score,
			score_id,
			has_virtual_stem
		);
	if (min_affix_score.count(score_id) == 0) {
		std::cerr << "Affix error: group: " << name <<
			", affix: " << prefix << "-" << suffix 
			<< ", invalid score name " << score_id << std::endl;
	}
	if (auto_score && as) {
		min_affix_score[score_id] += score;
	}
}


/* Core */

void AffixGroup::match(Index& words, WordList& vstems, Index& vindex) {
	for (auto& w : words) {
		for (auto& a : affixes) {
			a.match(words, vstems, vindex, w.second);
		}
	}
	for (auto& m : match_scores) {
		bool valid = true;
		for (auto& s : m.second) {
			if (min_affix_score.at(s.first) > s.second) {
				valid = false;
				break;
			}
		}

		if (valid) {
			confirmStem(*m.first);
		}
	}
}

void AffixGroup::countMatch(Word& stem, int score, Char score_id) {
	if (match_scores.count(&stem) == 0) {
		auto& ms = match_scores[&stem];
		for (auto& a : min_affix_score) {
			ms.emplace(a.first, 0);
		}
	}
	match_scores.at(&stem).at(score_id) += score;
}

void AffixGroup::confirmStem(Word& stem) {
	stem.setStemFor(*this);
	for (auto& w : stem.getAffixesByGroup(*this)) {
		w.word.setHasStem(true);
	}
}

/* Extras */

void AffixGroup::print() {
	std::cerr << id << ": " << name << " (";
	for (auto& c : min_affix_score) {
		std::cerr << c.first << c.second << ",";
	}
	std::cerr << ") " << (has_virtual_stem ? 'V' : ' ') << " {" << std::endl;
	for (auto& a : affixes) {
		a.print();
	}
	std::cerr << "}\n" << std::endl;
}
