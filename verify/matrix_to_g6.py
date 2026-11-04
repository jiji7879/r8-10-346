#!/usr/bin/env python3
"""Convert a 0/1 symmetric adjacency matrix (one row per line, no separators) to graph6.

Also verifies the round trip by decoding its own output, so a bug here cannot silently
produce a wrong .g6 file. Standard library only.

Usage:
    ./matrix_to_g6.py [--header] MATRIX [OUT.g6]

If OUT is omitted, the graph6 string is printed to stdout.  By default the output is the
bare encoding, as written by nauty's tools; --header prepends the optional ">>graph6<<"
file header, which some readers expect.
"""
import sys

MAGIC = b">>graph6<<"


def encode_n(n):
    """graph6 header for the vertex count."""
    if n <= 62:
        return bytes([n + 63])
    if n <= 258047:
        return b"~" + bytes([((n >> 12) & 63) + 63,
                             ((n >> 6) & 63) + 63,
                             (n & 63) + 63])
    raise ValueError("graph6 supports n <= 258047")


def encode_g6(n, adj, header=False):
    out = bytearray(MAGIC if header else b"")
    out += encode_n(n)
    acc = 0
    bits = 0
    for j in range(1, n):            # upper triangle, column-major by row
        for i in range(j):
            acc = (acc << 1) | (1 if adj[j][i] else 0)
            bits += 1
            if bits == 6:
                out.append(acc + 63)
                acc = 0
                bits = 0
    if bits:
        out.append((acc << (6 - bits)) + 63)
    return bytes(out)


def decode_g6(data):
    """Independent decoder, used only to check the encoder."""
    p = len(MAGIC) if data.startswith(MAGIC) else 0
    b0 = data[p]
    if b0 <= 62:
        n = b0
        p += 1
    elif b0 == 126:
        n = (data[p + 1] - 63) << 12 | (data[p + 2] - 63) << 6 | (data[p + 3] - 63)
        p += 4
    else:
        raise ValueError("unsupported graph6 header")
    adj = [[0] * n for _ in range(n)]
    acc = 0
    bits = 0
    it = iter(data[p:])
    for j in range(1, n):
        for i in range(j):
            if bits == 0:
                acc = next(it) - 63
                bits = 6
            bits -= 1
            v = (acc >> bits) & 1
            adj[j][i] = adj[i][j] = v
    return n, adj


def load_matrix(path):
    rows = [ln.strip() for ln in open(path) if ln.strip()]
    n = len(rows)
    if any(len(r) != n for r in rows):
        raise ValueError("matrix is not square")
    if any(c not in "01" for r in rows for c in r):
        raise ValueError("matrix must contain only 0 and 1")
    adj = [[1 if rows[i][j] == '1' else 0 for j in range(n)] for i in range(n)]
    if any(adj[i][j] != adj[j][i] for i in range(n) for j in range(n)):
        raise ValueError("matrix is not symmetric")
    if any(adj[i][i] for i in range(n)):
        raise ValueError("diagonal must be zero")
    return n, adj


def main():
    args = sys.argv[1:]
    header = "--header" in args
    args = [a for a in args if a != "--header"]
    if not args:
        sys.exit(__doc__)
    n, adj = load_matrix(args[0])
    g6 = encode_g6(n, adj, header=header)

    # self-check: decode and compare edge by edge
    n2, adj2 = decode_g6(g6)
    if n2 != n or adj2 != adj:
        sys.exit("internal error: graph6 round trip failed")

    edges = sum(sum(r) for r in adj) // 2
    sys.stderr.write("n=%d edges=%d bytes=%d round trip OK\n" % (n, edges, len(g6)))

    if len(args) > 1:
        with open(args[1], "wb") as f:
            f.write(g6 + b"\n")
        sys.stderr.write("wrote %s\n" % args[1])
    else:
        sys.stdout.write(g6.decode() + "\n")


if __name__ == "__main__":
    main()
