/*
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <utility>
#include <algorithm>
*/

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <queue>
using namespace std;

//#undef NDEBUG
//#include <cassert>


#include "common.h"
#include "cout11.h"

#define NUM_LINES 8
#define NUM_WORKSPACES 4
#define NUM_SUITES 4

#define LINE_BASE 0
#define WORKSPACE NUM_LINES
#define GOAL      NUM_LINES+1


char *rtrim(char *buf) {
    char *p = buf;
    while (*p >= ' ') ++p;
    *p = 0;
    return buf;
}

#if 0
void check_consistency(vector<vector<Card> >& _board, int line) {
    assert(_board.size() == 2 + NUM_LINES);

    rep(l, NUM_LINES) assert(_board[l].size() >= 0);

    assert(_board[WORKSPACE].size() == NUM_WORKSPACES);
    assert(_board[GOAL].size() == NUM_SUITES);

    // カードの枚数チェック
    set<Card> box;
    rep(w, NUM_WORKSPACES) {
        Card ws_card = _board[WORKSPACE][w];
        assert(ws_card == VACANT || is_valid_card(ws_card));
        if (ws_card != VACANT) box.insert(ws_card);
    }

    rep(s, NUM_SUITES) {
        Card goal_card = _board[GOAL][s];
        assert(goal_card == VACANT || is_valid_card(goal_card));
        if (goal_card != VACANT) {
            int num = card_num(goal_card), suite = card_suite(goal_card);
            for (int n=0; n<=num; ++n) {
                Card card = make_card(n, suite);
                box.insert(card);
            }
        }
    }
    rep(l, NUM_LINES) {
        int L = _board[l].size();
        rep(c, L) {
            Card card = _board[l][c];
            assert(is_valid_card(card));
            box.insert(card);
        }
    }
    // printf("box size = %lu, line = %d\n", box.size(), line);
    assert(box.size() == 52);
}
#endif

bool load__board(vector<vector<Card> >& _board, char *path) {
    _board.clear();

    FILE *fp = fopen(path, "r");
    if (fp == NULL) return false;

    rep(l, NUM_LINES) _board.push_back(vector<Card>());

    int num_cards = 0;
    while (1) {
        char buf[32];
        if (!fgets(buf, 32, fp)) break;
        rtrim(buf);
        int len = strlen(buf);
        bool suite_comes_next = false;
        int num = -1, suite = -1;
        for (int p=0, l=0; p < len; ++p) {
            char c = buf[p];
            if (c == ' ') {
                suite_comes_next = false;
                continue;
            }
            if (suite_comes_next) {
                suite = char_to_card_suite(c);
                Card card = make_card(num, suite);
                _board[l++].push_back(card);
                ++num_cards;
                if (l == NUM_LINES) break;
                suite_comes_next = false;
            } else {
                if (c == '1') continue;
                num = char_to_card_num(c);
                suite_comes_next = true;
            }
        }
    }
    fclose(fp);

    _board.push_back(vector<Card>(NUM_WORKSPACES, VACANT));
    _board.push_back(vector<Card>(NUM_SUITES, VACANT));

    // assert(num_cards == 52);
    // check_consistency(_board, 114);

    return true;
}

string board_serialize(vector<vector<Card> >& _board) {
    stringstream ss;

    // sort(all(_board[WORKSPACE]));
    rep(w, NUM_WORKSPACES) ss << card_serialize(_board[WORKSPACE][w]);
    rep(s, NUM_SUITES) ss << card_serialize(_board[GOAL][s]);

    vector<string> tmp; // (NUM_LINES);
    rep(l, NUM_LINES) {
        stringstream ssi;
        int L = _board[l].size();
        rep(c, L) ssi << card_serialize(_board[l][c]);
        tmp.push_back(ssi.str());
    }
    sort(all(tmp));
    rep(l, NUM_LINES) {
        if (l > 0) ss << '.';
        ss << tmp[l];
    }

    return ss.str();
}

void board_deserialize(vector<vector<Card> >& _board, string& serialized) {
    if (_board.size() != NUM_LINES + 2) _board.resize(NUM_LINES + 2);

    if (_board[WORKSPACE].size() != NUM_WORKSPACES)
        _board[WORKSPACE].resize(NUM_WORKSPACES);
    rep(w, NUM_WORKSPACES) _board[WORKSPACE][w] = card_deserialize(serialized[w]);

    if (_board[GOAL].size() != NUM_SUITES)
        _board[GOAL].resize(NUM_SUITES);
    rep(s, NUM_SUITES) _board[GOAL][s] = card_deserialize(serialized[NUM_WORKSPACES+s]);

    rep(l, NUM_LINES) _board[l].clear();

    for (int i=NUM_WORKSPACES+NUM_SUITES, l=0; i<serialized.size(); ++i) {
        if (serialized[i] == '.') { ++l; continue; }
        if (l == NUM_LINES) break;
        _board[l].push_back(card_deserialize(serialized[i]));
    }
}

