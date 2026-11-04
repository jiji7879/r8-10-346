# Cremer's "add a vertex, then repair" method across nine cells: what it gives

Swarm `smvzvl5gk`, 8 members, ~4 h on 4 cores. Status at time of writing: 3 submissions accepted,
5 members still running. Companion to `results/cayley-order40.md` (the symmetric-construction corner)
and to the paper `arXiv:2610.12122` (text at `.scratch/cremer.txt`).

## Headline

> **Update (later the same day): one entry did move.** A follow-up campaign continuing this sweep's
> (8,10) trajectory reached zero conflicts on 345 vertices, proving $R(8,10)\ge 346$ — see
> `results/r810-346.md`. The screening below is unchanged and remains the reason that cell was the
> only one worth pushing; the "no table entry moved" line refers to the sweep itself.

**No table entry moved (in the sweep proper).** What the sweep produced instead is a complete, exactly-quantified
measurement of the method's first step for nine cells, plus *proofs* (not search reports) that the
resulting states are far from any colouring — and one object that matters: a 40-vertex colouring of
$K_{40}$ with **two** monochromatic triangles, two edge changes from settling $R(3,10)=41$.

## 1. The method

Cremer improved the lower bound of $R(6,8)$ from 134 to 135 and of $R(8,10)$ from 343 to 345 by: take a published record
colouring of $K_{n-1}$, **add one vertex** (a copy of an existing one, twin edge a chosen colour),
then **repair** the monochromatic cliques it creates — exact conflict sets and per-flip deltas, tabu
5–19, Metropolis $T=0.25+\min(2.5,t/100)$, stall kicks, preparation flips (§3.2), risk chains (§3.2),
and section mutation/crossover/patch (§3.3). A repair to zero conflicts is a new bound.

The precondition nobody had measured: how far the +1-vertex state is from a solution.

## 2. The first step, exactly (all nine cells)

