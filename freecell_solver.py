#!/usr/bin/env python
import os
# import numpy as np
import click

from freecell_image_loader import scan_freecell_image
from freecell_game_loader import parse, class_to_num, class_to_suite

from freecell_model import Model, flatten_2d


@click.group()
def cli():
    pass


@cli.command()
@click.argument('image-path', type=click.Path(exists=True, readable=True))
@click.option('--short-form', is_flag=True, default=True)
def scan(image_path, short_form):
    Xs = flatten_2d(scan_freecell_image(image_path, 2))

    m = Model()
    m.load()
    pred = m.classify(Xs)

    if short_form:
        _pred = ['%s%s' % (class_to_num[1+num][-1], class_to_suite[1+suite]) for num, suite in pred]
    else:
        _pred = ['%s%s' % (class_to_num[1+num], class_to_suite[1+suite]) for num, suite in pred]

    # save_txt_path = image_path.replace('.png', '.txt')
    # with open(save_txt_path, 'w') as fp:
    for r in range(7):
        print ' '.join(_pred[r*8 + c] for c in range(8 if r < 6 else 4))


@cli.command()
@click.argument('game-path', type=click.Path(exists=True, readable=True))
def solve(game_path):
    if game_path.endswith('.png'):
        Xs = flatten_2d(scan_freecell_image(game_path, 3))

        m = Model()
        m.load()
        pred = m.classify(Xs)
        _pred = ['%s%s' % (class_to_num[1+num][-1], class_to_suite[1+suite]) for num, suite in pred]

        save_txt_path = game_path.replace('.png', '.txt')
        with open(save_txt_path, 'w') as fp:
            for r in range(7):
                row = ' '.join(_pred[r*8 + c] for c in range(8 if r < 6 else 4))
                print row
                fp.write(row + '\n')
        game_path = save_txt_path
    else:
        pass

    os.system('./FreeCellSolver %s' % game_path)


if __name__ == '__main__':
    cli()
