# How far is a repair?

**Exact Hamming-distance bounds for the "add a vertex, then repair" method on nine Ramsey
colourings**

Draft, 3 November 2026. Companion to `results/cremer-sweep.md` (raw results) and
`results/smvzvl5gk_m7_distance_screen.md` (the distance screen itself).

> **Note added later.** The $(8,10)$ campaign this note describes as a "live trajectory" reached zero
> conflicts on 345 vertices the same day, proving $R(8,10)\ge 346$ (`results/r810-346.md`). The
> distance bounds in §3 are unaffected — no screen finishes at $n=345$ — but the cell moved, which is
> exactly the outcome §4(d) says the distance analysis could not predict.

## Abstract

Adding one vertex to a record colouring and repairing the monochromatic cliques it creates is how
the current lower bounds of $R(6,8)$ and $R(8,10)$ were obtained (Cremer, arXiv:2610.12122). We
measure, for nine cells, the exact cost of that first step — the number of red $K_r$ and blue $K_s$
created by every one of the $2n$ possible twin insertions — and then decide, by CEGAR-SAT, the
repository question: **does any valid colouring of the target order lie within $k$ edge changes of
that state?** For six cells we prove an answer: the nearest valid colouring is at least $9$ to $17$
edge changes away. Two consequences follow. First, the method cannot improve these bounds from these
bases however long it runs or however well it is tuned — this is a distance statement, not a search
report. Second, and more transferable: the conflict count of the $+1$ state does not rank
repairability, so seeding a repair campaign by fewest conflicts selects the wrong cells. We also
record the closest known approach to $R(3,10)=41$: a 40-vertex colouring with two monochromatic
triangles, provably five or more edge changes from any colouring.

## 1. Setting and question

A colouring of $K_n$ with no red $K_r$ and no blue $K_s$ proves $R(r,s)\ge n+1$. Call a monochromatic
clique a *conflict*. The method under study takes a colouring that attains the current bound at $n_0$
vertices, appends a new vertex $v'$ (in practice a copy of an existing vertex $v$, the *twin*), then
repairs conflicts by flipping edges. A repair to zero conflicts is a new bound at $n_0+1$.

Whether that is likely is a question about distance, and inside a Hamming ball it is decidable exactly:
*is there an $(r,s)$-colouring of $K_{n_0+1}$ within Hamming distance $k$ of the $+1$ state?* We answer
it for increasing $k$ until the instances stop finishing.

## 2. The first step, exactly

