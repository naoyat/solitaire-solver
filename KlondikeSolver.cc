#include <vector>
#include <string>
#include <sstream>
#include <queue>
using namespace std;

#include "common.h"
#include "cout11.h"

#undef NDEBUG
#include <cassert>


#define NUM_LINES 7
#define NUM_SUITES 4

#define LINE_BASE 0
#define GOAL      NUM_LINES

/*
priority_queue<pair<pair<int, int>, string> > _pq;
set<string> _visited;
map<string, pair<int, string> > _last_step;
vector<vector<Card> > _board;
*/

inline char num_serialize(int n) {
    // printf("num_serialize(n=%d)\n", n);
    assert(-1 <= n && n < 26);
    return 'a' + n;
}
inline int num_deserialize(char ch) {
    assert(0x60 <= ch && ch <= 'z');
    return (int)(ch - 'a');
}

class Board {
 public:
    Board(vector<vector<Card> >& lines, vector<Card>& deck) {
        // this->lines.assign(all(lines));
        assert(lines.size() == NUM_LINES);
        this->lines.assign(NUM_LINES, vector<pair<Card, bool> >());
        int cnt = 0;
        for (int i=0; i<NUM_LINES; ++i) {
            int L = lines[i].size();
            assert(1 <= L && L <= 7);
            for (int j=0; j<L; ++j) {
                this->lines[i].push_back(make_pair(lines[i][j], j == L-1));
            }
            cnt += L;
        }
        assert(cnt == 28);

        this->deck.assign(all(deck));
        assert(deck.size() == 24);

        this->goal.assign(4, -1);
        this->deck_mask = 0xffffff;
        this->deck_ofs = -1;

        this->tops.assign(NUM_LINES, -1);

        refresh(true);

        // cout << "<ctor> " << dump() << endl;
    }
    ~Board() {}

 public:
    string _serialize() {
        stringstream ss;
        // goal[]
        for (int g=0; g<NUM_SUITES; ++g) {
            ss << card_serialize(goal[g]);
        }
        // lines[]
        sort(lines.begin(), lines.end());
        for (int l=0; l<NUM_LINES; ++l) {
            int L = lines[l].size();
            ss << num_serialize(L);
            for (int i=0; i<L; ++i) {
                ss << card_serialize(lines[l][i].first);
            }
            int cnt = 0;
            for (int i=L-1; i>=0; --i) {
                if (lines[l][i].second == true) ++cnt;
                else break;
            }
            ss << num_serialize(cnt);
        }
        // deck_ofs (0-24)
        ss << num_serialize(deck_ofs);  // -1〜23 : [a-y]
        // deck_mask (24bit)
        ss << hex << deck_mask << dec;

        return ss.str();
    }

    int deserialize(string encoded) {
//        serialized = encoded;

        int i = 0;
        // goal[]
        for (int g=0; g<NUM_SUITES; ++g) {
            goal[g] = card_deserialize(encoded[i++]);
        }
        // lines[]
        assert(lines.size() == NUM_LINES);
        for (int l=0; l<NUM_LINES; ++l) {
            lines[l].clear();
            int L = num_deserialize(encoded[i++]);
            for (int j=0; j<L; ++j) {
                Card c = card_deserialize(encoded[i++]);
                lines[l].push_back(make_pair(c, false));
            }
            int cnt = num_deserialize(encoded[i++]);
            assert(0 <= cnt && cnt <= L);
            for (int j=0; j<cnt; ++j) {
                lines[l][(L-1)-j].second = true;
            }
        }

        // deck_ofs (0-24)
        deck_ofs = num_deserialize(encoded[i++]);
        assert(-1 <= deck_ofs && deck_ofs <= 24);

        // deck_mask (24bit)
        deck_mask = strtol(encoded.c_str()+i, NULL, 16);
        assert(0 <= deck_mask && deck_mask <= 0xffffff);

        // refresh(false);

        refresh(true);
        assert(encoded == serialized);

        return 0;
    }

