// Klondike solver
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <utility>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>

using namespace std;

#include "common.h"
#include "cout11.h"

#undef NDEBUG
#include <cassert>

#define NUM_LINES 8
#define NUM_WORKSPACES 4
#define NUM_SUITES 4


// #define MAX_CALL_DEPTH 50
// #define USE_MD5_HASH 1

#ifdef USE_MD5_HASH
#include "md5.h"
typedef string HASH_TYPE;
#else
typedef size_t HASH_TYPE;
#endif



vector<vector<Card> > line; //(NUM_LINES);
vector<int> depth; // (NUM_LINES, 0);
vector<Card> workspace;
vector<Card> goal; // (NUM_SUITES, -1);

int W = 0;



bool _cmp(const char* left, const char* right) {
    int c = strcmp((char *)left, (char *)right);
    // printf("_cmp(%s, %s) -> %d\n", left, right, c);
    return c < 0;
}

HASH_TYPE calc_hash() {
    // printf("buf = %p\n", (void *)buf);
    char buf[256], *p = buf;

    vector<char> tmp(4);
    rep(i, NUM_WORKSPACES) tmp[i] = card_serialize(workspace[i]);
    sort(all(tmp));
    rep(i, NUM_WORKSPACES) *p++ = tmp[i];
    *p++ = '/';

    rep(suite, NUM_SUITES) *p++ = card_serialize(goal[suite]);
    *p++ = '/';

    char buf2[160];
    vector<char*> _lines;
    rep(i, NUM_LINES) {
        int L = line[i].size();
        assert(L <= 19);
        char *buf_i = buf2 + 20*i, *q = buf_i;
        rep(j, L) {
            *q++ = card_serialize(line[i][j]);
        }
        *q = 0;
        _lines.push_back(buf_i);
    }

    sort(all(_lines), _cmp);

    rep(i, NUM_LINES) {
        strcpy(p, _lines[i]); // .c_str());
        p += strlen(_lines[i]); // .size();
        *p++ = '/';
    }

    *p = 0;
    assert(p - buf < 256);
    // printf("P{%s}\n", buf);

    // ss << '/';
    // cout << ss.str() << endl;
    // cout << "HASH:" << h << endl;
#ifdef USE_MD5_HASH
    return md5(buf); // ss.str());
#else
    return hash<string>()(buf); // ss.str());
#endif
}

void recalc_depth() {
    rep(i, NUM_LINES) {
        depth[i] = 0;
        rep(j, line[i].size()) {
            if (j == 0) {
                depth[i] = 1;
            } else if (pilable(line[i][j], line[i][j-1])) {
                ++depth[i];
            } else {
                depth[i] = 1;
            }
        }
        assert(0 <= depth[i] && depth[i] <= line[i].size());
    }
}


int _st_line = 0, _call_depth = 0, _max_call_depth = 1000;

void show_status() {
    // ++_st_line;
    printf("%d %d)", ++_st_line, _call_depth);

    // if ((_st_line % 10000) != 0) return;
    // if (_st_line > 10) return;
//    printf("%d ", _st_line);

    rep(i, NUM_WORKSPACES) printf(" %c", _single(workspace[i]));
    printf(" |");

    rep(suite, NUM_SUITES) printf(" %c", _single(goal[suite]));
    /// rep(suite, NUM_SUITES) assert(goal[suite] == VACANT || card_suite(goal[suite]) == suite);
    printf(" | ");

    rep(i, NUM_LINES) {
        int L = line[i].size();
        rep(j, L) {
            /// if (j == L - depth[i]) putchar('.');  // visualize depth
            putchar(_single(line[i][j]));

        }
        // printf(" (%d)", depth[i]);
        printf(" | ");
    }
    printf("\n");
}


inline int get_vacant_workspace() {
    rep(i, NUM_WORKSPACES) {
        if (workspace[i] == -1) return i;
    }
    return -1;
}


