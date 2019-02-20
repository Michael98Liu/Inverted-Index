/**
 * Overload the comparison operators used for comparing Posting and
 * nPosting instantiations.
 * For positional posting, first compare term ID, then, document ID,
 * then fragment ID, then position in the fragment.
 * For non-positinal positng, first compare the term Id, then document ID.
 */

#ifndef COMPARISON_H
#define COMPARISON_H

#include "posting.hpp"

bool operator< (const Posting& p1, const Posting& p2);

bool operator> (const Posting& p1, const Posting& p2);

bool operator== (const Posting& p1, const Posting& p2);

bool operator< (const nPosting& p1, const nPosting& p2);

bool operator> (const nPosting& p1, const nPosting& p2);

bool operator== (const nPosting& p1, const nPosting& p2);

struct less_than_key{
    inline bool operator() (const Posting& p1, const Posting& p2);
};

#endif