    string dump() {
        stringstream ss;
        // deck
        if (deck_ofs == -1) {
            ss << "-";
        } else if (deck_ofs < 24) {
            ss << _full(deck_top);
        } else {
            ss << "#";
        }
        ss << " || ";
        // lines[]
        for (int l=0; l<NUM_LINES; ++l) {
            int L = lines[l].size();
            for (int i=0; i<L; ++i) {
                if (lines[l][i].second)
                    ss << _full(lines[l][i].first);
                else
                    ss << '#';
            }
            if (l < NUM_LINES-1)
                ss << " / ";
        }
        ss << " || ";
        // deck_ofs (0-24)
        ss << "ofs=" << deck_ofs;
        // deck_mask (24bit)
        ss << " mask=" << hex << deck_mask << dec;

        ss << " || ";
        // goal[]
        for (int g=0; g<NUM_SUITES; ++g) {
            ss << _full(goal[g]);
        }
        return ss.str();
        // cout << ss.str() << endl;
    }

    void debug() {
        cout << "[TOPS:";
        for (int l=0; l<NUM_LINES; ++l) {
            cout << " " << _full(tops[l]);
        }
        cout << "] [GOAL:";
        for (int g=0; g<NUM_SUITES; ++g) {
            cout << " " << _full(goal[g]);
        }
        cout << "]" << endl;
    }

    int take() {
        if (deck_ofs == -1) {
            return -1;
        } else {
            int m = 1 << deck_ofs;
            assert((deck_mask & m) == m);
            Card c = deck[deck_ofs];
            deck_mask &= ~m;
            --deck_ofs;
            m >>= 1;
            while (deck_ofs >= 0) {
                if (deck_mask & m) break;
                --deck_ofs;
                m >>= 1;
            }
            return c;
        }
    }

    int turn() {
        ++deck_ofs;
        for (int turned=0, m=1<<deck_ofs; deck_ofs<24; ++deck_ofs,m<<=1) {
            if (deck_mask & m) ++turned;
            if (turned == 3) break;
        }
        if (deck_ofs == 24) {
            deck_ofs = -1;
        }
        refresh(true);
        return deck_ofs;
    }

    void refresh(bool do_serialize) {
        for (int l=0; l<NUM_LINES; ++l) {
            int L = lines[l].size();
            if (L == 0) {
                tops[l] = -1;
                continue;
            } else {
                assert(lines[l].back().second == true);
                tops[l] = lines[l].back().first;
            }
        }

        if (deck_ofs == -1) {
            deck_top = -1;
        } else if (deck_ofs < 24) {
            deck_top = deck[deck_ofs];
        } else {
            deck_top = -1;
        }

        score = 0;
        for (int g=0; g<NUM_SUITES; ++g) {
#ifdef CLUB_ONLY
            if (g == CLUB)
                score += card_num(goal[g]) + 1;  // 0 or 1-13
            else
                score += 13;
#else
            score += card_num(goal[g]) + 1;  // 0 or 1-13
#endif
        }

        if (do_serialize) {
            serialized = _serialize();
        }
    }


public:
    vector<vector<pair<Card, bool> > > lines;
    vector<Card> deck;
    vector<Card> goal;
    vector<Card> tops;
    int deck_mask;
    int deck_ofs;
    Card deck_top;
    bool dirty;
    int score;

    string serialized;
};


priority_queue<pair<int, pair<int, string> > > _pq;
set<string> _visited;
map<string, pair<int, string> > _best;

inline void add_to_queue(Board *board, int curr_step, string current) {
    if (!found(board->serialized, _visited)) {
        _pq.push(make_pair(board->score, make_pair(-(curr_step+1), board->serialized)));

        if (found(board->serialized, _best)) {
            if (_best[board->serialized].first > curr_step) {
                _best[board->serialized] = make_pair(curr_step, current);
            }
        } else {
            _best[board->serialized] = make_pair(curr_step, current);
        }
    }
    if (current != "") {
        board->deserialize(current);
    }
}

