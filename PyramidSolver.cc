#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <queue>
//#include <algorithm>
using namespace std;

#include "common.h"
#include "cout11.h"

#undef NDEBUG
#include <cassert>

void _p(int card_num) {
    putchar(num_single[card_num]);
}

vector<Card> read_card_nums(FILE *fp) {
    assert(fp != NULL);

    char buf[80];
    if (fgets(buf, 80, fp) == NULL) return vector<Card>();

    vector<Card> cards;
    for (char *p=buf; *p>=' '; ++p) {
        if (*p == '1') continue;  // enable "10"
        int num = char_to_card_num(*p);
        if (num >= 0) cards.push_back(num);
    }
    return cards;
}

vector<Card> pyramid, deck;

void write_single(vector<Card>::iterator from, vector<Card>::iterator to) {
    for (auto it = from; it != to; ++it) {
        _p(*it);
    }
    putchar('\n');
}

// bool four(int x) { return x == 4; }

int okmask(int still) {
    int mask = 0xfe00000;
    for (int i=0,r=1; r<=6; i+=r, ++r) {
        for (int j=0; j<r; ++j) {
            int m = (3 << (i+j+r));
            if (!(still & m)) {
                mask |= (1 << (i+j));
            }
        }
    }
    return mask;
}

#define PARAM_P_REM  0
#define PARAM_D_REM  1
#define PARAM_ROUNDN 2
#define PARAM_C0     3
#define PARAM_C1     4

#define OPERATION_SOLVED      1
#define OPERATION_C0_KING     2
#define OPERATION_C0_C1       4
#define OPERATION_C0_P        8
#define OPERATION_C1_KING    16
#define OPERATION_C1_P       32
#define OPERATION_P_KING     64
#define OPERATION_P_P       128
#define OPERATION_TURN      256
#define OPERATION_BIG_TURN  512

const char *operations[] = {
    "SOLVED", "C0<KING>", "C0+C1", "C0+P", "C1<KING>", "C1+P", "P<KING>", "P1+P2", "TURN", "BIGTURN"
};

#define NUM_PYRAMID_CARDS 28
#define NUM_DECK_CARDS    24
#define NUM_ALLOWED_ROUNDS 3

#define UNAVAILABLE -1

#include <sstream>

string cname(int cid, bool detail=false) {
    assert(0 <= cid && cid < NUM_DECK_CARDS);
    stringstream ss;
    if (detail) {
        ss << cid << ":";
    }
    ss << "<" << num_single[deck[cid]] << ">";
    return ss.str();
}
string pyname(int pid) {
    stringstream ss;
    assert(0 <= pid && pid < NUM_PYRAMID_CARDS);
    // ss << "[" << pid << "]";
    for (int i=0,r=1; r<=7; i+=r,r++) {
        if (i <= pid && pid < i+r) {
            ss << "*abcdefg"[r] << (1+pid-i);
            ss << "<" << num_single[pyramid[pid]] << ">";
        }
    }
    return ss.str();
//    return "?";
}

string ops_pp(vector<int>& ops) {
    // cout << "OPS_PP" << ops << endl;
    assert(ops.size() == 3);
    int op = ops[0], arg1 = ops[1], arg2 = ops[2];
    if (op == 0) {
        return "NOP";
    }
    assert(__builtin_popcount(op) == 1);

    int op_id = (int)log2(op);
    assert(0 <= op_id && op_id < 10);

    stringstream ss;
    ss << operations[op_id];
    switch (op) {
        case OPERATION_TURN:
        case OPERATION_BIG_TURN:
        case OPERATION_SOLVED:
            break;

        case OPERATION_C0_KING:
            ss << "(" << cname(arg1, true) << ")"; break;
        case OPERATION_C1_KING:
            ss << "(" << cname(arg1, true) << ")"; break;
        case OPERATION_P_KING:
            ss << "(" << pyname(arg1) << ")"; break;

        case OPERATION_C0_C1:
            ss << "(" << cname(arg1, true) << "," << cname(arg2, true) << ")"; break;
        case OPERATION_C0_P: case OPERATION_C1_P:
            ss << "(" << cname(arg1, true) << "," << pyname(arg2) << ")"; break;
        case OPERATION_P_P:
            ss << "(" << pyname(arg1) << "," << pyname(arg2) << ")"; break;
    }
    return ss.str();
}

