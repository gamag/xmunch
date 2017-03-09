
# xmunch - word-list affix compression tool using affix definitions #

xmunch essentially does, what the 'munch' command of, for example, hunspell
does, but is not compatible with hunspell affix definition files. So why use it
then?  What makes xmunch different from the other tools is the ability to extend
incomplete word-lists. For hunspells munch to identify a stem and add an
affix mark, every word created by the affix mark with the stem has to be in
the original word-list. This makes sense for an compression tool, but if
your word list is incomplete, you have to add all possible word-forms of a
word first, before any compression is done. Using xmunch instead, you can
define a subset forms that are required to be in the word-list to allow the
word to be used as stem and extend you word-list like that, a bit like using
an incomplete affix definition file with munch and a complete one later.
Since this identification of word forms has to be done quite carefully,
xmunch supports a score system for word forms, giving you a flexibility,
incomplete affix definitions used with munch would never provide. 

xmunch has originally been written to improoving the georgien spell checking dictionary
[https://github.com/gamag/ka_GE.spell]

Note: At the moment, xmunch doesn't change the encoding of input, so make sure
to use the same everywhere.  Internally 8 byte chars, so I'd suggest
using UTF-8, maybe C-normalized.

## Build ##

To build xmunch, you need a c++14 capable compiler and `make`.

in a terminal, run `make`

## Usage ##

`xmunch [wordlist] [affixes] [output]`

where all arguments are filenames. If output or wordlist are 
'-', xmunch uses standard input and standard output respectively.

wordlist should contain the number of words in the first line and then one
word per line. Omitting the number will slow down the loading process.

## Affix file syntax ##

Here a small example affix file, showing all available features.
You are expected to know how the hunspell .aff files work in order to understand
the explanations below.

```CONF

# use # for comments.

# Output syntax: W [stem-affix-separator] A [affix-affix-separator] A
# [virtual stem/needs affix marker] this is always required and indicates, how the
# output file should be formatted. It will always have one word per line, and,
# following the definition below, as stem will be separated form its affix marks
# by a / different affix marks will be separated by a , and virtual stems
# will be marked using '!'  so for example
# 'house/n,p' 
W/A,A!
# For compatibility with de_DE.aff on my system, you would need
# W/AAh

# The for the affix definition syntax, you have to think the other way around as
# when writing a .aff file. Think in terms of affixes which are are stripped from
# the word and replaced by an ending to form a stem. xmunch doesn't support regex
# conditions like in .aff files (may be implemented later). 

# lets start with suffixes, the default case, to show the general syntax
## Basic affix group
# The N correspond to the N in 
# SPF N Y 5 
N {
.       e   # to form a stem, e is stripped, then then this stem is looked up in the
            # word-list. If it exists, we have a match.

x,y     ch  # here ch will be stripped and x or y will be added to build the stem.

x,y     .a  # here . is replaced by x or y before stripping the affix (xa or ya)
            # then x or y, depending on the stripped affix, is added again for
            # the stem. This actually means only stems ending in x or y can have
            # a form ending with a appended, and is a, little less powerful,
            # replacement for regex conditions.
            # . can only appear as the first character in an affix.
}
# For the group above to be used, all words it checks have to appear in the word
# list, so essentially, this behaves like hunspell munch would.
# 
# abcx
# abcxe
# abcch
# abcxa
# 
# will be compressed to
# abcx/N
# 
# if one of them is missing, nothing is done.
# Note that abcch is a valid form of abcx and abcy. Xmunch uses a word-form only
# once (except for stems) so after abcx you will never get abcy since abcch is
# missing. Which one xmunch would take if all forms of abcx and abcy are
# available, is undefined.

## Virtual stems 
# Lets imagine the following word list
# abca
# abcb
# abcc
# Here it would be useful to allow compression using abc as stem, but since this
# abc is not in the word list, normal affix compression doesn't work. So we need
# to add abc as virtual stem to the list and mark it as wrong using
# NEEDAFFIX flag of hunspell for example, which is in this file defined to
# be !. Then we need to tell xmunch, that it is allowed to create stems if
# needed: (V has the same meaning as N in the first rule)
V (v) {
.        aa
.        bb
.        cc
}
# this will result in 
# abc/V!
#
# for affix definitions, generally the same syntax as in N is supported, but
# note, that it might behave unexpected with virtual stems, since they are
# created as needed - so x,y ch from above would create for every word ending
# with ch two stems, one ending in x, one in y. 

## Score system
# Lets look at what makes xmunch different. Say we have a wordlist
# lai
# laa
# la
# but our imaginary language has more derivative forms of the stem la:
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
# Here (2) tells xmunch, to require only two matches from the affixes defined in
# the group so it will compress the list above to 
# la/E
# in the same way
# xvau
# xvarr
# xvai
# xva
# will be compressed to xva/E. The given rule requires the stem to end in a.
# This functionality can be combined with virtual stems, just use (2 v).

# Actually, xmunch doesn't simply count matches, but counts scores for each
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
# this behaves more or less the same then the E rule requiring the stem to end
# in b, except for two cases: the word lists
# oba
# ob
#  and 
# wbrr
# wb
# will be compressed by xmunch to ob/B and wb/B even though there was only
# one match - since the total match score is still >= 2.
#
# If this is not flexible enough to avoid false-positives, scores can be
# grouped, and the score limit has to be reached in each group for the stem to
# be used.
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
# name is one letter. 
# this means, we require the stem to be in the word list (can be avoided using
# (1a 2b v)), then a form ending in a or i, and one ending in rr or in two of c, s
# or u.
#
# It is possible to use negative scores to forbid some forms to exist in the
# dictionary.

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
# works like above (the implicit score name has been used). You can use - in
# front of the affix to mark it as suffix, but that is optional.

## Circumfixes
# In the same way as suffixes, circumfixes can be defined:
CX {
a:.    b-e
a,b,c : x,y,z     beg.-.end
. : .     abeg-aend
}
# here before the : the is the stem ending/affix replacement definition for the
# prefix, after it the one for the suffix (they are both required), afterwards
# we have prefix-suffix. This can of course be combined with everything
# explained before like scoring and virtual stems.
#
# Note that there are no spaces allowed before and after the dash (-)
#
# Prefix, suffix and circumfix definitions can be mixed in one group.
#
```

## License ##

xmunch is relased under GNU GPLv3

