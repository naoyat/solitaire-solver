#!/usr/bin/env python

from collections import defaultdict
import numpy as np

import click


num_to_class = {'A':1, '2':2, '3':3, '4':4, '5':5, '6':6, '7':7, '8':8, '9':9, '10':10, 'J':11, 'Q':12, 'K':13,
                '0':10}  # 0 for abbrev.
suite_to_class = {'s':1, 'd':2, 'h':3, 'c':4}

class_to_num = [None, 'A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K']
class_to_suite = [None, 's', 'd', 'h', 'c']


def parse(game_path):  # returns 7 rows of 8 cols
    c, n, s = defaultdict(int), defaultdict(int), defaultdict(int)
    data = []

    with open(game_path, 'r') as fp:
        for row in fp:
            row = row.rstrip()
            if row == '': continue

            items = row.split(' ')
            if len(items) not in (4, 8): continue
            cards_in_row = [(num_to_class[item[:-1]]-1, suite_to_class[item[-1]]-1) for item in items]

            for num, suite in cards_in_row:
                assert 0 <= num <= 12
                assert 0 <= suite <= 3
                c[num*13 + suite] += 1
                n[num] += 1
                s[suite] += 1

            data.append(cards_in_row)

    for card, occ in c.items():
        if occ >= 2:
            print class_to_num[1+(card / 13)] + class_to_suite[1+(card % 13)], occ
    assert len(c) == 52 and len(n) == 13 and len(s) == 4

    return data


def transpose(data): # 7x8 -> 8x7
    lines = []
    for c in range(8):
        line = []
        for r in range(7 if c < 4 else 6):
            line.append(data[r][c])
        lines.append(line)
    return lines


def dump_data(data):
    for row in data:
        print ' '.join(class_to_num[1+num] + class_to_suite[1+suite] for num, suite in row)


@click.command()
@click.argument('game-path', type=click.Path(exists=True, readable=True), default='game_8758887.teacher')
def main(game_path):
    data = parse(game_path)

    dump_data(data)
    print
    dump_data(transpose(data))


if __name__ == '__main__':
    main()