string ops_human(vector<int>& ops) {
    // cout << "OPS_PP" << ops << endl;
    assert(ops.size() == 3);
    int op = ops[0], arg1 = ops[1], arg2 = ops[2];
    if (op == 0) {
        return "";
    }
    assert(__builtin_popcount(op) == 1);

    int op_id = (int)log2(op);
    assert(0 <= op_id && op_id < 10);

    stringstream ss;
    switch (op) {
        case OPERATION_TURN:
        case OPERATION_BIG_TURN:
        case OPERATION_SOLVED:
            ss << operations[op_id];
            break;

        case OPERATION_C0_KING:
            ss << "deck(right)" << cname(arg1);
            break;
        case OPERATION_C1_KING:
            ss << "deck(left)" << cname(arg1);
            break;
        case OPERATION_P_KING:
            ss << "pyramid " << pyname(arg1);
            break;
        case OPERATION_C0_C1:
            ss << "deck(left)" << cname(arg2);
            ss << " + deck(right)" << cname(arg1);
            break;
        case OPERATION_C0_P:
            ss << "deck(right)" << cname(arg1);
            ss << " + pyramid " << pyname(arg2);
            break;
        case OPERATION_C1_P:
            ss << "deck(left)" << cname(arg1);
            ss << " + pyramid " << pyname(arg2);
            break;
        case OPERATION_P_P:
            ss << "pyramid " << pyname(arg1);
            ss << " + pyramid " << pyname(arg2);
            break;
    }
    return ss.str();
}

string params_pp(vector<int>& params) {
    // cout << "ASSERT_IF_VALID_PARAMS:" << params << endl;
    int p_rem = params[PARAM_P_REM]; // ピラミッド側の残り
    int d_rem = params[PARAM_D_REM]; // 手持ち札の残り
    int roundn = params[PARAM_ROUNDN]; // 回(0〜)
    int c0 = params[PARAM_C0], c1 = params[PARAM_C1]; // c0, c1の位置
    // printf("  p_rem = %d\n", p_rem);
    stringstream ss;
    ss << "[" << hex << p_rem
       << " " << hex << d_rem
       << " " << dec << "R" << (1+roundn)
       << "(" << c0 << "," << c1 << ")]";
    return ss.str();
}

void show_status(int step, vector<int> curr_params) {
    int p_rem = curr_params[PARAM_P_REM],
        d_rem = curr_params[PARAM_D_REM],
        roundn = curr_params[PARAM_ROUNDN],
        c0 = curr_params[PARAM_C0],
        c1 = curr_params[PARAM_C1];
    cout << step << ") " << params_pp(curr_params);

    putchar(' ');
    int m = 1, mm = 2, x = p_rem, mask = okmask(p_rem), available = p_rem & mask;
    for (int st=1,mm=2,base=0; st<=7; base+=st,mm<<=st,st++) {
        if (st > 1) cout << '/';

        stringstream ss;
        bool visible = false;
        for (int j=0; j<st; ++j) {
            int i = base + j;
            if (p_rem & m) {
                if (available & m) {
                    // _p(pyramid[i]);
                    ss << (char)num_single[pyramid[i]];
                    visible = true;
                }
                else ss << '#';
            } else {
                ss << '.';
                visible = true;
            }
            x &= ~m;
            m <<= 1;
        }
        if (visible) cout << ss.str();
        if (x == 0) break;
    }

    putchar(' ');
    if (c1 >= 0) _p(deck[c1]); else putchar('-');
    if (c0 >= 0) _p(deck[c0]); else putchar('-');
}

priority_queue<pair<int, pair<int, vector<int> > > > pq;
set<vector<int> > _visited;
map<vector<int>, pair<int, pair<vector<int>, vector<int> > > > last_step;