For a twin insertion the conflict count is immediate. If $v'$ copies $v$ and the twin edge $vv'$ has
colour $c$, then the only new conflicts are $\{v,v'\}\cup C$ with $C$ a $c$-clique of size $r-2$
(resp. $s-2$) inside $N_c(v)$; the other colour contributes none, because the base colouring is
valid. One clique pass per colour and per vertex therefore gives all $2n$ twin counts exactly.

| cell | $n_0$ | best twin | conflicts after $+1$ | source of base |
|---|---|---|---|---|
| (3,10) | 39 | $v_{35}$, red | **6** (6 red $K_3$) | Exoo |
| (3,11) | 46 | $v_{20}$, red | 8 | Exoo |
| (4,6) | 35 | $v_{31}$, red | 24 | Exoo |
| (4,7) | 48 | $v_{15}$, red | 40 | Exoo |
| (4,9) | 72 | $v_0$, red (vertex-transitive) | 72 | Exoo |
| (5,6) | 58 | $v_{56}$, red | 136 | Exoo |
| (6,8) | 134 | $v_{115}$, red | 338 | Cremer |
| (6,9) | 182 | $v_0$, red | 2 830 | Kuznetsov |
| (8,10) | 344 | $v_{335}$, **blue** | 29 814 | Cremer |

Two remarks. The red twin is cheapest on every cell except (8,10), where the roles of the two clique
sizes force the blue one. And the spread across twins is large — for (6,8) the median red twin costs
2 473 against a minimum of 338 — so the choice of twin matters more than most parameter tuning.

## 3. Bounded distance: what is proved

**Encoding.** One Boolean per edge ($1$ = red); hard clauses say "no red $K_r$" and "no blue $K_s$",
added lazily: solve, enumerate every violated clique exactly, add its blocking clause, re-solve
(CEGAR). The large side (e.g. $\binom{40}{10}=8.5\times10^{8}$ blue 10-sets) is never enumerated. The
distance constraint is a totalizer over flip variables $d_e \leftrightarrow (x_e \neq \text{base}_e)$,
i.e. "at most $k$ edges differ from the base".

**Controls.** A SAT negative is worthless without a positive control, because an over-constrained
encoding returns UNSAT for everything. We therefore corrupt a valid colouring with a known number of
edge flips and require SAT at that distance and UNSAT one below it. Concretely, for the (4,6)
colouring of $K_{35}$ with 5 edges flipped, the encoding returns SATISFIABLE with a verified witness
at $k=5$ and UNSAT at $k=4$. The same control was applied to two independently written encodings.
Solvers: CaDiCaL 1.5.3 and MiniSat 2.2, which agree on every rung both finish.

**Results.** Largest $k$ for which the instance is UNSAT, i.e. no valid colouring within $k$ changes:

| cell | $k$ proved impossible | a repair needs | conflicts after $+1$ |
|---|---|---|---|
| (5,6) | 16 | $\ge 17$ edge changes | 136 |
| (4,6) | 16 | $\ge 17$ | 24 |
| (3,10) | 12 | $\ge 13$ | 6 |
| (4,7) | 12 | $\ge 13$ | 40 |
| (3,11) | 10 | $\ge 11$ | 8 |
| (4,9) | 8 | $\ge 9$ | 72 |
| (6,8), (6,9), (8,10) | no rung finishes | unknown | 338 / 2 830 / 29 814 |

The bounds are for the specific best-twin extension of each base colouring; they are not statements
about $R(r,s)$ itself.

## 4. Consequences

**(a) The method cannot move these cells from these bases.** For (4,6) this is decisive: every engine
in the study stalled at 12 conflicts, and the nearest valid 36-vertex colouring is at least 17 edge
changes away. The stall was a fact about the cell, not about the heuristic — no amount of tuning,
restarts or extra seeds closes a gap of 17.

**(b) Conflict count does not rank repairability.** This is the finding most likely to be useful to
others. Across cells, (4,9) starts from 72 conflicts and needs only $\ge 9$ changes, while (3,10)
starts from 6 — the smallest in the table, and the natural first choice — and needs $\ge 13$. Within a
single cell the effect is starker: a (6,8) colouring of $K_{134}$ with a *single* red $K_6$ admits no
repair in 3 flips (exhaustive: 29 279 nodes) and none in 4 (5 169 551 nodes), while a state with
hundreds of conflicts can be closer in Hamming distance. Repair campaigns that seed by fewest
conflicts are selecting the wrong targets.

**(c) Structure, not budget, is the wall.** §3 combined with §2 shows the $+1$ step is a large jump
almost everywhere, which is consistent with the histories: Cremer's own (8,10) campaign needed ten
30-minute segments plus section crossovers to close a state that started from 19 271 conflicts. The
last few conflicts are the difficulty, and they are a distance phenomenon.

**(d) The closest known approach to $R(3,10)=41$.** $R(3,10)\in\{40,41\}$; the upper bound is
Angeltveit's. Letting the new vertex take a chosen red neighbourhood $A$ rather than copying one
vertex, with $A=\{0,3,5,14,19,22,32,35,36,37\}$ on Exoo's colouring of $K_{39}$, gives a 40-vertex
colouring with **4** red triangles and no blue $K_{10}$; a short repair reaches **2** red triangles.
Both objects are recorded. Their status is now exact: the 4-triangle state has no valid colouring
within 11 edge changes, the 2-triangle state has none within 4 flips (exhaustive) and none within 13
of the twin. A 40-vertex $(3,10)$-colouring, if one exists, is not a small perturbation of either.

## 5. Limits

* These are distances from *specific* starting states, not classifications. A colouring of the target
  order may exist far from all of them.
* For (6,8), (6,9) and (8,10) no rung finishes; those cells' negatives rest on search alone, and no
  distance statement is made.
* The (3,10) floor at 5 flips is the depth-4 exhaustion; depth 5 is undecided ($1.8\times10^{7}$
  nodes in 27 minutes without a conclusion).
* No new Ramsey number is determined here.

## 6. Reproduction

Every number above is re-derivable from the files deposited with `results/cremer-sweep.md`, whose §8
lists the engines and states. Two conventions must be respected, and both have already produced wrong
counts for the authors:

1. The six `.scratch/exoo/` matrices store `1 = blue`; the Cremer and Kuznetsov files store `1 = red`.
   Call checkers with $(R,S)$ for the cell after normalising.
2. Checkers assume a **zero diagonal**. Flipping a zero-diagonal matrix to switch colour convention
   leaves 1s on the diagonal, and both `count` and the clique enumerator then over-report by an order
   of magnitude (a state with 40 red $K_4$ was reported as 7 151).
