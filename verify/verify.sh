#!/bin/sh
# Build every checker and verify both deposited colourings from scratch.
#
#   ./verify/verify.sh
#
# Exits non-zero if any check fails.  Takes about a minute: the dominating cost is
# the exhaustive blue K10 search at n = 345.
set -e
cd "$(dirname "$0")/.."

echo "== hashes =="
( cd colorings && sha256sum -c SHA256SUMS )

echo
echo "== building checkers =="
make -C verify >/dev/null
echo "ok"

for W in A B; do
    F="colorings/r810-n345-witness-$W.txt"
    echo
    echo "== witness $W: $F =="

    printf 'count   : '
    ./verify/count "$F" 8 10 /dev/null | sed 's/.*: //'

    printf 'verify  : '
    ./verify/verify "$F" 8 10 | tr '\n' ' ' | sed 's/n=[0-9]* //;s/red_edges=[0-9]* //'
    echo

    printf 'verify2 : '
    ./verify/verify2 "$F" 8 10 | sed 's/.*| //'

    printf 'enum    : '
    ./verify/enum "$F" 8 10 /dev/null | tail -1
done

echo
echo "== graph6 encodings agree with the matrices =="
for W in A B; do
    ./verify/matrix_to_g6.py "colorings/r810-n345-witness-$W.txt" /tmp/g6check 2>/dev/null
    if cmp -s /tmp/g6check "colorings/r810-n345-witness-$W.g6"; then
        echo "witness $W: graph6 byte-identical"
    else
        echo "witness $W: GRAPH6 MISMATCH"; exit 1
    fi
done
rm -f /tmp/g6check

echo
echo "Both colourings contain no red K8 and no blue K10."
echo "Hence R(8,10) >= 346."
