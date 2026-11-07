# Huffman con BitWriter/BitReader

Compresor Huffman en C++17 con:
- BitWriter / BitReader (empaquetado real de bits).
- Serialización de árbol en preorden (0 = interno, 1 + 8 bits = hoja).
- Header con `original_size`, `tree_bit_len` y `valid_last` (anti-padding-trap).
- Manejo correcto de archivo vacío y alfabeto de un solo símbolo.

## Compilar

    make

## Uso

    ./huffman-practica4-ada/huffman c <entrada>    <salida.huf>
    ./huffman-practica4-ada/huffman d <entrada.huf> <salida>

## Tests

    make test

## Formato del archivo .huf

| Campo | Tipo | Bytes |
|---|---|---:|
| magic "HUF1" | char[4] | 4 |
| original_size | uint64 LE | 8 |
| tree_bit_len | uint32 LE | 4 |
| valid_last (0 = sin payload) | uint8 | 1 |
| árbol (bits empaquetados) | bytes | ceil(tree_bit_len/8) |
| payload | bytes | variable |

## Benchmarks teóricos exactos

| Caso | Entrada | Orig. | Comprimido | Ratio |
|---|---|---:|---:|---:|
| A | mensaje × 10 000 | 360 000 | 166 289 | 46.1913889 % |
| B | 0x00..0xFF × 100 | 25 600 | 25 937 | 101.31640625 % |
| C | 'A' × 10 000 000 | 10 000 000 | 19 | 0.00019 % |