set<HASH_TYPE> visited;
bool solved = false;

int solve() {
    if (solved) return 0;
    if (_call_depth >= _max_call_depth) return -1;
    // if (_st_line == 10) return 0;
#ifdef DO_HASH
    HASH_TYPE hash = calc_hash();
    if (found(hash, visited)) {
        // printf("%zu has already been visited;\n", hash);
        return 0; // visited
    }
    visited.insert(hash);
#endif

    ++_call_depth;

    recalc_depth();

    show_status();

    assert(depth.size() == NUM_LINES);
    rep(i, NUM_LINES) {
        assert(line[i].size() < 19);
        // printf("[i=%d][0 . %d . %d]\n", i, depth[i], line[i].size());
        assert(0 <= depth[i] && depth[i] <= line[i].size());
    }

    {
        bool check_solved = true;
        rep(i, NUM_LINES) {
            if (depth[i] > 0) { check_solved = false; break; }
        }
        if (check_solved && W == 0) {
            printf("SOLVED\n");
            solved = true;
            --_call_depth;
            return 0;
        }
    }

    // hands
    rep(i, NUM_LINES) {
        int L = line[i].size();
        if (L == 0) continue;
        assert(depth[i] <= L);
        for (int d=depth[i]; d>0; --d) {
            int ix = L - d;
            assert(0 <= ix && ix < L);
            // printf("line[%d].size = %lu, d = %d, ix = %d\n", i, line[i].size(), d, ix);
            Card card = line[i][ix];
            int suite = card_suite(card);
            // int num = line[i][ix].first, suite = line[i][ix].second;
            // printf(" (i=%d/%lu, num=%d/%d),", i, depth.size(), num, 13);
            // if d==1
            if (d == 1) {
                // assert ix == line[i].size() - 1;
                // put it to goal if possible
                assert(is_valid_card(card));
                if (card_num(goal[suite]) == card_num(card)-1) {
//                    printf("  can be put on goal\n");
                    Card last_top = goal[suite];
                    // pile
                    goal[suite] = card;
                    assert(line[i].size() > 0);
                    line[i].pop_back();
                    assert(line[i].size() == L - 1);
                    // recalc_depth();

                    // go
                    solve();

                    // unpile
                    goal[suite] = last_top;
                    line[i].push_back(card);
                    assert(line[i].size() == L);
                    recalc_depth();
                }

                if (W < NUM_WORKSPACES) {
//                    printf("  can be piled onto workspace\n");
                    // pile onto workspace
                    int last_W = W;
                    int vacant_workspace_at = get_vacant_workspace();
                    assert(0 <= vacant_workspace_at && vacant_workspace_at < NUM_WORKSPACES);
                    assert(workspace[vacant_workspace_at] == VACANT);
                    workspace[vacant_workspace_at] = card;
                    W++;
                    assert(W <= NUM_WORKSPACES);
                    assert(line[i].size() == L);
                    line[i].pop_back();
                    assert(line[i].size() == L - 1);
                    // recalc_depth();

                    // go
                    solve();

                    // unpile from workspace
                    assert(W == last_W + 1);
                    workspace[vacant_workspace_at] = VACANT;
                    --W;
                    assert(W == last_W);
                    assert(line[i].size() == L - 1);
                    line[i].push_back(card);
                    assert(line[i].size() == L);
                    recalc_depth();
                }
            }

            int zeroes = 0;
            rep(j, NUM_LINES) {
                if (j == i) continue;
                int J = line[j].size();
                if ((d >= 2 && J == 0) || pilable(line[j][line[j].size()-1], card)) { // line[i][ix])) {
                    if (J == 0) zeroes++;
                    if (zeroes == 2) continue;

                    // pile
                    assert(line[j].size() == J);
                    assert(line[i].size() == L && L >= d);
                    assert(0 <= ix && ix < line[i].size());

                    line[j].insert(line[j].end(), line[i].begin()+ix, line[i].end());
                    line[i].erase(line[i].begin()+ix, line[i].end());
                    // line[i].resize(L - d);
                    /*
                    for (int k=0; k<d; ++k) {
                        Card _card = line[i].back();
                        line[i].pop_back();
                        line[j].push_back(_card);
                    }
                    */
                    assert(line[j].size() == J + d);
                    assert(line[i].size() == L - d);
                    // recalc_depth();

                    // go
                    solve();

                    // unpile
                    // int jx = line[j].size() - d;
                    assert(line[i].size() == L - d);
                    assert(line[j].size() == J + d);

                    line[i].insert(line[i].end(), line[j].begin()+J, line[j].end());
                    line[j].erase(line[j].begin()+J, line[j].end());
                    // line[j].resize(J);

                    assert(line[i].size() == L);
                    assert(line[j].size() == J);

                    recalc_depth();
                }
            }
            //
        }
    }

    // workspace
    rep(i, NUM_WORKSPACES) {
        Card card = workspace[i];
        if (card == VACANT) continue;
        // to lines
        rep(j, NUM_LINES) {
            int J = line[j].size();
            if (J == 0 || pilable(line[j][line[j].size()-1], line[i][line[i].size()-1])) {
                // pile
                assert(line[j].size() == J);

                assert(W >= 1);
                int last_W = W;
                workspace[i] = VACANT;
                --W;

                // recalc_depth();
                line[j].push_back(card);

                assert(W == last_W - 1);
                assert(line[j].size() == J + 1);

                // go
                solve();

                // unpile
                assert(line[j].size() == J + 1 && J >= 0);
                assert(W == last_W - 1);

                line[j].pop_back();
                workspace[i] = card;
                ++W;

                assert(line[j].size() == J);
                assert(W == last_W);

                recalc_depth();
            }
        }
        // to goal
        int suite = card_suite(card);
        if (card_num(goal[suite]) == card_num(card)-1) {
//            printf("  can be put on goal\n");
            Card last_top = goal[suite];
            // pile
            goal[suite] = card;
            // assert(workspace.size() == W && W > 0);
            int last_W = W;
            assert(W >= 1);
            workspace[i] = VACANT;
            --W;
            assert(W == last_W - 1);
            // recalc_depth();

            // go
            solve();

            // unpile
            goal[suite] = last_top;
            // line[i].push_back(card);
            assert(W == last_W - 1 && W < NUM_WORKSPACES);
            workspace[i] = card;
            ++W;
            assert(W == last_W);

            recalc_depth();
        }
    }

#ifdef DO_HASH
    HASH_TYPE hash_again = calc_hash();
    assert(hash_again == hash);
#endif

    --_call_depth;
    return 0;
}

void usage() {
    printf("usage: Klondike_solver <game_file> [max_call_depth]\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage();
        return 0;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        usage();
        return 0;
    }

    if (argc >= 3) {
        int mcd = atoi(argv[2]);
        if (mcd > 0) _max_call_depth = mcd;
    }

    line.assign(NUM_LINES, vector<Card>());
    depth.assign(NUM_LINES, 0);
    workspace.assign(NUM_WORKSPACES, VACANT);
    W = 0;
    goal.assign(NUM_SUITES, VACANT);

    rep(i, NUM_LINES) {
        line[i].clear();
    }

    char buf[32]; // 24 suffices
    while (1) {
        if (fgets(buf, 32, fp) == NULL) break;
        // cout << "H:" << hash<string>()(buf) << endl;
        int len = strlen(buf);
        rep(i, NUM_LINES) {
            int n = char_to_card_num(buf[3*i]), s = char_to_card_suite(buf[3*i+1]);
            if (n < 0 || s < 0) break;
            line[i].push_back(make_card(n, s));
        }
    }
    fclose(fp);

    visited.clear();
    solve();

    return 0;
}