void _show(int st, vector<vector<Card> >& lines, vector<Card>& goal) {
    printf("[%d] ", st);
#if 0
    for (int a=0; a<NUM_LINES; ++a) {
        int L = lines[a].size();
        for (int j=0; j<L; ++j) {
            cout << _full(lines[a][j]);
        }
        cout << " / ";
    }

    cout << "[";
    for (int j=0; j<NUM_SUITES; ++j) {
        cout << " " << _full(goal[j]);
    }
    cout << " ] ";
#endif
}

void show_solution(Board *board, vector<vector<Card> >& initial_lines, string& current) {
    vector<string> history;
    while (true) {
        if (!found(current, _best)) break;
        history.push_back(current);
        current = _best[current].second;
    }
    history.pop_back();
    reverse(history.begin(), history.end());

    vector<int> last_goal(4, -1);
    vector<vector<Card> > last_lines(all(initial_lines));
    int last_ofs = -1, last_mask = 0xffffff;

    for (int i=0; i<history.size(); ++i) {
        board->deserialize(history[i]);

        vector<vector<Card> > curr_lines(NUM_LINES, vector<Card>());
        for (int l=0; l<NUM_LINES; ++l) {
            int L = board->lines[l].size();
            for (int j=0; j<L; ++j) {
                curr_lines[l].push_back(board->lines[l][j].first);
            }
        }

        vector<int> curr_empty, last_empty;
        for (int j=0; j<NUM_LINES; ++j) {
            if (last_lines[j].size() == 0) {
                last_empty.push_back(j);
            }
            if (curr_lines[j].size() == 0) {
                curr_empty.push_back(j);
            }
        }

        vector<int> mapping(NUM_LINES, -1);
        int mapping_mask = 0;
        for (int a=0; a<NUM_LINES; ++a) {
            if (last_lines[a].size() == 0) continue;
            for (int b=0; b<NUM_LINES; ++b) {
                if (curr_lines[b].size() == 0) continue;
                if (last_lines[a][0] == curr_lines[b][0]) {
                    mapping[a] = b;
                    mapping_mask |= (1 << b);
                }
            }
        }
        int M = min(curr_empty.size(), last_empty.size());
        for (int j=0; j<M; ++j) {
            int a = last_empty.back(); last_empty.pop_back();
            int b = curr_empty.back(); curr_empty.pop_back();
            mapping[a] = b;
            mapping_mask |= (1 << b);
        }
        assert(__builtin_popcount(mapping_mask) >= 6);
        for (int a=0; a<NUM_LINES; ++a) {
            if (mapping[a] == -1) {
                for (int b=0,m=1; b<NUM_LINES; ++b,m<<=1) {
                    if (mapping_mask & m) continue;
                    mapping[a] = b;
                    break;
                }
                break;
            }
        }

        set<int> tmp(all(mapping));
        assert(tmp.size() == 7);

        int incl = -1, decl = -1, nb = 0;
        vector<vector<Card> > curr_lines_2(7);
        for (int a=0; a<NUM_LINES; ++a) {
            int b = mapping[a];
            curr_lines_2[a] = curr_lines[b];
            int len_diff = curr_lines_2[a].size() - last_lines[a].size();
            if (len_diff > 0) {
                incl = a;
                nb = len_diff;
            } else if (len_diff < 0) {
                decl = a;
                nb = -len_diff;
            }
        }

        // show
        _show(1+i, curr_lines_2, board->goal);

        bool turned = false;
        if (board->deck_mask != last_mask) {
            // deck -> somewhere
            assert(board->deck_ofs != last_ofs);
            assert(0 <= last_ofs && last_ofs < 24);
            cout << "(DECK->) " << _full(board->deck[last_ofs]);
            nb = 1;
        } else {
            // deck is not consumed
            if (board->deck_ofs != last_ofs) {
                // just turned
                turned = true;
                if (board->deck_ofs == -1)
                    cout << "(BIGTURN)";
                else
                    cout << "(TURN =>" << _full(board->deck_top) << ")";
            } else {
                // line -> somewhere
                printf("(LINE#%d->) ", 1+decl);
                int L = last_lines[decl].size();
                for (int j=L-nb; j<L; ++j) {
                    cout << _full(last_lines[decl][j]);
                }
            }
        }

        if (!turned) {
            if (board->goal != last_goal) {
                cout << " (->GOAL)";
            } else {
                int bottom = last_lines[incl].size() > 0 ? last_lines[incl].back() : -1;
                cout << " (->LINE#" << (1+incl) << " on " << _full(bottom) << ")";

            }
        }

        cout << endl;
        // cout << "/ " << board->dump() << endl;

        last_goal = board->goal;
        last_lines = curr_lines_2;
        last_ofs = board->deck_ofs;
        last_mask = board->deck_mask;
    }
}