void board_pp(vector<vector<Card> >& _board, bool full=false) {
    // printf("%d %d)", ++_st_line, _call_depth);
    if (full) {
        rep(w, NUM_WORKSPACES) cout << " " << _full(_board[WORKSPACE][w]);
        cout << " |";

        rep(s, NUM_SUITES) cout << " " << _full(_board[GOAL][s]);
        cout << " |";

        rep(l, NUM_LINES) {
            int L = _board[l].size();
            rep(j, L) {
                cout << _full(_board[l][j]);
            }
            cout << " | ";
        }
        cout << endl;
    } else {
        rep(w, NUM_WORKSPACES) { putchar(' '); putchar(_single(_board[WORKSPACE][w])); }
        putchar(' '); putchar('|');

        rep(s, NUM_SUITES) { putchar(' '); putchar(_single(_board[GOAL][s])); }
        putchar(' '); putchar('|');

        rep(l, NUM_LINES) {
            putchar(' ');
            int L = _board[l].size();
            rep(j, L) {
                putchar(_single(_board[l][j]));
            }
            putchar(' '); putchar('|');
        }
        putchar('\n');
    }
}

priority_queue<pair<pair<int, int>, string> > _pq;
set<string> _visited;
map<string, pair<int, string> > _last_step;
vector<vector<Card> > _board;

void queue_nextstep(string& curr, int curr_distance, int next_score) {
    string next = board_serialize(_board);
    // cout << "queuing " << curr << " -> " << next << endl;
    if (!found(next, _visited)) {
        int next_distance = curr_distance + 1;
        _pq.push(make_pair(make_pair(next_score, -next_distance), next));
        if (found(next, _last_step)) {
            int min_distance = _last_step[next].first;
            if (next_distance < min_distance) {
                _last_step[next] = make_pair(next_distance, curr);
            }
        } else {
            _last_step[next] = make_pair(next_distance, curr);
        }
    }
}

