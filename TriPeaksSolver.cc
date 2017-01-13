#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cstdio>
using namespace std;

#include "common.h"
#include "cout11.h"

typedef long long ll;

vector<int> pyramid, deck;
map<int, int> m_okmask;

int FULL = (1 << 28)-1;

int ok_base[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    10, 11, 13, 14, 16, 17,
    19, 21, 23 };

// (((mask >> ok_base[i]) & 3) == 3)
int okmask(int mask) {
    if (m_okmask.find(mask) != m_okmask.end()) {
        return m_okmask[mask];
    }

    int ok = 0x3FF;

    if (((mask >> 0) & 3) == 3) ok |= (1 << 10);
    if (((mask >> 1) & 3) == 3) ok |= (1 << 11);
    if (((mask >> 2) & 3) == 3) ok |= (1 << 12);
    if (((mask >> 3) & 3) == 3) ok |= (1 << 13);
    if (((mask >> 4) & 3) == 3) ok |= (1 << 14);
    if (((mask >> 5) & 3) == 3) ok |= (1 << 15);
    if (((mask >> 6) & 3) == 3) ok |= (1 << 16);
    if (((mask >> 7) & 3) == 3) ok |= (1 << 17);
    if (((mask >> 8) & 3) == 3) ok |= (1 << 18);

    if (((mask >> 10) & 3) == 3) ok |= (1 << 19);
    if (((mask >> 11) & 3) == 3) ok |= (1 << 20);
    if (((mask >> 13) & 3) == 3) ok |= (1 << 21);
    if (((mask >> 14) & 3) == 3) ok |= (1 << 22);
    if (((mask >> 16) & 3) == 3) ok |= (1 << 23);
    if (((mask >> 17) & 3) == 3) ok |= (1 << 24);

    if (((mask >> 19) & 3) == 3) ok |= (1 << 25);
    if (((mask >> 21) & 3) == 3) ok |= (1 << 26);
    if (((mask >> 23) & 3) == 3) ok |= (1 << 27);

    m_okmask[mask] = ok;
    return ok;
}

int is_next[14][14];
bool solved = false;

map<ll, int> m_solve_visited;
map<ll, vector<ll> > before;

void init() {
    for (int i=0; i<13; ++i) {
        for (int j=0; j<13; ++j) is_next[i][j] = 0;
        is_next[i][(i+1)%13] = 1;
        is_next[i][(i+12)%13] = 1;
    }
}

inline ll _key(int head, int st, int mask) {
    return ((ll)head << 33LL) | ((ll)st << 28LL) | (ll)mask; // 4bit, 5bit + 28bit = 33bit
}

// #include <iostream>
void render_key(ll key, int& head, int& st, int& mask) {
    head = (int)((key >> 33LL) & 15LL);
    st = (int)((key >> 28LL) & 31LL);
    mask = (int)((key & 0xFFFFFFF));
}

char buf[3];
int T = 0;

string _loc(int loc) {
/* a:    0     1     2
 * b:   0 1   2 3   4 5
 * c:  0 1 2 3 4 5 6 7 8
 * d: 0 1 2 3 4 5 6 7 8 9
 */
  stringstream ss;
  if (loc < 10) {
    ss << 'd' << (1+loc);
  } else if (loc < 19) {
    ss << 'c' << (1+loc-10);
  } else if (loc < 25) {
    ss << 'b' << (1+loc-19);
  } else {
    ss << 'a' << (1+loc-25);
  }
  return ss.str();
}

