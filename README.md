# MSソリティアソルバ

12月のデイリーチャレンジをやってて、たまにどうしても解けないやつがあるので書いた。
C++の実装練習にちょうど良かった。
競プロ用のマクロとか使ってるし綺麗なコードとは言えないだろうけど。

データでのカードの表現は `([A234567890JQK]|10)([sdhc])?` を想定している。
10は"0"でも可。（全て１文字で表せてパースが楽なので最初は0で書いていた）

全てのカードを確認できるぐらいなら恐らく解けてる Spider については割愛。

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
pyramid: Q6J8847Q496Q29K6A39K35A58A23
deck: 0J45KQ2067704JKJ78350A29
536553 <solved>
1) from pyramid d3 (J)
2) from pyramid d1 (Q)
3) turn deck (J)
4) from pyramid d8 (Q)
...
46) turn deck (A)
47) turn deck (2)
48) from pyramid b4 (A)
49) from pyramid a2 (2)
50) from pyramid a3 (3)
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
################################################
 ~ ~ ~ ~ | ~ ~ ~ ~ | 7509Q82 | JAK6K08 | 9452787 | 74QQ2J3 | 6Q9365 | J903AK | K864AA | 345J02 |
(1) A♠ at line 7 -> GOAL
 ~ ~ ~ ~ | ~ ~ ~ A | 7509Q82 | JAK6K08 | 9452787 | 74QQ2J3 | 6Q9365 | J903AK | K864A | 345J02 |
(2) 2♠ at line 1 -> GOAL
 ~ ~ ~ ~ | ~ ~ ~ 2 | 7509Q8 | JAK6K08 | 9452787 | 74QQ2J3 | 6Q9365 | J903AK | K864A | 345J02 |
(3) 3♠ at line 4 -> GOAL
 ~ ~ ~ ~ | ~ ~ ~ 3 | 7509Q8 | JAK6K08 | 9452787 | 74QQ2J | 6Q9365 | J903AK | K864A | 345J02 |
...
(92) J♣ at line 2 -> GOAL
 ~ ~ ~ ~ | Q J K K |  |  |  |  |  |  | K | KQ |
(93) Q♣ at line 8 -> GOAL
 ~ ~ ~ ~ | Q Q K K |  |  |  |  |  |  | K | K |
(94) K♡ at line 8 -> GOAL
 ~ ~ ~ ~ | K Q K K |  |  |  |  |  |  | K |  |
(95) K♣ at line 7 -> GOAL
 ~ ~ ~ ~ | K K K K |  |  |  |  |  |  |  |  |
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

〜

♣だけ最速で揃えたい、みたいな問題（デイリーチャレンジ2017/1/25版）には、scoringを変えれば対応できることを確認。
（fcs125.cc参照）

```fcs125.diff
--- FreeCellSolver.cc	2017-01-03 03:42:12.000000000 +0900
+++ fcs125.cc	2017-01-25 08:03:26.000000000 +0900
@@ -460,8 +460,11 @@
         rep(l, NUM_LINES) if (_board[l].size() == 0) ++max_movable;

         int score = 0;
-        rep(s, NUM_SUITES) score += card_num(_board[GOAL][s]);
-        if (score == 48) {
+        rep(s, NUM_SUITES) {
+            score += card_num(_board[GOAL][s]) * (s == CLUB ? 1 : 0);
+        }
+        // if (score == 48) {
+        if (score == 12) {
             printf("\n"); // SOLVED.
             return here;
         }
```

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
############################(53)
1) //////AJ6AJ0Q 9-	; pyramid g1<A> + pyramid g7<Q>
2) //////.J6AJ0. 9-	; TURN
3) //////.J6AJ0. 99	; TURN
4) //////.J6AJ0. 39	; deck(left)<3> + pyramid g6<0>
5) /////#####5/.J6AJ.. 49	; TURN
...
49) /4. 9-	; deck(left)<9> + pyramid b1<4>
50) 5 K-	; TURN
51) 5 KK	; TURN
52) 5 3K	; TURN
53) 5 83	; deck(left)<8> + pyramid a1<5>
54) . A3	;
```

## KlondikeSolver

```
cat <<EOF > klondike_sample_1.txt
8c
3s3d
Kc4h2d
Jd3c7c9s
QhQd5s0hAc
7s3h5d4cAh0s
JcAsJs5cQc6hQs
2s6sKh 2h9h9d Jh8d2c 4s5h0d 0cAd4d 7d8s6d Kd8hKs 7h6c9c
EOF
```

Klondikeは全てのカードの配置を確認できるぐらいなら解けるはずだが、手数を制限されている場合のためにソルバを書いた。

```
$ ./KlondikeSolver klondike_sample_1.txt
SOLVED.#############################################
[1] (LINE#5->) A♣ (->GOAL)
[2] (TURN =>K♡)
[3] (TURN =>9♢)
[4] (TURN =>2♣)
[5] (DECK->) 2♣ (->GOAL)
[6] (DECK->) 8♢ (->LINE#4 on 9♠)
...
[125] (LINE#2->) K♡ (->GOAL)
[126] (LINE#6->) K♠ (->GOAL)
```