void route_back(vector<vector<Card> >& initial_board, string& here) {
    vector<string> route;
    while (true) {
        route.push_back(here);
        if (!found(here, _last_step)) break;
        here = _last_step[here].second;
    }
    reverse(all(route));

    vector<vector<Card> > b0 = initial_board, b1;
    // board_deserialize(b0, route[0]);

    for (int i=0; i<route.size(); ++i) {
//        printf("%d)\n", i);

        int from = -1, from_extra = -1,
            to = -1, to_extra = -1;
        vector<Card> moved;

        board_deserialize(b1, route[i]);
        // WS
        vector<int> ws_mapping(NUM_WORKSPACES, -1);
        set<int> ws_used;
        rep(w1, NUM_WORKSPACES) {
            Card c1 = b1[WORKSPACE][w1];
            rep(w0, NUM_WORKSPACES) {
                if (found(w0, ws_used)) continue;
                if (b0[WORKSPACE][w0] == c1) {
                    ws_mapping[w1] = w0;
                    ws_used.insert(w0);
                    break;
                }
            }
        }
        rep(w, NUM_WORKSPACES) {
            if (ws_mapping[w] == -1) {
                rep(i, NUM_WORKSPACES) {
                    if (!found(i, ws_used)) {
                        ws_mapping[w] = i;
                        ws_used.insert(i);
                        break;
                    }
                }
            }
        }
        vector<Card> wtmp(NUM_WORKSPACES);
        rep(w1, NUM_WORKSPACES) {
            int w0 = ws_mapping[w1];
            wtmp[w0] = b1[WORKSPACE][w1];
        }
        rep(w1, NUM_WORKSPACES) {
            b1[WORKSPACE][w1] = wtmp[w1];
        }

        // from/to
        rep(w, NUM_WORKSPACES) {
            if (b0[WORKSPACE][w] != b1[WORKSPACE][w]) {
                if (b1[WORKSPACE][w] == VACANT) {
                    from = WORKSPACE; from_extra = w;
                    // moved.push_back(b0[WORKSPACE][w]);
                }
                if (b0[WORKSPACE][w] == VACANT) {
                    to = WORKSPACE; to_extra = w;
                    moved.push_back(b1[WORKSPACE][w]);
                }
            }
        }

        // GOAL
        rep(s, NUM_SUITES) {
            if (b0[GOAL][s] != b1[GOAL][s]) {
                // printf("  GOAL #%d: %c -> %c\n", s, _single(b0[GOAL][s]), _single(b1[GOAL][s]));
                to = GOAL; to_extra = s;
                moved.push_back(b1[GOAL][s]);
                break;
            }
        }

        // LINES
        vector<int> line_mapping(NUM_LINES, -1);
        // vector<bool> line_used(NUM_LINES, false);
        set<int> line_used;
        rep(l1, NUM_LINES) {
            int L1 = b1[l1].size();
            // if (L1 == 0) continue;
            rep(l0, NUM_LINES) {
                if (found(l0, line_used)) continue;
                int L0 = b0[l0].size();
                if ((L0 == 0 && L1 == 0) ||
                    (L0 > 0 && L1 > 0 && b0[l0][0] == b1[l1][0])) {
                    line_mapping[l1] = l0;
                    line_used.insert(l0);
                    break;
                }
            }
        }
        // cout << line_used << endl;
        // assert(line_used.size() >= 7);
        // find unmatched...
        rep(l, NUM_LINES) {
            if (line_mapping[l] == -1) {
                rep(i, NUM_LINES) {
                    if (!found(i, line_used)) {
                        line_mapping[l] = i;
                        line_used.insert(i);
                        break;
                    }
                }
            }
        }
        // cout << "line_mapping: " << line_mapping << endl;

        vector<vector<Card> > ltmp(NUM_LINES);
        rep(l1, NUM_LINES) {
            int l0 = line_mapping[l1];
            ltmp[l0] = b1[l1];
        }
        rep(l1, NUM_LINES) {
            b1[l1] = ltmp[l1];
        }

        rep(l, NUM_LINES) {
            if (b0[l] != b1[l]) {
                if (b0[l].size() > b1[l].size()) {
                    from = l;
                    from_extra = b0[l].size() - b1[l].size();
                } else {
                    to = l;
                    to_extra = b1[l].size() - b0[l].size();
                    moved.insert(moved.end(), b1[l].begin()+b0[l].size(), b1[l].end());
                }
            }
        }
/*
            if (find(b1.begin(), b1.begin()+NUM_LINES, b0[l]) == b1.begin()+NUM_LINES) {
                printf("  LINE: -"); cout << b0[l] << endl;
            }
            if (find(b0.begin(), b0.begin()+NUM_LINES, b1[l]) == b0.begin()+NUM_LINES) {
                printf("  LINE: +"); cout << b1[l] << endl;
            }
        }
*/

        if (i > 0) {
            printf("(%d) ", i);
            rep(i, moved.size()) {
                cout << _full(moved[i]);
            }
            switch (from) {
                case WORKSPACE:
                    printf(" at workspace"); // #%d", 1+from_extra);
                    break;
//                case GOAL:
//                    printf(" in GOAL"); // #%d", 1+from_extra);
//                    break;
                default:
                    printf(" at line %d", 1+from);
                    // if (from_extra > 1) printf("<%d>", from_extra);
                    break;
            }
            switch (to) {
                case WORKSPACE:
                    printf(" -> workspace"); // #%d", 1+to_extra);
                    break;
                case GOAL:
                    printf(" -> GOAL"); // #%d", 1+to_extra);
                    break;
                default:
                    printf(" -> line %d", 1+to);
                    // if (to_extra > 1) printf("<%d>", to_extra);
                    break;
            }
            cout << endl;

//            cout << "  "; board_pp(b0);
        }
        // cout << "  "; 
        board_pp(b1);

        b0 = b1;
    }
}

int max_score = 0;