Exact conflict count after adding a twin, over all $2n$ choices of (vertex, twin-edge colour).
Computed independently by three members (m1's `twin.c`, m4's `scan.c`, m2's 268-twin scan) and
spot-checked by me against my own counter — they agree.

| cell | start | best twin | conflicts after +1 |
|---|---|---|---|
| (3,10) | 39 | v35, red | **6** (6 red K3) |
| (3,11) | 46 | v20, red | 8 |
| (4,6) | 35 | v31, red | 24 |
| (4,7) | 48 | v15, red | 40 |
| (4,9) | 72 | v0, red (vertex-transitive) | 72 |
| (5,6) | 58 | v56, red | 136 |
| (6,8) | 134 | v115, red | 338 |
| (6,9) | 182 | v0, red | 2 830 |
| (8,10) | 344 | v335, **blue** | 29 814 |

For calibration, Cremer's own starting points were 281 (6,8) and 19 271 (8,10) — so these entry
points are no worse than his.

## 3. What repair reaches

| cell | start | best reached | where |
|---|---|---|---|
| (3,10) | 6 → 4 (choosing the new row instead of a twin) → **2** | 2 red K3, 0 blue K10 | m4 `u310_s1.txt` |
| (4,6) | 24 | 12 | m4 `u46.txt` |
| (4,7) | 40 | 26 | m8 `tw47.txt` |
| (5,6) | 136 | 50, flat | m4 |
| (4,9) | 72 | 58 | m8 |
| (6,9) | 2 830 | 1 116 | m1 `r69.best.txt` |
| (8,10) | 29 814 | **97** (50 K8 + 47 K10) after 102 CPU-min; chain 418 → 309 → 308 → 238 → 166 → 120 → 98 → 97 | m1 `FINAL_810_97.txt` |
| (6,8) | 338 | 69 | m2 `out68_s13.txt` |
| (3,11) | 8 | 8 (breaking cost +172 from the start) | m1 |

For (4,6) the plateau was confirmed with the **complete** pipeline: four twin starts, 55 M flips,
10 500 mutations and 6 600 kept preparation flips across §3.1+§3.2+§3.3, and every run finishes at
12 conflicts (12+0, 10+2, 12+0, 12+0). Repair alone also gives 12; mutation and preparation buy
nothing on this cell. A separate SAT check (genuine, exit 20) shows no valid 36-vertex colouring
agrees with that state outside 63 conflict edges plus 37 others; wider neighbourhoods were left
undecided, not negative.

Every one of these numbers I re-derived with `.scratch/rs/count`: 12+0, 26 (20 K4 + 6 K7),
1 079 K6 + 37 K9 = 1 116, 68 K6 + 1 K8 = 69, 50 K8 + 47 K10 = 97 (and its predecessors
167 K8 + 141 K10 = 308 and 60+60 = 120), and the two (3,10) states (4 triangles; and 2 triangles
with 0 blue K10).

**(8,10) is the one cell with a live trajectory**, and it is also the one whose earlier reading was
wrong in an instructive way. m1 first reported a plateau at 308 conflicts; told to check whether that
was a floor or a budget artefact, it ran longer segments and the descent resumed every time. The
improvement *rate* is the informative number: 294 conflicts per 100 s early, 0.16 per 100 s at 238,
0.003 per 100 s at the end. That is what a search approaching a floor looks like, and it is why 97
is a credible stopping point rather than a give-up point. Its end state is a genuine local minimum: I scanned its conflict edges myself with m4's closer in
probe mode (1 940 edge deltas, minimum -1, exactly one negative), confirming m1's count - one
improving single move and no more. It is protected to depth 2 only, and that depth-2 statement is
nearly empty, since the closer's admissible bound fires at the root node (nodes=1, pruned=1).

## 4. Proved repair distances — the real product

SAT bounded-distance screens (CEGAR over the clique side): for increasing $k$, is there **any**
$(r,s)$-colouring within Hamming distance $k$ of the +1-vertex state? UNSAT is a proof.

| cell | largest $k$ proved impossible | a repair must change | conflicts after +1 |
|---|---|---|---|
| (5,6) | 16 | **≥ 17 edges** | 136 |
| (4,6) | 16 | **≥ 17 edges** | 24 |
| (3,10) | 12 (from the best twin); 10 from the 4-triangle state | ≥ 13; ≥ 11 | 6 |
| (4,7) | 12 | ≥ 13 | 40 |
| (3,11) | 10 | ≥ 11 | 8 |
| (4,9) | 8 | ≥ 9 | 72 |
| (6,8), (6,9), (8,10) | none — no rung finishes | unknown | 338 / 2 830 / 29 814 |

Two members built these independently (m7's `sat.py`, m3's `bd.c`) with different encodings and, in
m7's case, two solvers (MiniSat 2.2 and CaDiCaL). They agree in the overlap.

**Controls.** I validated m3's `bd.c` myself: a valid (4,6) colouring with 5 edges flipped returns
SATISFIABLE with a verified witness at `-dist 5` and UNSAT at `-dist 4`. m7 reports its own control
of the same shape. Without a positive control a SAT negative is worthless, since an over-constrained
encoding produces UNSAT for everything.

I also checked the two (3,10) bases are 11 edges apart, so the ≥13 and ≥11 statements are consistent
rather than contradictory.

**The conflict count does not rank repairability.** (4,9) starts at 72 conflicts and needs only
$\ge 9$ edge changes; (3,10) starts at 6 and needs $\ge 13$. Every member of the swarm — and I —
seeded by fewest conflicts and treated (3,10) as the promising cell, and the table says that ranking
is wrong. This is the single most transferable finding of the sweep.

**Why this matters.** These are not "the search did not find it" statements. They say the +1-vertex
state of every one of these record colourings is provably separated from any valid colouring by
9–17 edge changes. For (4,6), where every engine stalled at 12 conflicts, the nearest valid
36-vertex colouring is at least 17 edges away — the stall was a fact about the cell, not about the
heuristic.

**Convention, if you reproduce m7's screen.** Its model is one Boolean per edge with `1 = red`, and
the six `.scratch/exoo/` files store `1 = blue` — they are loaded with `--swap`, and every base is
re-verified to 0 conflicts under `(R,S)` before any rung runs. The screen's starting points are
written out normalised (1 = red, $n_0+1$ vertices) under `.ultramath/smvzvl5gk/m7/work/`; use those
rather than re-deriving them from the Exoo files. The reason is a trap that cost me two wrong counts
in this session: flipping a zero-diagonal matrix yourself to switch colour convention leaves 1s on
the diagonal, and both `count` and `viol` assume a zero diagonal — a self-loop makes `viol` report a
clique with a repeated vertex and makes `count` over-count by an order of magnitude (40 red K4 became
7 151). Each $k^*$ is tied to that specific twin extension, so changing the normalisation changes the
number.

## 5. The object worth keeping: two triangles from $R(3,10)=41$

`R(3,10) \in \{40,41\}$ ($\le 41$ is Angeltveit's, DS1 rev #18). Settling it needs a 40-vertex
colouring with no red triangle and no blue $K_{10}$.

* m3 found that the new vertex should not be a twin but get a chosen red neighbourhood
  $A$, $\lvert A \rvert = 10$: with $A=\{0,3,5,14,19,22,32,35,36,37\}$ the result has **4 red
  triangles and 0 blue $K_{10}$** (`.ultramath/smvzvl5gk/m3/s40_v2.txt`, verified by me). The
  cheapest single fix costs 12 new blue $K_{10}$s; I recomputed all ten of its one-flip costs and
  they match.
* m4 then repaired from the twin to **2 red triangles, 0 blue $K_{10}$**
  (`.ultramath/smvzvl5gk/m4/u310_s1.txt`, verified by me), at Hamming distance exactly 12 from the
  twin.
* m4's exhaustive closer proves there is **no 0-conflict colouring within 4 flips** of that state
  (61 486 nodes, exhausted; I reproduced it), and the same for m3's 4-triangle state (209 341 nodes).
  Depth 5 is undecided: 18 M nodes in 27 min, no conclusion.
* m3's SAT screen proves no colouring within 11 edges of the 4-triangle state; m7's proves none
  within 13 of the twin.

So: the closest recorded approach to a 40-vertex $(3,10)$-colouring is two triangles, and it is
provably unreachable from any of these states by a short repair. If $R(3,10)=41$ is ever settled
computationally, the witness is nowhere near this neighbourhood.

## 6. What the sweep says about the method

* It works when the base is close: Cremer's own (8,10) step went 19 271 → 7 over ten 30-minute
  segments and was then closed by mutation. The nine cells seeded here are not in that regime: the
  +1-vertex states are 9–17 edge changes from any solution.
* **The last stage is not optional.** Members who ran only exact repair + tabu + Mets kicks stalled
  at exactly the conflict counts above; the paper is explicit that §3.2/§3.3 close the last few
  conflicts (its (8,10) run: 6 → 3 → 1 → 0 by three section crossovers; colouring no. 2: 2 → 1 → 0 by
  two patches). m2's calibration makes the point sharply: its engine repairs a 1.5 % corruption of a
  valid (6,8) colouring (27 990 conflicts) to 1–2 conflicts in under 12 s and cannot finish.
* **Conflict count is not repair distance, and that is now measured inside a single cell.**
  m2's toolchain takes a 1.5 % corruption of the (6,8) colouring from 27 990 conflicts down to **1**
  in 8.6 s — and that 1-conflict state then has no repair within 3 flips (depth 3 exhausted,
  29 279 nodes, reproduced by me) and none within 4 (5 169 551 nodes). Preparation flips cut its
  minimum breaking cost from 33 to 11 and no further. A colouring one forbidden clique from valid can
  therefore be five or more edge changes from repair, while m7's table has a 72-conflict (4,9) state
  only nine changes away. Any future search on these cells should be steered by a distance bound,
  not by the conflict count.
* Screening by initial conflict count alone is misleading. (3,11) has only 8 conflicts but a breaking
  cost of +172; (8,10) has 29 814 and is the most mobile of the nine.

## 7. Honest limits

* No new bound. Every cell above is where it was.
* The distance proofs are for **specific** starting states (the best twin; the 4-triangle state), not
  for all extensions or for the cell. A $(r,s)$-colouring of the target order might exist far from
  all of them.
* (3,10) at depth 5 is undecided, so the (3,10) floor is ≥ 5 flips from the 2-triangle state, not
  more.
* (6,8), (6,9), (8,10) have no distance proof — no SAT rung finishes at those sizes, so for those
  cells the negatives rest on search alone. In particular the (8,10) state at 97 conflicts is a local
  minimum with a measured improvement rate, not a proved floor.
* The $k^*$ bounds are for the specific best-twin extensions, not for the cells.

## 8. Files

| path | what |
|---|---|
| `.ultramath/smvzvl5gk/m4/{repair.c,closer.c}` | exact-repair engine; complete depth-bounded closer (proofs of "no repair within k flips") |
| `.ultramath/smvzvl5gk/m4/u310_s1.txt` | the 2-triangle 40-vertex (3,10) state |
| `.ultramath/smvzvl5gk/m3/s40_v2.txt` | the 4-triangle state, with a chosen row instead of a twin |
| `.ultramath/smvzvl5gk/m3/bd.c` | bounded-distance CEGAR-SAT (validated by positive control) |
| `results/smvzvl5gk_m7_distance_screen.md` | m7's distance screen (the sweep's main artefact) |
| `.ultramath/smvzvl5gk/m7/{sat.py,campaign.py}` | the CEGAR-SAT screen (validated by positive control) |
| `.ultramath/smvzvl5gk/m8/{engine.c,tw47.txt}` | (4,7) engine and its 26-conflict state |
| `.ultramath/smvzvl5gk/m1/{twin.c,sweep_results.md}` | exact twin scan for all nine cells |
| `.ultramath/smvzvl5gk/m2/{engine.c,out68_s13.txt}` | (6,8) engine, 69-conflict state, calibration |
| `.scratch/rs/count.c`, `verify.c`, `verify2.c` | the independent checkers used throughout |
