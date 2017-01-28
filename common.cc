#include <vector>
#include <iostream>
using namespace std;

#include "common.h"
#include "cout11.h"

const char *num_full[13] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
const char *num_single = "A234567890JQK";
const char *suite_full[4] = {"♡", "♣", "♢", "♠"};
const char *suite_single = "hcds";

int char_to_card_num(char ch) {
    // returns 0-12 or -1
    int n;
    switch (ch) {
    case 'A':
        n = 1; break;
    case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        n = (ch - '0'); break;
    case '0':
        n = 10; break;
    case 'J':
        n = 11; break;
    case 'Q':
        n = 12; break;
    case 'K':
        n = 13; break;
    default:
        n = 0; break;
    }
    return n - 1;
}

int char_to_card_suite(char ch) {
    // returns 0-3 or -1
    switch (ch) {
        case 's': return SPADE;
        case 'c': return CLUB;
        case 'd': return DIAMOND;
        case 'h': return HEART;
        default: return -1;
    }
}

int trans_str(std::vector<int>& v, const char *s) {
    v.clear();
    for (const char *p=s; *p; ++p) {
        int n = char_to_card_num(*p);
        if (n > 0) v.push_back(n-1);
    }
    std::cout << s << " " << v << std::endl;
    return v.size();
}

int read_card_num(FILE *fp) {
    while (true) {
        int ch = fgetc(fp);
        if (ch == EOF) return EOF;
        if (ch == '\n') return -1;
        if (ch <= ' ') continue;
        int num = char_to_card_num(ch);
        if (num == -1) {
            ungetc(ch, fp);
            return -1;
        } else {
            return num;
        }
    }
}

int read_card_suite(FILE *fp) {
    while (true) {
        int ch = fgetc(fp);
        if (ch == EOF) return EOF;
        if (ch == '\n') return -1;
        if (ch <= ' ') continue;
        int suite = char_to_card_suite(ch);
        if (suite == -1) {
            ungetc(ch, fp);
            return -1;
        } else {
            return suite;
        }
    }
}

Card read_card(FILE *fp) {
    int num = -1;
    while (true) {
        int ch = fgetc(fp);
        if (ch == EOF || ch == '\n') return -1;
        if (ch <= ' ' || ch == '1') continue;
        num = char_to_card_num(ch);
        if (num == -1) return -1;
        break;
    }

    int suite = -1;
    while (true) {
        int ch = fgetc(fp);
        if (ch == EOF || ch == '\n') return -1;
        if (ch <= ' ') continue;
        suite = char_to_card_suite(ch);
        if (suite == -1) return -1;
        break;
    }

    return make_card(num, suite);
}

std::vector<int> read_card_nums(FILE *fp)  {
    std::vector<int> nums;
    while (true) {
        int num = read_card_num(fp);
        if (num == -1) break;
        nums.push_back(num);
    }
    return nums;
}

std::vector<Card> read_cards(FILE *fp) {
    std::vector<Card> cards;
    while (true) {
        Card card = read_card(fp);
        if (card == -1) break;
        cards.push_back(card);
    }
    return cards;
}