inline void assert_if_valid_params(vector<int>& params) {
    int p_rem = params[PARAM_P_REM]; // ピラミッド側の残り
    assert(0 <= p_rem && p_rem < (1 << NUM_PYRAMID_CARDS));

    int d_rem = params[PARAM_D_REM]; // 手持ち札の残り
    assert(0 <= d_rem && d_rem < (1 << NUM_DECK_CARDS));

    int roundn = params[PARAM_ROUNDN]; // 回(0〜)
    assert(0 <= roundn && roundn < NUM_ALLOWED_ROUNDS);

    int c0 = params[PARAM_C0], c1 = params[PARAM_C1]; // c0, c1の位置
    assert((c0 == UNAVAILABLE && c1 >= 0) || (c0 == UNAVAILABLE && c1 == UNAVAILABLE)
        || (c0 >= 0 && c1 > c0) || (c0 >= 0 && c1 == UNAVAILABLE));
}

void add_to_queue(int curr_step, vector<int>& curr_params, vector<int>& next_params, vector<int>& ops) {
    if (found(next_params, _visited)) return;

    assert_if_valid_params(next_params);

    int next_step = curr_step + 1;
    int p_rem = next_params[PARAM_P_REM],
        d_rem = next_params[PARAM_D_REM],
        roundn = next_params[PARAM_ROUNDN],
        c0 = next_params[PARAM_C0],
        c1 = next_params[PARAM_C1];

    int p_taken = NUM_PYRAMID_CARDS - __builtin_popcount(p_rem);
    int d_taken = NUM_DECK_CARDS - __builtin_popcount(d_rem);

    int score = -next_step + p_taken*4 - (c0+c1) - (roundn*NUM_DECK_CARDS);
//    score = -step + p_taken*3 - c1 - (roundn*NUM_DECK_CARDS);
//    score = -step + p_taken*3 - d_taken;
    score = -next_step + p_taken*4 - d_taken;

    if (found(next_params, last_step)) {
        if (curr_step > last_step[next_params].first) {
            last_step[next_params] = make_pair(curr_step, make_pair(curr_params, ops));
        }
    } else {
        last_step[next_params] = make_pair(curr_step, make_pair(curr_params, ops));
    }

    pq.push(make_pair(score, make_pair(-next_step, next_params)));
    _visited.insert(next_params);
}

inline int search_prev_c0(int next_d_rem, int c0) {
    while (c0 >= 0) {
        if (next_d_rem & (1 << c0)) return c0;
        --c0;
    }
    return c0;
}

inline int search_next_c1(int next_d_rem, int c1) {
    while (c1 < NUM_DECK_CARDS) {
        if (next_d_rem & (1 << c1)) return c1;
        ++c1;
    }
    if (c1 == NUM_DECK_CARDS) c1 = UNAVAILABLE;
    return c1;
}


