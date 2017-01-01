#!/usr/bin/env python
# -*- encoding: utf-8 -*-
import os
import re
import sys
import numpy as np
from keras.models import Sequential, model_from_json
from keras.layers import Dense
from keras.callbacks import EarlyStopping

import click

from freecell_image_loader import scan_freecell_image
from freecell_game_loader import parse, class_to_num, class_to_suite


def flatten_2d(mx):
    return np.array([item for row in mx for item in row])
    # return [item for row in mx for item in row]


def one_dim(a):
    M, H, W = a.shape
    return a.reshape((M, H*W))


def one_of_k(n, k):
    z = np.zeros(k)
    z[n] = 1
    return z


class data_X:
    def __init__(self, Xs=[]):
        self._Xs = Xs

    def append(self, Xs):
        self._Xs += Xs

    def clear(self):
        self._Xs = []

    def data(self):
        a = np.array(self._Xs)
#        print '(X)', a.shape
        return one_dim(a)

class data_y:
    def __init__(self, k, ys=[]):
        self._ys = ys
        self.k = k

    def append(self, ys):
        self._ys += ys

    def clear(self):
        self._ys = []

    def data(self):
        a = np.array([one_of_k(num, self.k) for num in self._ys])
#        print '(y)', a.shape
        return a


class Model:
    def __init__(self, X_nums=[], y_nums=[], X_suites=[], y_suites=[]):
        self.model_nums = None
        self._X_nums, self._y_nums = data_X(X_nums), data_y(13, y_nums)
        self.model_suites = None
        self._X_suites, self._y_suites = data_X(X_suites), data_y(4, y_suites)

    def add_Xs(self, Xs):
        X_nums, X_suites = zip(*Xs)
        self._X_nums.append(X_nums)
        self._X_suites.append(X_suites)

    def add_ys(self, ys):
        y_nums, y_suites = zip(*ys)
        self._y_nums.append(y_nums)
        self._y_suites.append(y_suites)

    def X_nums(self):
        return self._X_nums.data()

    def X_suites(self):
        return self._X_suites.data()

    def y_nums(self):
        return self._y_nums.data()

    def y_suites(self):
        return self._y_suites.data()

    #
    def learn(self):
        def _learn(Xs, ys):
            # print "LEARN", Xs[0], ys[0]
            nb_epoch = 100
            callbacks = []
            patience = 10
            early_stopping = EarlyStopping(monitor='val_acc', mode='max', patience=patience)
            callbacks.append(early_stopping)

            input_dim = len(Xs[0])
            nb_classes = len(ys[0])

            model = Sequential()
            model.add(Dense(nb_classes, input_dim=input_dim, init='normal', activation='sigmoid'))
            # model.add(Dense(nb_classes, init='normal', activation='sigmoid'))
            model.compile(loss='categorical_crossentropy', optimizer='rmsprop', metrics=['accuracy'])

            hist = model.fit([Xs], [ys], nb_epoch=nb_epoch, verbose=2, validation_split=0.25, callbacks=callbacks)
            # save model
            return model

        self.model_nums = _learn(self.X_nums(), self.y_nums())
        self.model_suites = _learn(self.X_suites(), self.y_suites())

        self.save()

    def evaluate(self, Xs, ys):
        def _evaluate(model, Xs, ys):
            loss, acc = model.evaluate([Xs], [ys], batch_size=32, verbose=0)
            print loss, acc

        _X_nums, _X_suites = zip(*Xs)
        _y_nums, _y_suites = zip(*ys)

        X_nums, y_nums = data_X(_X_nums), data_y(13, _y_nums)
        # print '>', X_nums.data().shape, y_nums.data().shape
        _evaluate(self.model_nums, X_nums.data(), y_nums.data())

        X_suites, y_suites = data_X(_X_suites), data_y(4, _y_suites)
        _evaluate(self.model_suites, X_suites.data(), y_suites.data())

    def load(self):
        def _load_model(model_name):
            model_json_path = 'model_data/%s_model.json' % model_name
            param_hdf5_path = 'model_data/%s_param.hdf5' % model_name
            if not os.path.isdir('model_data') \
                or not os.path.exists(model_json_path) \
                or not os.path.exists(param_hdf5_path):
                raise Exception('model data not found')

            with open(model_json_path, 'r') as fp:
                json_string = fp.read()
                model =  model_from_json(json_string)

            model.compile(loss='categorical_crossentropy', optimizer='rmsprop', metrics=['accuracy'])
            model.load_weights(param_hdf5_path)

            return model

        self.model_nums = _load_model('nums')
        self.model_suites = _load_model('suites')

    def save(self):
        def _save_model(model_name, model):
            if not os.path.isdir('model_data'):
                os.path.mkdir('model_data')
            model_json_path = 'model_data/%s_model.json' % model_name
            param_hdf5_path = 'model_data/%s_param.hdf5' % model_name

            with open(model_json_path, 'w') as fp:
                fp.write(model.to_json())
            model.save_weights(param_hdf5_path, overwrite=True)

        _save_model('nums', self.model_nums)
        _save_model('suites', self.model_suites)

    def classify(self, Xs):
        def _classify(model, Xs):
            return [np.argmax(pred) for pred in model.predict([Xs], verbose=0)]

        _X_nums, _X_suites = zip(*Xs)
        X_nums, X_suites = data_X(_X_nums), data_X(_X_suites)

        pred_nums = _classify(self.model_nums, X_nums.data())
        pred_suites = _classify(self.model_suites, X_suites.data())

        return zip(pred_nums, pred_suites)


@click.group()
def cli():
    pass


snapshot_dir_path = 'freecell_snapshots'

@cli.command()
def build():
    _Xs, _ys = [], []

    print 'LOADING DATA...'
    assert os.path.isdir(snapshot_dir_path)
    for name in os.listdir(snapshot_dir_path):
        if name.endswith('.png'):
            image_path = snapshot_dir_path + '/' + name
            teacher_path = re.sub(r'\.png$', '.teacher', image_path)
            if os.path.exists(teacher_path):
                _Xs += scan_freecell_image(image_path, step=2) # 5ぐらいでも行けたけど
                _ys += parse(teacher_path)
                sys.stdout.write('[%s]' % name[:-4])
                sys.stdout.flush()
    sys.stdout.write('\n')

    Xs = flatten_2d(_Xs)
    ys = flatten_2d(_ys)

    m = Model()
    m.add_Xs(Xs)
    m.add_ys(ys)

    print 'LEARNING...'
    m.learn()

    print 'EVALUATING...'
    m.evaluate(Xs, ys)

    print 'DONE'


if __name__ == '__main__':
    cli()
