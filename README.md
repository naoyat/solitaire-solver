# MSソリティアソルバ

12月のデイリーチャレンジをやってて、たまにどうしても解けないやつがあるので書いた。
C++の実装練習にちょうど良かった。
競プロ用のマクロとか使ってるし綺麗なコードとは言えないだろうけど。

データでのカードの表現は `([A234567890JQK]|10)([sdhc])?` を想定している。
10は"0"でも可。（全て１文字で表せてパースが楽なので最初は0で書いていた）

全てのカードを確認できるぐらいなら恐らく解けてる Klondike と Spider については割愛。

## TriPeaksSolver

```
$ cat <<EOF > tripeaks_sample_2.txt
   A  2  3
  K3 5A 58
 6Q29K6A39
Q6J8847Q49
0J45KQ2067704JKJ78350A29
EOF

$ ./TripeaksSolver tripeaks_sample_2.txt
```
undoを駆使しながらピラミッドとデッキのカードを調べてデータ化する必要がある。
最後の行が手元のカードデッキ。（上のデータだと、最初に表示されているのが10で、めくっていくとK,4,5,...が続く）

DFSでバックトラックしながら解いてる。

## FreeCellSolver

最初DFSで書いたやつが謎のEXC_BAD_ACCESSで落ちまくって、再帰しすぎると関数呼び出しで死ぬ、という自然の摂理がC++にも及ぶことを体感したのでBFSで書き直した。

```
cat <<EOF > freecell_8130153.txt
7d Jd 9c 7s 6c Js Kc 3d
5s Ad 4d 4h Qh 9d 8c 4c
0h Ks 5h Qs 9s 0s 6h 5c
9h 6s 2h Qc 3c 3h 4s Jh
Qd Kh 7c 2c 6d Ah Ac 0c
8s 0d 8d Jc 5d Kd As 2d
2s 8h 7h 3s
EOF

$ ./FreecellSolver freecell_8130153.txt
```

そしてデータ入力が面倒くさかったので、スナップショット画像を勝手に読み取らせることにした。
A〜Kの文字と♠♢♡♣が来る位置は分かっているので識別は割と容易なはず…と思って
当該位置のビットイメージ（を1ピクセルおきに読んで白黒化したもの）をクラス分類する1層のNeural Networkをkerasで書いた。
kerasなのは普段使ってて馴染んでるから。

```
$ python freecell_model.py build
```
でサンプル画像（＋教師）からモデルを学習しておいて

```
$ python freecell_solver.py solve snapshot_picture.png
```

で画像認識、からのソルバ処理が行える。（画像認識だけなら`solve`の代わりに`scan`で実行）

## PyramidSolver

```
cat <<EOF > pyramid_sample_1.txt
5
4 7
Q 4 3
10 9 J 7
8 3 10 K 7
8 4 J 5 Q 5
A J 6 A J 10 Q
9934KK8592726A638AQK21062
EOF
```

TriPeaks同様、undoを駆使しながらピラミッドとデッキのカードを調べてデータ化する必要がある。

```
$ ./PyramidSolver pyramid_sample_1.txt
```
