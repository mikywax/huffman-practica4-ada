#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

BIN=./huffman
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
fail() { echo "FAIL: $*"; exit 1; }
pass() { echo "PASS: $*"; }

size_of() { stat -c%s "$1" 2>/dev/null || stat -f%z "$1"; }

# T1: mensaje de la práctica
printf '%s' 'ABRACADABRA_ALGORITMO_AVANZADO_ESCOM' > "$TMP/t1.txt"
$BIN c "$TMP/t1.txt" "$TMP/t1.huf"
$BIN d "$TMP/t1.huf" "$TMP/t1.out"
cmp -s "$TMP/t1.txt" "$TMP/t1.out" || fail "T1 round-trip"
pass "T1 round-trip mensaje"

# T2: archivo vacío
: > "$TMP/empty.txt"
$BIN c "$TMP/empty.txt" "$TMP/empty.huf"
$BIN d "$TMP/empty.huf" "$TMP/empty.out"
cmp -s "$TMP/empty.txt" "$TMP/empty.out" || fail "T2 empty"
pass "T2 vacío"

# T3: single-symbol 10^6 'A's
head -c 1000000 /dev/zero | tr '\0' 'A' > "$TMP/aaa.txt"
$BIN c "$TMP/aaa.txt" "$TMP/aaa.huf"
$BIN d "$TMP/aaa.huf" "$TMP/aaa.out"
cmp -s "$TMP/aaa.txt" "$TMP/aaa.out" || fail "T3 single-symbol"
pass "T3 single-symbol"

# T4: binario aleatorio
head -c 65536 /dev/urandom > "$TMP/rand.bin"
$BIN c "$TMP/rand.bin" "$TMP/rand.huf"
$BIN d "$TMP/rand.huf" "$TMP/rand.out"
cmp -s "$TMP/rand.bin" "$TMP/rand.out" || fail "T4 random"
pass "T4 binario aleatorio"

# T5: Caso A sintético exacto (mensaje x 10000) => 166289 bytes
python3 -c 'import sys; sys.stdout.write("ABRACADABRA_ALGORITMO_AVANZADO_ESCOM"*10000)' > "$TMP/a10k.txt"
$BIN c "$TMP/a10k.txt" "$TMP/a10k.huf"
sz=$(size_of "$TMP/a10k.huf")
[ "$sz" -eq 166289 ] || fail "T5 tamaño esperado 166289, obtenido $sz"
$BIN d "$TMP/a10k.huf" "$TMP/a10k.out"
cmp -s "$TMP/a10k.txt" "$TMP/a10k.out" || fail "T5 round-trip"
pass "T5 Caso A sintético = 166289 bytes exactos"

# T6: Caso B sintético exacto (0x00..0xFF x 100) => 25937 bytes
python3 -c 'import sys; sys.stdout.buffer.write(bytes(range(256))*100)' > "$TMP/b.bin"
$BIN c "$TMP/b.bin" "$TMP/b.huf"
sz=$(size_of "$TMP/b.huf")
[ "$sz" -eq 25937 ] || fail "T6 tamaño esperado 25937, obtenido $sz"
$BIN d "$TMP/b.huf" "$TMP/b.out"
cmp -s "$TMP/b.bin" "$TMP/b.out" || fail "T6 round-trip"
pass "T6 Caso B sintético = 25937 bytes exactos"

# T7: Caso C sintético exacto ('A' x 10^7) => 19 bytes
head -c 10000000 /dev/zero | tr '\0' 'A' > "$TMP/c.txt"
$BIN c "$TMP/c.txt" "$TMP/c.huf"
sz=$(size_of "$TMP/c.huf")
[ "$sz" -eq 19 ] || fail "T7 tamaño esperado 19, obtenido $sz"
$BIN d "$TMP/c.huf" "$TMP/c.out"
cmp -s "$TMP/c.txt" "$TMP/c.out" || fail "T7 round-trip"
pass "T7 Caso C sintético = 19 bytes exactos"

echo "Todos los tests pasaron."