string solve(string& intial_board_serialized) {
    while (!_pq.empty()) _pq.pop(); // _pq.clear();
    _visited.clear();
    _last_step.clear();
    // queue<string> q;
    // q.push(intial_board_serialized);
    _pq.push(make_pair(make_pair(0, 0), intial_board_serialized));

    int step = 0;
//    while (!q.empty()) {
    while (!_pq.empty()) {
        int _score = _pq.top().first.first, distance = -_pq.top().first.second;
        string here = _pq.top().second;
        _pq.pop();

        if (max_score < _score) {
            for (int i=max_score; i<_score; ++i) {
                putchar('#');
                fflush(stdout);
            }
            max_score = _score;
        }

        ++step;

        if (found(here, _visited)) continue;
        // string board_serialized = q.front(); q.pop();
        _visited.insert(here);

        board_deserialize(_board, here);
        // cout << _board << endl;
/*
        if (step % 2000 == 0) {
            printf("step=%d distance=%d) ", step, distance);
            board_pp(_board);
        }
*/
        // check_consistency(_board, 193);

        int max_movable = 1;
        rep(w, NUM_WORKSPACES) if (_board[WORKSPACE][w] == VACANT) ++max_movable;
        rep(l, NUM_LINES) if (_board[l].size() == 0) ++max_movable;

        int score = 0;
        rep(s, NUM_SUITES) score += card_num(_board[GOAL][s]);
        if (score == 48) {
            printf("\n"); // SOLVED.
            return here;
        }

        // line#l -> Goal, WS, other line
        rep(l, NUM_LINES) {
            int L = _board[l].size(), depth = 0;
            if (L == 0) continue;

            // Line -> Goal
            Card card = _board[l][L-1];
            int num = card_num(card), suite = card_suite(card);

            Card goal_card = _board[GOAL][suite]; // can be -1
            if (num == card_num(goal_card) + 1) {
                // pilable to Goal
                _board[GOAL][suite] = card;
                _board[l].pop_back();
                // queue
                queue_nextstep(here, distance, score+1);
                // restore
                _board[GOAL][suite] = goal_card;
                _board[l].push_back(card);
            }

            // LINE -> WS
            int vacant_ws = -1;
            rep(w, NUM_WORKSPACES) {
                if (_board[WORKSPACE][w] == VACANT) { vacant_ws = w; break; }
            }
            if (vacant_ws >= 0) {
                // pilable to WS
                _board[WORKSPACE][vacant_ws] = card;
                sort(all(_board[WORKSPACE]));
                _board[l].pop_back();
                // queue
                queue_nextstep(here, distance, score);
                // restore
                rep(w, NUM_WORKSPACES) if (_board[WORKSPACE][w] == card) _board[WORKSPACE][w] = VACANT;
                sort(all(_board[WORKSPACE]));
                _board[l].push_back(card);
            }

            for (int d=1; d<=L; ++d) {
                if (d > max_movable) break;

                card = _board[l][L-d];
                int empty_line_count = 0;
                rep(oth, NUM_LINES) {
                    if (oth == l) continue;
                    int OL = _board[oth].size();
                    if (d < L && OL == 0) {
                        if (empty_line_count > 0) break;
                        // move d cards from l to oth<empty>
                        if (d <= max_movable-1) {
                            // move
                            _board[oth].assign(_board[l].begin()+L-d, _board[l].end());
                            _board[l].resize(L-d);
                            // queue
                            queue_nextstep(here, distance, score);
                            // restore
                            _board[l].insert(_board[l].end(), _board[oth].begin(), _board[oth].end());
                            _board[oth].clear();
                        }
                        ++empty_line_count;
                    } else {
                        Card oth_card = _board[oth].back();
                        if (pilable(oth_card, card)) {
                            // move d cards from l to oth
                            // move
                            _board[oth].insert(_board[oth].end(), _board[l].begin()+L-d, _board[l].end());
                            _board[l].resize(L-d);
                            // queue
                            queue_nextstep(here, distance, score);
                            // restore
                            _board[l].insert(_board[l].end(), _board[oth].begin()+OL, _board[oth].end());
                            _board[oth].resize(OL);
                        }
                    }
                }
                // depth check
                if (d < L) {
                    Card next_card = _board[l][L-d-1];
                    if (!pilable(next_card, card)) break;
                }
            }
        }

        // workspace#w -> Goal, line
        vector<Card> _workspace(all(_board[WORKSPACE]));
        rep(w, NUM_WORKSPACES) {
            Card card = _board[WORKSPACE][w];
            if (card == VACANT) continue;

            int num = card_num(card), suite = card_suite(card);

            Card goal_card = _board[GOAL][suite]; // can be -1
            if (num == card_num(goal_card) + 1) {
                // pilable to Goal
                _board[GOAL][suite] = card;
                _board[WORKSPACE][w] = VACANT;
                sort(all(_board[WORKSPACE]));
                // queue
                queue_nextstep(here, distance, score+1);
                // restore
                _board[GOAL][suite] = goal_card;
                rep(i, NUM_WORKSPACES)
                _board[WORKSPACE].assign(all(_workspace));
            }

            // W -> Line
            int empty_line_count = 0;
            rep(oth, NUM_LINES) {
                int OL = _board[oth].size();
                if (OL == 0 || pilable(_board[oth].back(), card)) {
                    if (OL == 0) {
                        if (empty_line_count > 0) continue;
                        ++empty_line_count;
                    }
                    // move from WS to line#oth <empty>
                    _board[oth].push_back(card);
                    _board[WORKSPACE][w] = VACANT;
                    // queue
                    queue_nextstep(here, distance, score);
                    // restore
                    _board[oth].pop_back();
                    _board[WORKSPACE].assign(all(_workspace));
                }
            }

        }
    }
    return "";
}

void usage() {
    printf("usage: Klondike_solver_2 <game-file>\n");
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage();
        return 0;
    }
    char *path = argv[1];

    vector<vector<Card> > initial_board;
    if (!load__board(initial_board, path)) {
        printf("invalid data file.\n");
        return 0;
    }

    // cout << _board << endl;

    string s = board_serialize(initial_board);
    // assert(s.size() == 67);
    // cout << s << " " << endl;
    string goal = solve(s);
    route_back(initial_board, goal);

    return 0;
}