int solve(int head=deck[0], int st=1, int mask=0) {
    ll key = _key(head, st, mask);
    if (solved) return -1;
    if (m_solve_visited.find(key) != m_solve_visited.end()) {
        // printf("// already visited (%d,%d)\n", st, mask);
        return m_solve_visited[key];
    }

    if (mask == FULL) {
        printf("%d <solved>\n", T);
        vector<ll> iter;
        iter.push_back(key);
        while (true) {
            if (before[key].size() == 0) break;
            key = before[key][0];
            iter.push_back(key);
        }
        reverse(all(iter));
        for (int i=0; i<iter.size()-1; ++i) {
            printf("%d) ", 1+i);
            ll key_b = iter[i], key_a = iter[i+1];
            int head_b, st_b, mask_b, head_a, st_a, mask_a;
            render_key(key_b, head_b, st_b, mask_b);
            render_key(key_a, head_a, st_a, mask_a);
//                printf("- (%c %d %07x) -> (%c %d %07x) : ",
//                    num_single[head_b], st_b, mask_b, num_single[head_a], st_a, mask_a);
            if (st_a == st_b) {
                int maskdiff = mask_a - mask_b;
                int loc = __builtin_ctz(maskdiff);
                cout << "from pyramid " << _loc(loc) << " (" << num_single[pyramid[loc]] << ")" << endl;
            } else {
                cout << "turn deck (" << num_single[deck[st_b]] << ")" << endl;
            }
        }
        solved = true;
        m_solve_visited[key] = 1;
        return 1;
    }
    else if (st == 24) {
        // printf("not solved. (%07x)\n", mask);
        m_solve_visited[key] = 0;
        return 0;
    }

    int ok = okmask(mask);

    /// printf("SOLVE(head=%d, st=%d, mask=%07x) : ok=%07x\n", 1+head, st, mask, ok);

    bool possible = false;
    for (int i=0; i<28; ++i) {
        int m = 1 << i;
        if (m & mask) continue; // もう引いたやつ
        if (i >= 10 && (ok & m) == 0) continue;
        // if (((mask >> ok_base[i]) & 3) < 3) continue;
        int card = pyramid[i];
        if (!is_next[head][card]) continue;
        // printf("  can draw %d at %d\n", 1+card, i);

        possible = true;
        // これを引く場合
        int next_mask = mask | (1 << i);
        before[_key(card, st, next_mask)].push_back(key);
        /// printf("drawing %c at %d...\n", num_single[card], i);
        /// putchar(num_single[card]);
        int r = solve(card, st, mask | (1 << i));
        if (r) return 1;
    }
    // 引かずにただめくるだけ
    if (!possible) {
      /// printf("no drawable card...");
    }
    /// printf("go next...\n");
    /// putchar('\r');
    if (++T % 100000 == 0) {
      printf("%d\r", T);
      /// putchar('.');
      fflush(stdout);
    }
    before[_key(deck[st], st+1, mask)].push_back(key);
    return solve(deck[st], st+1, mask);
}

void usage() {
    printf("usage: TriPeaks_solver <game-file>\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage();
        return 0;
    }

    char _pyramid[29], _deck[25];

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        usage();
        return 0;
    }

    vector<char> tmp;
    while (1) {
        int ch = fgetc(fp);
        if (ch == EOF) break;

        int c = char_to_card_num(ch);
        if (c < 0) continue;

        tmp.push_back(c);
    }
    if (tmp.size() != 52) {
        printf("invalid data.\n");
        return 0;
    }

    // 0+3 -> pyramid[25:]
    // 3+6 -> pyramid[19:]
    // 9+9 -> pyramid[10:]
    // 18+10 -> pyramid[0:]
    pyramid.insert(pyramid.end(), tmp.begin()+18, tmp.begin()+28);
    pyramid.insert(pyramid.end(), tmp.begin()+9, tmp.begin()+18);
    pyramid.insert(pyramid.end(), tmp.begin()+3, tmp.begin()+9);
    pyramid.insert(pyramid.end(), tmp.begin(), tmp.begin()+3);
    // 28+24 -> deck
    deck.insert(deck.end(), tmp.begin()+28, tmp.end());

    cout << "pyramid: ";
    tr(it, pyramid) putchar(num_single[*it]);
    cout << endl;

    cout << "deck: ";
    tr(it, deck) putchar(num_single[*it]);
    cout << endl;

    if (pyramid.size() != 28 || deck.size() != 24) {
        printf("invalid data length (%d, %d)\n", (int)pyramid.size(), (int)deck.size());
        fclose(fp);
        return 0;
    }

    init();

    vector<int> cnt(13, 0);
    for (int i=0; i<28; ++i) ++cnt[pyramid[i]];
    for (int i=0; i<24; ++i) ++cnt[deck[i]];
    for (int i=0; i<13; ++i) {
        if (cnt[i] != 4) {
            printf("invalid data.\n");
            cout << cnt << endl;
            return 0;
        }
    }

    solve();

    fclose(fp);
    return 0;
}
// 8Q8AAKK0260739346Q96A227 437058A574K26JQQ439J05J89JK5
