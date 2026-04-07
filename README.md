# LZ78 Compression Tool

A Python command-line tool that compresses and decompresses any file using the LZ78 algorithm with guaranteed lossless round-trip integrity.

## Project Structure

```
plc-project/
├── lz78.py            # Core LZ78 compress/decompress functions
├── cli.py             # Command-line interface (entry point)
├── requirements.txt   # Python dependencies (pytest)
├── tests/
│   ├── test_lz78.py   # Correctness tests + image round-trip tests
│   └── images/        # 2 JPEG test images
└── README.md
```

### File Summary

- **`lz78.py`** — Implements the LZ78 algorithm. Provides `compress(data) -> bytes` and `decompress(data) -> bytes`. Builds a dictionary of seen byte patterns and encodes them as (index, next_byte) pairs in a compact binary format.
- **`cli.py`** — Command-line wrapper around `lz78.py`. Accepts `-c` (compress) or `-d` (decompress), `-i` (input file), `-o` (output file), and `-v` (verbose stats).
- **`tests/test_lz78.py`** — Pytest test suite with 5 core correctness tests (empty input, strings, null bytes, repetitive data, all byte values) and 1 image round-trip test that compresses and decompresses both JPEG images and verifies zero data loss.

## How to Run

### Install dependencies

```bash
pip install -r requirements.txt
```

### Compress a file

```bash
python3 cli.py -c -i <input_file> -o <output_file>
```

### Decompress a file

```bash
python3 cli.py -d -i <input_file.lz78> -o <output_file>
```

### Examples

```bash
# Compress an image (output auto-named as .lz78)
python3 cli.py -c -i tests/images/1675398821215.jpeg -v

# Decompress it back
python3 cli.py -d -i tests/images/1675398821215.jpeg.lz78 -o restored.jpeg -v

# Compress a text file
python3 cli.py -c -i myfile.txt -o myfile.txt.lz78 -v
```

### Run tests

```bash
python3 -m pytest tests/test_lz78.py -v
```

## CLI Options

| Flag | Description |
|------|-------------|
| `-c` | Compress mode |
| `-d` | Decompress mode |
| `-i <file>` | Input file path (required) |
| `-o <file>` | Output file path (auto-generated if omitted) |
| `-v` | Verbose — print size, ratio, time stats |

## Notes

- Compression is **lossless**: `decompress(compress(file)) == original file` for any input.
- JPEG/PNG files are already compressed, so LZ78 output will be *larger* than the original. LZ78 works best on uncompressed data like plain text or raw binary.
