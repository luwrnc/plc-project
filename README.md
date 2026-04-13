# LZ78 (C Program)

This repository is centered on a C implementation of LZ78 compression and decompression.

## Project Layout

```
plc-project/
├── lz78.c
├── lz78.h
├── main.c
├── test.c
├── Makefile
└── tests/
    ├── images/
    │   ├── 1675398821215.jpeg
    │   └── images_2.jpeg
    ├── test_lz78.py
    └── python/
        ├── __init__.py
        ├── lz78.py
        ├── cli.py
        └── requirements.txt
```

Notes:
- Top-level build/run flow is C-only.
- Python files are kept under `tests/python/` as test/reference utilities.

## Build

```bash
make build
```

This builds:
- `lz78_main` (CLI wrapper from `main.c`)
- `lz78_test` (round-trip test from `test.c`)

## Run C CLI

```bash
./lz78_main -c <input_file> <output_file>
./lz78_main -d <input_file> <output_file>
```

## Run C Round-Trip Test

```bash
make test
```

Generated test artifacts are written to `tests/artifacts/`.

## Clean

```bash
make clean
```

This removes binaries, object files, and generated artifacts.
