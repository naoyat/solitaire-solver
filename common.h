#ifndef __COMMON_H
#define __COMMON_H

#include <vector>
#include <utility>
#include <sstream>
// #include <algorithm>
using namespace std;

#define rep(var,n)  for(int var=0;var<(n);var++)
#define all(c)  (c).begin(),(c).end()
#define tr(i, c)  for(auto i=(c).begin(); i!=(c).end(); i++)
#define found(e, s)  ((s).find(e) != (s).end())

extern const char *num_full[13];
extern const char *num_single;
extern const char *suite_full[4];
extern const char *suite_single;

#define HEART   0
#define CLUB    1
#define DIAMOND 2
#define SPADE   3

#define VACANT  -1
#define VACANT_CH '~'
#define VACANT_STR "~"

inline bool is_black(int suite) { return suite % 2; }

int char_to_card_num(char ch);
int char_to_card_suite(char ch);
int trans_str(vector<int>& v, const char *s);


typedef int Card;

inline int make_card(int num, int suite) { // 0-12, 0-3
    if (num < 0 || suite < 0) return VACANT;
    return (num << 2) | (suite);
}

inline int card_num(Card card) { return (card >= 0) ? (card >> 2) : -1; }
inline int card_suite(Card card) { return (card >= 0) ? (card & 3) : -1; }

inline bool pilable(Card card_below, Card card_above) {
    return (is_black(card_suite(card_below)) != is_black(card_suite(card_above))
            && card_num(card_below) == card_num(card_above) + 1);
}

inline bool is_valid_card(Card card) {
    int num = card_num(card), suite = card_suite(card);
    return ((0 <= num && num < 13) && (0 <= suite && suite < 4));
}
inline char _single(Card card) {
    if (card == VACANT) return VACANT_CH;
    // int num = card_num(card);
    // assert(0 <= num && num < 13);
    return num_single[card_num(card)];
}
inline string _full(Card card) {
    if (card == VACANT) return VACANT_STR;

    stringstream ss;
    int num = card_num(card), suite = card_suite(card);
    ss << num_full[num] << suite_full[suite];
    return ss.str();
}

inline char card_serialize(Card card) {
    if (!is_valid_card(card)) return VACANT_CH;
    int num = card_num(card), suite = card_suite(card);
    char c;
    if (suite < 2) {
        c = 'A' + suite*13 + num;
    } else {
        c = 'a' + (suite - 2)*13 + num;
    }
    // assert(0x20 <= c && c <= 0x7e);
    return c;
}

inline Card card_deserialize(char c) {
    if (c == VACANT_CH) return VACANT;

    int num = -1, suite = -1;
    if ('A' <= c && c <= 'M') {
        num = (c - 'A'); suite = 0;
    }
    if ('N' <= c && c <= 'Z') {
        num = (c - 'N'); suite = 1;
    }
    if ('a' <= c && c <= 'm') {
        num = (c - 'a'); suite = 2;
    }
    if ('n' <= c && c <= 'z') {
        num = (c - 'n'); suite = 3;
    }

    return make_card(num, suite);
}


#endif
