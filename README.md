# R(8,10) ≥ 346

A colouring of the edges of $K_{345}$ with **no red $K_8$ and no blue $K_{10}$**, which proves

$$R(8,10) \ge 346.$$

The previous bound was $R(8,10) \ge 345$: Kuznetsov's circulant colouring of $K_{342}$ gives 343
(recorded in revision 18 of Radziszowski's survey), and Cremer (2026) raised it to 345 in two steps.
This repository contains the colouring, four independent checkers, and the search code that found it.

**Paper:** [`paper/main.pdf`](paper/main.pdf) · **Short note:** [`docs/r810-346.md`](docs/r810-346.md)

---

## Verify it in one command

```sh
./verify/verify.sh
```

This checks both deposited colourings against their SHA-256 hashes, builds four independent checkers
from source, and runs all of them. It takes about a minute and needs only a C compiler and Python 3
(for the graph6 cross-check). No dependencies, no network.

The expected output is, for each witness:

```
count   : n=345  red K8 = 0, blue K10 = 0, total 0
verify  : omega_red=7 omega_blue=9
verify2 : red K8: none ... blue K10: none ... VALID
enum    : ALREADY VALID
```

If you would rather not run the script, the essential check is a single line:

```sh
make -C verify && ./verify/count colorings/r810-n345-witness-A.txt 8 10 /dev/null
```

which prints the exact numbers of red $K_8$'s and blue $K_{10}$'s in the colouring.

## The colourings

| file | encoding | red edges | SHA-256 |
|---|---|---|---|
| `colorings/r810-n345-witness-A.txt` | matrix, $345\times345$, `1` = red, zero diagonal | 26 045 | `0af45901…47935b12` |
| `colorings/r810-n345-witness-A.g6` | graph6 of the red graph | — | `4135f58f…54158d62` |
| `colorings/r810-n345-witness-B.txt` | matrix, same conventions | 26 037 | `60416408…52f6eccf` |
| `colorings/r810-n345-witness-B.g6` | graph6 of the red graph | — | `6bc68594…7fcef9` |

Both colourings were found by the search described below, from different starting states, and each
was verified independently of the other. Witness A is the one described in the paper. Witness B came
from a lane that solved from a 1-conflict state; its best one-vertex extension is cheaper than A's
(28 358 versus 32 764 conflicts), which makes it the better starting point for anyone attempting
$R(8,10) \ge 347$.

`colorings/SHA256SUMS` is checked by the verification script.

## What is in here

```
colorings/   the two colourings, as matrices and as graph6, with checksums
verify/      four independent checkers (see below) + a matrix→graph6 converter + verify.sh
search/      the search engine that found the colourings, and the exact twin-scan utility
docs/        the technical note, a note on the limits of the method, and the screening record
paper/       LaTeX source and PDF of the paper
```

### The four checkers

They share no code, which is the point:

| program | method |
|---|---|
| `verify/count.c` | exhaustive enumeration of red $K_8$ and blue $K_{10}$ |
| `verify/verify.c` | maximum-clique branch and bound on both colours |
| `verify/verify2.c` | depth-first search for a clique of the target size only |
| `verify/enum.c` | independent clique enumerator; also does exhaustive 1- and 2-flip repair checks |

### The search

`search/engine.c` is a single-threaded C program doing exact-conflict local search: it maintains the
complete set of monochromatic cliques, with per-edge counts, so the effect of every edge flip is exact
rather than sampled, and drives them down by tabu search with Metropolis acceptance, conflict-directed
kicks and a patch move.

It is research code, not a polished tool. The command that produced a colouring from a 345-vertex
input with six conflicts was:

```sh
./engine start.txt 8 10 0 run 20000000 32 222 3600 1000 out 1.607 64
```

Arguments, in order: input matrix, red clique size, blue clique size, colour-convention flag
(`0` = the file's 1s are red), mode, iteration cap, candidate sample size, random seed, CPU-second
budget, stall threshold, output prefix, red weight $\binom{10}{2}/\binom{8}{2}$, cap on new conflicts
per flip.

`search/twin.c` computes, for every vertex and both colours of the joining edge, the exact number of
conflicts created by copying that vertex into a new one — the operation from which each campaign
starts:

```sh
./twin colorings/r810-n345-witness-B.txt 8 10 0
```

## How the colourings were found

Cremer's method (arXiv:2610.12122): add one vertex to a record colouring, then repair the
monochromatic cliques the addition creates. Applied once more than he did, and driven to zero.

From Cremer's colouring of $K_{344}$, adding a copy of vertex 335 joined by a blue edge gives a
$K_{345}$ with 29 814 blue $K_{10}$'s — the exact minimum over all 688 choices of vertex and edge
colour. Exact-conflict local search reduced that to zero over four rounds of four parallel lanes,
with fresh random seeds at each round:

```
29 814 → 97 → 96 → 41 → 23 → 6 → 0
```

One thing worth knowing if you run this: a single long search segment is misleading. In one run the
improvement rate fell to 0.003 conflicts per 100 seconds at 97 conflicts, which looks like
convergence. It was not — restarting from the same colouring with new seeds reached 41 within two
hours. Judge progress per round, not per segment.

## What this repository does not claim

* It does not determine $R(8,10)$. The upper bound is 4 402; this moves the lower end by one.
* It makes no claim about $K_{346}$. A failed search for a $(8,10)$-colouring on 346 vertices would
  not be evidence that none exists, and that search was not run.
* The colourings are not canonical or known to be part of a family. Whether they are isolated among
  345-vertex colourings is open.

Appendix A of the paper — reproduced in `docs/repair-distance-note.md` — describes a related negative
result: for six smaller cells, an exact bounded-distance SAT screen proves that no valid colouring lies
within 9 to 17 edge changes of the corresponding one-vertex extension, so for those cells the method
cannot succeed from those starting colourings at all. `docs/cremer-sweep.md` is the full screening record.

## Ancestry and credit

The colouring descends from Cremer's $K_{344}$, which descends from Kuznetsov's circulant $K_{342}$.
If you use these colourings, please cite both this work and:

* F. Cremer, *Lower bounds for Ramsey numbers: R(6,8) ≥ 135 and R(8,10) ≥ 345*, arXiv:2610.12122
  (2026), with colourings at <https://github.com/fritzcremer/ramsey-lower-bounds>.
* E. Kuznetsov, *Computational lower limits on small Ramsey numbers*, arXiv:1505.07186 (2016).
* S. P. Radziszowski, *Small Ramsey numbers*, Electronic Journal of Combinatorics, Dynamic Survey
  DS1, revision 18 (24 April 2026).

The search was carried out by an AI system (Open Math Model, running DeepSeek V4.1 Flash); the paper
describes what that system did and what was verified independently of it.

## Licence

Code (`verify/`, `search/`) is released under the MIT licence, see `LICENSE`.
The colourings (`colorings/`) and documents (`docs/`, `paper/`) are released under
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