void solve(bool verbose=false) {
    assert(pyramid.size() == NUM_PYRAMID_CARDS && deck.size() == NUM_DECK_CARDS);

    // pyramidの残りカード (28bit)
    // deckの残りカード (24bit)
    // いまどこを開いてるか (0-47) の値2つ =6bit x 2 = 12bit

    int _initial[] = { (1 << NUM_PYRAMID_CARDS)-1, (1 << NUM_DECK_CARDS)-1, 0, UNAVAILABLE, 0 };
    vector<int> initial_params( _initial, _initial+5 );
    assert_if_valid_params(initial_params);
    pq.push(make_pair(0, (make_pair(0, initial_params))));
    _visited.insert(initial_params);

    int max_p_taken = 0;

    while (!pq.empty()) {
        int score = pq.top().first;
        int curr_step = -pq.top().second.first;
        vector<int> curr_params = pq.top().second.second;
        pq.pop();
        assert_if_valid_params(curr_params);
        if (verbose) {
            cout << curr_params << params_pp(curr_params) << endl;
        }
        int p_rem = curr_params[PARAM_P_REM], // ピラミッド側の残り
            d_rem = curr_params[PARAM_D_REM], // 手持ち札の残り
            roundn = curr_params[PARAM_ROUNDN], // 回(0〜)
            c0 = curr_params[PARAM_C0], // c0の位置
            c1 = curr_params[PARAM_C1]; // c1の位置

        int p_taken = NUM_PYRAMID_CARDS - __builtin_popcount(p_rem);
        if (p_taken > max_p_taken) {
            rep(i, p_taken - max_p_taken) {
                putchar('#');
                fflush(stdout);
            }
            max_p_taken = p_taken;
        }

        if (p_rem == 0) {
            // possible_operations = OPERATION_SOLVED;
            printf("\nSOLVED.\n");

            vector<pair<vector<int>, vector<int> > > history;
            history.push_back(make_pair(curr_params, vector<int>(3, 0)));
            while (1) {
                if (!found(curr_params, last_step)) break;
                vector<int> prev_params = last_step[curr_params].second.first;
                vector<int> ops = last_step[curr_params].second.second;
                history.push_back(make_pair(prev_params, ops));
                curr_params = prev_params;
            }
            reverse(all(history));
            rep(i, history.size()) {
                show_status(1+i, history[i].first);
                vector<int> ops = history[i].second;
                cout << "\t; " << ops_human(ops) << endl;
            }

            return;
        }
        // else ...
        vector<int> available_pyramid_cards;
        set<int> available_pyramid_card_numbers;
        int mask = p_rem & okmask(p_rem);
        for (int i=0,m=1; i<NUM_PYRAMID_CARDS; ++i,m<<=1) {
            if (mask & m) available_pyramid_cards.push_back(i);
        }
        int A = available_pyramid_cards.size();
        assert(A > 0);
        rep(i, A) {
            int pi = available_pyramid_cards[i];
            int p_card = 1 + pyramid[pi];
            assert(1 <= p_card && p_card <= 13);
            available_pyramid_card_numbers.insert(p_card);
        }
        assert(available_pyramid_card_numbers.size() <= 13);

        int possible_operations = 0;

        int num_p_rem = __builtin_popcount(p_rem);
        assert(num_p_rem >= 1);

        if (found(13, available_pyramid_card_numbers)) {
            possible_operations |= OPERATION_P_KING;
        }

        if (num_p_rem >= 2) {
            for (int p_card = 1; p_card <= 12; ++p_card) {
                if (found(p_card, available_pyramid_card_numbers)
                    && found(13 - p_card, available_pyramid_card_numbers)) {
                        possible_operations |= OPERATION_P_P;
                }
            }
        }

        if (c1 >= 0 && c0 == UNAVAILABLE) {
            // 左(c1)だけ開いている
            int c1_card = 1 + deck[c1];

            if (c1_card == 13) {
                possible_operations |= OPERATION_C1_KING;
            } else {
                if (found(13 - c1_card, available_pyramid_card_numbers)) {
                    possible_operations |= OPERATION_C1_P;
                }
            }
            possible_operations |= OPERATION_TURN;
        } else if (c1 > c0 && c0 >= 0) {
            int c0_card = 1 + deck[c0], c1_card = 1 + deck[c1];
            // ２枚開いている状態。
            if (c0_card == 13) {
                possible_operations |= OPERATION_C0_KING;
            } else {
                if (found(13 - c0_card, available_pyramid_card_numbers)) {
                    possible_operations |= OPERATION_C0_P;
                }
            }
            if (c1_card == 13) {
                possible_operations |= OPERATION_C1_KING;
            } else {
                if (found(13 - c1_card, available_pyramid_card_numbers)) {
                    possible_operations |= OPERATION_C1_P;
                }
                if (c0_card + c1_card == 13) {
                    possible_operations |= OPERATION_C0_C1;
                }
            }
            possible_operations |= OPERATION_TURN;
        } else if (c1 == UNAVAILABLE && c0 >= 0) {
            int c0_card = 1 + deck[c0];
            // ターンの最後。次がめくれるかはround依存
            if (c0_card == 13) {
                possible_operations |= OPERATION_C0_KING;
            } else {
                if (found(13 - c0_card, available_pyramid_card_numbers)) {
                    possible_operations |= OPERATION_C0_P;
                }
            }
            if (roundn < NUM_ALLOWED_ROUNDS - 1) possible_operations |= OPERATION_BIG_TURN;
        } else { // c0 == c1 == UNAVAILABLE
            ;
        }

        vector<int> next_params(5);
        vector<int> ops(3, -1);
        int next_d_rem;

        // if (possible_operations & OPERATION_SOLVED)
        if (possible_operations & OPERATION_C0_KING) {
            // 右(c0) = KING
            // turn c0
            assert(d_rem & (1 << c0));

            next_params[PARAM_P_REM] = p_rem;
            next_params[PARAM_D_REM] = next_d_rem = d_rem - (1 << c0);
            next_params[PARAM_ROUNDN] = roundn;
            next_params[PARAM_C0] = search_prev_c0(next_d_rem, c0);
            next_params[PARAM_C1] = c1;
            ops[0] = OPERATION_C0_KING; ops[1] = c0; ops[2] = UNAVAILABLE;
            add_to_queue(curr_step, curr_params, next_params, ops);
        }
        if (possible_operations & OPERATION_C0_C1) {
            // 右(c0) + 左(c1)
            // turn c0, c1
            assert(d_rem & (1 << c0));
            assert(d_rem & (1 << c1));

            next_params[PARAM_P_REM] = p_rem;
            next_params[PARAM_D_REM] = next_d_rem = d_rem - (1 << c0) - (1 << c1);
            next_params[PARAM_ROUNDN] = roundn;
            next_params[PARAM_C0] = search_prev_c0(next_d_rem, c0);
            next_params[PARAM_C1] = search_next_c1(next_d_rem, c1);
            ops[0] = OPERATION_C0_C1; ops[1] = c0; ops[2] = c1;
            add_to_queue(curr_step, curr_params, next_params, ops);
        }
        if (possible_operations & OPERATION_C0_P) {
            // 右(c0) + ピラミッドのどれか
            int c0_card = 1 + deck[c0];
            rep(i, A) {
                int pi = available_pyramid_cards[i];
                int p_card = 1 + pyramid[pi];
                if (p_card + c0_card == 13) {
                    assert(p_rem & (1 << pi));
                    assert(d_rem & (1 << c0));

                    next_params[PARAM_P_REM] = p_rem - (1 << pi);
                    next_params[PARAM_D_REM] = next_d_rem = d_rem - (1 << c0);
                    next_params[PARAM_ROUNDN] = roundn;
                    next_params[PARAM_C0] = search_prev_c0(next_d_rem, c0);
                    next_params[PARAM_C1] = c1;
                    ops[0] = OPERATION_C0_P; ops[1] = c0; ops[2] = pi;
                    add_to_queue(curr_step, curr_params, next_params, ops);
                }
            }
        }
        if (possible_operations & OPERATION_C1_KING) {
            // 左(c1) = KING
            // turn c1
            assert(d_rem & (1 << c1));
            next_params[PARAM_P_REM] = p_rem;
            next_params[PARAM_D_REM] = next_d_rem = d_rem - (1 << c1);
            next_params[PARAM_ROUNDN] = roundn;
            next_params[PARAM_C0] = c0;
            next_params[PARAM_C1] = search_next_c1(next_d_rem, c1);
            ops[0] = OPERATION_C1_KING; ops[1] = c1; ops[2] = UNAVAILABLE;
            add_to_queue(curr_step, curr_params, next_params, ops);
        }
        if (possible_operations & OPERATION_C1_P) {
            // 左(c1) + ピラミッドのどれか
            int c1_card = 1 + deck[c1];
            rep(i, A) {
                int pi = available_pyramid_cards[i];
                int p_card = 1 + pyramid[pi];
                if (p_card + c1_card == 13) {
                    assert(p_rem & (1 << pi));
                    assert(d_rem & (1 << c1));

                    next_params[PARAM_P_REM] = p_rem - (1 << pi);
                    next_params[PARAM_D_REM] = next_d_rem = d_rem - (1 << c1);
                    next_params[PARAM_ROUNDN] = roundn;
                    next_params[PARAM_C0] = c0;
                    next_params[PARAM_C1] = search_next_c1(next_d_rem, c1);
                    ops[0] = OPERATION_C1_P; ops[1] = c1; ops[2] = pi;
                    add_to_queue(curr_step, curr_params, next_params, ops);
                }
            }
        }
        if (possible_operations & OPERATION_P_KING) {
            // ピラミッドのどれか = KING
            rep(i, A) {
                int pi = available_pyramid_cards[i];
                int p_card = 1 + pyramid[pi];
                if (p_card == 13) {
                    assert(p_rem & (1 << pi));

                    next_params[PARAM_P_REM] = p_rem - (1 << pi);
                    next_params[PARAM_D_REM] = next_d_rem = d_rem;
                    next_params[PARAM_ROUNDN] = roundn;
                    next_params[PARAM_C0] = c0;
                    next_params[PARAM_C1] = c1;
                    ops[0] = OPERATION_P_KING; ops[1] = pi; ops[2] = UNAVAILABLE;
                    add_to_queue(curr_step, curr_params, next_params, ops);
                }
            }
        }
        if (possible_operations & OPERATION_P_P) {
            // ピラミッドのどれか + ピラミッドのどれか
            for (int i=0; i<A-1; ++i) {
                for (int j=i+1; j<A; ++j) {
                    int pi = available_pyramid_cards[i], pj = available_pyramid_cards[j];
                    int pi_card = 1 + pyramid[pi], pj_card = 1 + pyramid[pj];
                    if (pi_card + pj_card != 13) continue;

                    assert(p_rem & (1 << pi));
                    assert(p_rem & (1 << pj));

                    next_params[PARAM_P_REM] = p_rem - (1 << pi) - (1 << pj);
                    next_params[PARAM_D_REM] = next_d_rem = d_rem;
                    next_params[PARAM_ROUNDN] = roundn;
                    next_params[PARAM_C0] = c0;
                    next_params[PARAM_C1] = c1;
                    ops[0] = OPERATION_P_P; ops[1] = pi; ops[2] = pj;
                    add_to_queue(curr_step, curr_params, next_params, ops);
                }
            }
        }
        if (possible_operations & OPERATION_TURN) {
            // 取らずにめくる
            next_params[PARAM_P_REM] = p_rem;
            next_params[PARAM_D_REM] = next_d_rem = d_rem;
            next_params[PARAM_ROUNDN] = roundn;
            next_params[PARAM_C0] = c1;
            next_params[PARAM_C1] = search_next_c1(next_d_rem, c1+1);
            ops[0] = OPERATION_TURN; ops[1] = UNAVAILABLE; ops[2] = UNAVAILABLE;
            add_to_queue(curr_step, curr_params, next_params, ops);
        }
        if (possible_operations & OPERATION_BIG_TURN) {
            // deckを最初に戻す
            next_params[PARAM_P_REM] = p_rem;
            next_params[PARAM_D_REM] = next_d_rem = d_rem;
            next_params[PARAM_ROUNDN] = roundn;
            next_params[PARAM_C0] = -1;
            next_params[PARAM_C1] = search_next_c1(next_d_rem, 0);
            ops[0] = OPERATION_BIG_TURN; ops[1] = UNAVAILABLE; ops[2] = UNAVAILABLE;
            add_to_queue(curr_step, curr_params, next_params, ops);
        }

        // return;
    }
}


void load(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return;

    pyramid.clear();
    for (int r=1; r<=7; ++r) {
        vector<Card> row = read_card_nums(fp);
        assert(row.size() == r);
        pyramid.insert(pyramid.end(), all(row));
    }

    deck.clear();
    deck = read_card_nums(fp);

    vector<int> st(13, 0);
    tr(it, pyramid) ++st[*it];
    tr(it, deck) ++st[*it];
    rep(i, 13) assert(st[i] == 4);

    fclose(fp);
}


int main(int argc, char **argv) {
    load(argv[1]);
    bool verbose = (argc > 2);

    solve(verbose);

    return 0;
}
