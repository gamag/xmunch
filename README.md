
# xmunch - word-list affix compression tool using affix definitions #

xmunch essentially does, what the 'munch' command of, for example, hunspell
does, but is not compatible with hunspell affix definition files. So why use it
then? What makes xmunch different from the other tools is the ability to extend
incomplete word-lists. For hunspell's munch to identify a stem and add an
affix mark, every word formed by the affix with the stem has to be in
the original word-list. This makes sense for a compression tool. However if
your word list is incomplete, you have to add all possible word-forms of a
word first, before any compression is done. Using xmunch instead, you can
define a subset of forms which are required to be in the word-list to allow the
word to be used as stem. Like this, you can extend the word-list. The process
is a bit like using an incomplete affix definition file with munch and a
complete one later in hunspell. Since this identification of word forms has to
be done quite carefully, xmunch supports a score system for word forms. This gives
you a flexibility which incomplete affix definitions used with munch would never
provide. 

xmunch has originally been written to improove the georgien spell checking dictionary
[https://github.com/gamag/ka_GE.spell]

Note: At the moment, xmunch doesn't change the encoding of input, so make sure
to use the same everywhere.  Internally it uses 8 byte chars, so I'd suggest
using UTF-8, maybe C-normalized.

## Build ##

To build xmunch, you need a c++14 capable compiler and `make`.

in a terminal, run `make`

## Usage ##

`xmunch [wordlist] [affixes] [output] [premunched] [options]`

where:

- `[wordlist]` is the filename of the word list or - for standard input
- `[affixes]` is the affix definition file
- `[output]` ist the filename of an output file or - for standard output
- `[premunched]` is an optional file with stem-affix combinations used as basis
  for the xmunch run.
- `[options]` are optional options:
  - `--print-tree` prints the parsed affix-definitions to standard error output.
  - `--no-compression` writes to `[output]` in an uncompressed format, that can be
	used as `[premunched]` input file

wordlist should contain the number of words in the first line and then one
word per line. Omitting the number will slow down the loading process.

## Affix file syntax ##

Here a small example affix file, showing all available features.
You are expected to know how the hunspell .aff files work in order to understand
the explanations below.

```CONF

# use # for comments.

# Output syntax: 
# W[stem-affix-separator]A[affix-affix-separator]A[needs affix marker] 
# This is always required and indicates, how the output file should be
# formatted. It will always have one word per line. Following the
# definition below, a stem will be separated form its affix marks by [stem-affix-separator]
# different affix marks will be separated by [affix-affix-separator] and virtual stems will be
# marked using [needs affix marker].

W/A,A!

# In the output this would give for example:
# 'house/n,p' 
# For compatibility with de_DE.aff on my system, you would need
# W/AAh

# Affix definitions:
# To understand the affix definition syntax, you have to think the other way around as
# when writing a .aff file. Think in terms of affixes which are are stripped from
# the word and replaced by an ending to form a stem. xmunch doesn't support regex
# conditions like in .aff files (may be implemented later). 

# lets start with suffixes, the default case, to show the general syntax.

## Basic affix group
# The N corresponds to the N in 
# SPF N Y 5 
N {
.       e   # to form a stem, e is stripped, then the result is looked up in the
			# word-list. If it exists, we have a match, and the word is taken
			# as probably derived from the stem.

x,y     ch  # here ch will be stripped and x or y will be added to build the stem.

x,y     .a  # here . is replaced by x or y before stripping the affix (xa or ya)
			# then x or y, depending on the stripped affix, is added again to
			# form the stem the stem. This is equivalent to just removing the
			# 'a' and checking that the resulting stem ends in x or y.
			# This is a less powerful replacement for hunspell's regex
			# conditions. . can only appear as the first character in an affix.
}
# For the group above to be used, all derived words it checks for have to
# appear in the word list. So essentially, this behaves like hunspell's munch
# would.
# 
# abcx
# abcxe
# abcch
# abcxa
# 
# will be compressed to
# abcx/N
# 
# If one of them is missing, nothing is done.
#
# Note that abcch is a valid form of abcx and abcy. Xmunch uses a word only
# once (except for stems). Therefore, after the stem abcx, you will never get
# abcy as stem, since abcch is already derived from abcx. Which stem xmunch
# takes if all forms of abcx and abcy are available, is undefined.

## Virtual stems 
# Lets imagine the following word list
# abca
# abcb
# abcc
# Here it would be useful to allow compression using abc as stem, but since this
# abc is not in the word list, normal affix compression doesn't work. So we need
# to add abc as virtual stem to the list and mark it as wrong using the
# NEEDAFFIX flag of hunspell, which, is in this file, is defined to
# be !. Then we need to tell xmunch, that it is allowed to create stems if
# needed: (W has the same meaning as N in the first rule)
W (v) {
.        aa
.        bb
.        cc
}
# this will result in 
# abc/W!
#
# Instead of v, two other modifiers can be used: c and o. c lets xmunch create
# a missing stem without adding a virtual/need-affix flag, and o (for optional
# stem) is something in between v and c. If a matching stem is only virtual, 
# it will be used, as opposed to c. If needed, a new virtual stem is created
# like with v, but if a matching stem is found in the word list, that one is
# used without need-affix flag (v would not match in this case). 
#
# A normal stem can match definitions with c and o at the same time and might be
# created if needed (note that normal (without v,c,o) affix definition will not use 
# a stem created by c or o). A virtual stem can only match definitions with v and o.
#
# for affix definitions, generally the same syntax as in N is supported. Note
# however, that they might behave unexpected with virtual stems, since they are
# created as needed. For example x,y ch from above would create for every word ending
# with ch two stems, one ending in x, one in y. 

## Score system
# Lets look at what makes xmunch different. Say we have a wordlist
# lai
# laa
# la
# but our imaginary language has more derivatives of the stem la:
# lac, las, larr, lau
# xmunch is now able to compress the word list, even though some words are
# missing:
E (2) {
a   .i
a   .a
a   .c
a   .s
a   .rr
a   .u 
}
# Here (2) tells xmunch to require only two matches from the affixes defined in
# the E group so it will compress the list above to 
# la/E
# In the same way
# xvau
# xvarr
# xvai
# xva
# will be compressed to xva/E. The given rule requires the stem to end in a.
# This functionality can be combined with virtual stems, just use (2 v) .

# Actually, xmunch does not simply count matches, but counts scores for each
# match. If nothing is given, xmunch assumes score 1 for a match, but you can
# define the score to be different:
B (2) {
b   .i      (1)
b   .a      (2) 
b   .c      (1)
b   .s      (1)
b   .rr     (2)
b   .u      (1)
}
# this behaves more or less like the E rule, requiring the stem to end in b,
# except for two cases: the word lists
# oba
# ob
#  and 
# wbrr
# wb
# will be compressed by xmunch to ob/B and wb/B even though there was only
# one match. The total match score is still >= 2.
#
# If this is not flexible enough to avoid false-positives, scores can be
# grouped. Like this, the score limit has to be reached in each group for the
# stem to be used.
C (1a 2b) {
c   .i      (1a)
c   .a      (2a) 
c   .c      (1b)
c   .s      (1b)
c   .rr     (2b)
c   .u      (1b)
}
# here 1a and 2b mean that a total match score of 1 in group a and two in group
# b needs to be reached for a stem to be valid with this affix, where the group
# name is one letter. If no name is given, xmunch uses *.
# This means, we require the stem to be in the word list (can be avoided using
# (1a 2b v)), also we require a form ending in a or i, and one ending in rr or
# in two of c, s or u.
#
# It is possible to use negative scores to forbid the existence of some forms
# in the word list.

## Prefixes
# Until here, we only spoke of suffixes. The same functionality can be used on
# prefixes too:
PR (3) {
a    b.-    (1)
a    cc-    (2)
.    f-     (1)
.    abc-   (2)
}
# here, the - after the affix text marks the affix as prefix, everything else
# works like above (the implicit score name has been used). You could use - in
# front of the affix instead to mark suffixes, but that is optional.

## Circumfixes
# In the same way as prefixes, circumfixes can be defined:
CX {
a:.    b-e
a,b,c : x,y,z     beg.-.end
. : .     abeg-aend
}
# Here, before the : is the stem ending/affix replacement definition for the
# prefix, after it the one for the suffix (they are both required), afterwards
# we have prefix-suffix. This can of course be combined with everything
# explained before like scoring and virtual stems.
#
# Note that there are no spaces allowed before and after the dash (-)
#
# Prefix, suffix and circumfix definitions can be mixed in one group like CX.
#
```

## Manual tuning and pre-munched input ##

xmunch might not always create optimal output, and you might want to give it
some hints or force some grammatical exceptions that can't be described
efficiently in the affix definition. Also, when creating a hunspell dictionary,
it is important to have a way to review its content and merge the reviewed data
with the word-list. Here the "pre-munched" input comes in handy.

Running xmunch with `--no-compression` makes it print a file where all stems
xmunch found are grouped together with their derived forms. This might be
easier to review than the final dictionary and can be feed back to xmunch. If
such pre-munched data is given, xmunch will take it as basis and try to combine
words from the word-list with stems already defined in the pre-munched data
before searching for new stems in the word-list. For this pre-munched stems,
all score checks are disabled.

The pre-munched data has the following format:

```conf
# basically, this is a word list:
word1;
word2; # if a word is missing in the input word-list, it will be added from here.
word3;
# With stems, it looks like this:
stemx {
	B { # B is a affix group, which has to be defined in the affix file.
		stemxa # this words are derived from stemx following the rules defined
		stemxb # by B in the affix file. However, they are never checked if
		vstemb # they actually match - so this can be used to force xmunch to
		abcede # make some exceptions.
	}
	C {} # C has to be defined in the affix file too, and if matching derived
		 # words are found, they will be added here, ignoring any score counts.
		 # Even if no derived forms exist in the word-list, the resulting
		 # dictionary will have
		 # stemx/B,C
};
asdf;
sdfa;
word5@v { # @v means this is a virtual stem. If it exists in the word-list, it
		  # will be removed from there. 
		  # @o might be used if the stem should only be virtual if it doesn't
		  # exist in the word-list. (So it is not removed if it exists)
	W {
		word5x	
	}
};
# All this is just take as basis for the xmunch run - so word1 for example
# might be found to be derived from a word in the word-list, or might be taken
# as stem of some words in there. Also, other derived forms of even other affix
# definition groups might be added to stemx or word5.
```

## License ##

xmunch is relased under GNU GPLv3