int solve(vector<vector<Card> >& initial_lines, vector<Card>& initial_deck) {
    Board *board = new Board(initial_lines, initial_deck);

#if 0
    string s = board->serialize();
    board->deserialize(s);
    cout << "ORIG:";
    board->dump();
    board->turn();
    cout << "TURNED:";
    board->dump();

    board->deserialize(s);
    board->turn();
    cout << _full(board->deck_top()) << endl;
    board->turn();
    cout << _full(board->deck_top()) << endl;

    cout << "TAKEN:";
    board->dump();
#endif

    add_to_queue(board, 0, "");

    while (!_pq.empty()) {
        int score = _pq.top().first, step = -_pq.top().second.first;
        int next_step = step + 1;
        string current = _pq.top().second.second;
        _pq.pop();

        if (found(current, _visited)) continue;
        _visited.insert(current);

        board->deserialize(current);
#ifdef DEBUG
        // cout << endl;
        // cout << score << " " << step << ") " << board->dump() << endl;
        putchar('\r');
        for (int i=0; i<score; ++i) putchar('#');
        fflush(stdout);
#endif
        if (board->score == 13*4) {
            cout << "\rSOLVED." << endl;
            show_solution(board, initial_lines, current);
            break;
        }

        // cout << "1"; board->debug();
        // suppliers
        vector<pair<pair<int, int>, Card> > supplies;
        for (int l=0; l<NUM_LINES; ++l) {
            int L = board->lines[l].size();
            if (L == 0) continue;

            for (int j=L-1; j>=0; --j) {
                if (!board->lines[l][j].second) break;
                Card c = board->lines[l][j].first;
                supplies.push_back(make_pair(make_pair(l, L-j), c));
            }
        }
        if (board->deck_top != -1) {
            supplies.push_back(make_pair(make_pair(-1, 1), board->deck_top));
        }
        // cout << "2"; board->debug();

        // find all possibilities
        tr(it, supplies) {
            int from = it->first.first, nb = it->first.second;
            Card c = it->second;
            assert(c != -1);

            bool already_moved_to_vacant_line = false;
            for (int to=0; to<NUM_LINES; ++to) {
                if (to == from) continue;
                Card ct = board->tops[to];
                // cout << "to=" << to; board->debug();
                // cout << "// " << from << " " << _full(c) << " : " << to << " " << _full(ct) << endl;
                if ((ct == -1 && card_num(c)==12) || pilable(ct, c)) {
                    if (ct == -1) {
                        if (already_moved_to_vacant_line) continue;
                        already_moved_to_vacant_line = true;
                    }
                    int orig_from_len = -1;
                    int orig_to_len = board->lines[to].size();

                    if (from == -1) {
#ifdef DEBUG_VERBOSE
                        cout << "Deck";
#endif
                        board->take();
                        board->lines[to].push_back(make_pair(c, true));

                        assert(board->lines[to].size() == orig_to_len + 1);
                    } else {
                        orig_from_len = board->lines[from].size();
#ifdef DEBUG_VERBOSE
                        cout << "L" << from << "." << nb;
#endif
                        board->lines[to].insert(board->lines[to].end(),
                            board->lines[from].end()-nb, board->lines[from].end());
                        board->lines[from].erase(board->lines[from].end()-nb, board->lines[from].end());
                        if (board->lines[from].size() >= 1) {
                            board->lines[from].back().second = true;
                        }
                        assert(board->lines[from].size() == orig_from_len - nb);
                        assert(board->lines[to].size() == orig_to_len + nb);
                    }
                    // cout << "!" << board->tops << endl;
                    // cout << "!"; board->debug();
#ifdef DEBUG_VERBOSE
                    cout << "(" << _full(c) << ") on L" << to << "." << orig_to_len << "(" << _full(ct) << ")" << endl;
#endif
                    board->refresh(true);
                    add_to_queue(board, step, current);
                    // board->deserialize(current);
                    // cout << "@"; board->debug();

                    if (from != -1) {
                        assert(board->lines[from].size() == orig_from_len);
                    }
                    assert(board->lines[to].size() == orig_to_len);
                }
            }

            if (nb == 1) {
                int num = card_num(c), suite = card_suite(c);
                Card cg = board->goal[suite];
                if (card_num(cg) == num - 1) {
                    int orig_from_len = -1;

                    if (from == -1) {
#ifdef DEBUG_VERBOSE
                        cout << "Deck";
#endif
                        board->take();
                    } else {
                        orig_from_len = board->lines[from].size();

#ifdef DEBUG_VERBOSE
                        cout << "L" << from << "." << nb;
#endif
                        board->lines[from].pop_back();
                        if (board->lines[from].size() >= 1) {
                            board->lines[from].back().second = true;
                        }

                        assert(board->lines[from].size() == orig_from_len - 1);
                    }
#ifdef DEBUG_VERBOSE
                    cout << "(" << _full(c) << ") on G" << suite << "(" << _full(cg) << ")" << endl;
#endif
                    board->goal[suite] = c;

                    board->refresh(true);
                    add_to_queue(board, step, current);
                    // board->deserialize(current);

                    if (from != -1) {
                        // printf("[%d] %d ==? %d\n", from, board->lines[from].size(), orig_from_len);
                        assert(board->lines[from].size() == orig_from_len);
                    }
                }
            }
        }

        // turn
        {
#ifdef DEBUG_VERBOSE
            printf("Turn (ofs=%d ->)\n", board->deck_ofs);
#endif
            board->turn();
            add_to_queue(board, step, current);
            // board->deserialize(current);
        }

//        break;
    }

    delete board;
    return 0;
}

void show_usage() {
    printf("./KlondikeSolver <game-file>\n");
}

int load(char *path, vector<vector<Card> >& lines, vector<Card>& deck) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return -1;

    deck.clear();
    lines.clear();

    set<Card> s;

    for (int i=0; i<7; ++i) {
        lines.push_back(read_cards(fp));
        // cout << _full_cards(lines[i]) << endl;
        s.insert(all(lines[i]));
    }

    deck = read_cards(fp);
    // cout << _full_cards(deck) << endl;
    s.insert(all(deck));
    // printf("%d\n", s.size());
    assert(s.size() == 13*4);

    fclose(fp);

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        show_usage();
        return 0;
    }

    vector<vector<Card> > lines;  // 1+2+3+4+5+6+7=28
    vector<Card> deck;  // 24=8*3

    if (load(argv[1], lines, deck) == -1) {
        show_usage();
        return 0;
    }

    solve(lines, deck);

    return 0;
}
