#!/usr/bin/env python
from __future__ import print_function

from PIL import Image
import os
import sys
import math

import numpy as np
from collections import defaultdict

import click


def _scan_border(gen):
    bright, dark = -1, -1

    borders = []
    for i, (r, g, b) in enumerate(gen):
        if (r + g + b) > 650:
            if bright != i-1: borders.append((True, i))
            bright = i
            dark = -1
        else:
            if dark != i-1: borders.append((False, i))
            bright = -1
            dark = i
    return borders


def _render(borders):
    _tmp = []
    bright_from = -1
    for onoff, i in borders:
        if onoff == True:
            bright_from = i
        else:
            _tmp.append((bright_from, i))
            bright_from = -1
    print(_tmp)


def scan_border_vert(im, x):
    width, height = im.size

    borders = _scan_border(im.getpixel((x, y)) for y in range(height))
    _render( borders )


def scan_border_horiz(im, y):
    width, height = im.size

    borders = _scan_border(im.getpixel((x, y)) for x in range(width))
    _render( borders )


def check_is_red(im, left, top, right, bottom):
    st = defaultdict(int)

    for y in range(top, bottom, 10):
        for x in range(left, right, 10):
            r, g, b = im.getpixel((x, y))
            if r > 153 and g > 153 and b > 153:
                st['white'] += 1
            elif r > 102 and g < 102 and b < 102:
                st['red'] += 1
            elif r < 102 and g < 102 and b < 102:
                st['black'] += 1

    is_red = ('red' in st)
    return is_red


def scan_image(pix, left, top, right, bottom, step=2):
    width, height = right - left, bottom - top

    _X = []
    for y in range(top, bottom, step):
        row = []
        for x in range(left, right, step):
            r, g, b = pix[x, y]
#            avg = float(r + g + b) / 3 / 256  # [0.0, 1.9)
#            avg = math.sqrt(avg)
#            c = int(avg * scale)
            if r >= 128 and g >= 128 and b >= 128:  # white
                c = 1
            else:  # red/black
                c = 0

            row.append(c)
        _X.append(row)

    return np.array(_X)


pix_mode = True # 0.452
# pix_mode = False # 0.513

def scan_freecell_image(image_path, step=2):
    assert os.path.exists(image_path)

    im = Image.open(image_path)

    width, height = im.size
    assert (width, height) == (750, 1334)

    pix = im.load()

    # scan_border_horiz(im, height/3)
    # 24 (96) 114 (186) 204 (276) 294 (366) 384 (456) 474 (546) 564 (636) 654 (726)

    data = []

    num_processed = 0
    for r in range(7):
        top = 338 + 35*r
        bottom = top + 35

        row = []
        for c in range(8 if r < 6 else 4):
            left = 24 + 90*c  # 24, 114, ..., 654 (-96)
            right = left + 72  # 96, 186, ..., 726 (-24)
            middle = (left + right) / 2
            # scan_border_vert(im, left + 3)
            # print((c, r))
            img_left = scan_image(pix, left, top, middle, bottom, step)
            img_right = scan_image(pix, middle, top, right, bottom, step)
            row.append((img_left, img_right))
            num_processed += 1

        data.append(row)

    assert num_processed == 52

    return data


def visualize(X):
    H, W = X.shape
    for y in range(H):
        for x in range(W):
            c = X[y][x]
            sys.stdout.write(".*"[c])
        sys.stdout.write('\n')
    sys.stdout.write('\n')


@click.command()
@click.argument('image-path', type=click.Path(exists=True, readable=True), default='freecell_snapshots/game_8758887.png')
# @click.argument('scale', type=int, default=2)
@click.argument('step', type=int, default=2)
def main(image_path, step):
    def flatten_2d(mx):
        return np.array([item for row in mx for item in row])

    Xs = flatten_2d(scan_freecell_image(image_path, step=step))
    X_nums, X_suites = [np.array(x) for x in zip(*Xs)]

    for X_num, X_suite in zip(X_nums, X_suites):
        visualize(X_num)
        visualize(X_suite)


if __name__ == '__main__':
    main()
