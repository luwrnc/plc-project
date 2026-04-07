#!/usr/bin/env python3
"""
LZ78 Compression CLI Tool

A command-line interface for LZ78 compression and decompression.
Usage: python cli.py [-c|-d] [-i input] [-o output] [-v]
"""

import argparse
import os
import sys
import time
from pathlib import Path
from lz78 import compress, decompress, compress_ratio


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="LZ78 compression and decompression tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s -c -i file.txt -o file.txt.lz78    # Compress file
  %(prog)s -d -i file.txt.lz78 -o file.txt   # Decompress file
  %(prog)s -c -i file.txt -v                 # Compress with verbose output
        """
    )
    
    mode_group = parser.add_mutually_exclusive_group(required=True)
    mode_group.add_argument('-c', '--compress', action='store_true',
                           help='Compress the input file')
    mode_group.add_argument('-d', '--decompress', action='store_true',
                           help='Decompress the input file')
    
    parser.add_argument('-i', '--input', type=str, required=True,
                       help='Input file path')
    parser.add_argument('-o', '--output', type=str,
                       help='Output file path (default: auto-generated)')
    parser.add_argument('-v', '--verbose', action='store_true',
                       help='Verbose output with statistics')
    
    return parser.parse_args()


def get_output_path(input_path: str, mode: str, output_arg: str = None) -> str:
    """Generate output file path based on mode and input."""
    if output_arg:
        return output_arg
    
    input_path_obj = Path(input_path)
    
    if mode == 'compress':
        return str(input_path_obj.with_suffix(input_path_obj.suffix + '.lz78'))
    elif mode == 'decompress':
        if input_path_obj.suffix == '.lz78':
            return str(input_path_obj.with_suffix(''))
        else:
            return str(input_path_obj.with_suffix('.decompressed'))
    
    return input_path


def read_file(filepath: str) -> bytes:
    """Read file contents as bytes."""
    try:
        with open(filepath, 'rb') as f:
            return f.read()
    except FileNotFoundError:
        print(f"Error: Input file '{filepath}' not found", file=sys.stderr)
        sys.exit(1)
    except PermissionError:
        print(f"Error: Permission denied reading '{filepath}'", file=sys.stderr)
        sys.exit(1)


def write_file(filepath: str, data: bytes) -> None:
    """Write data to file."""
    try:
        with open(filepath, 'wb') as f:
            f.write(data)
    except PermissionError:
        print(f"Error: Permission denied writing to '{filepath}'", file=sys.stderr)
        sys.exit(1)


def print_stats(original_size: int, compressed_size: int, 
                processing_time: float, input_path: str, output_path: str) -> None:
    """Print compression/decompression statistics."""
    ratio = compressed_size / original_size if original_size > 0 else 1.0
    savings = (1 - ratio) * 100
    
    print(f"Input file:  {input_path}")
    print(f"Output file: {output_path}")
    print(f"Original size:    {original_size:,} bytes")
    print(f"Compressed size: {compressed_size:,} bytes")
    print(f"Compression ratio: {ratio:.3f}")
    print(f"Space savings:    {savings:.1f}%")
    print(f"Processing time:  {processing_time:.3f} seconds")


def main():
    """Main CLI entry point."""
    args = parse_args()
    
    # Validate input file exists
    if not os.path.exists(args.input):
        print(f"Error: Input file '{args.input}' does not exist", file=sys.stderr)
        sys.exit(1)
    
    # Determine output path
    mode = 'compress' if args.compress else 'decompress'
    output_path = get_output_path(args.input, mode, args.output)
    
    # Read input file
    start_time = time.time()
    input_data = read_file(args.input)
    read_time = time.time()
    
    # Process data
    if args.compress:
        output_data = compress(input_data)
    else:
        try:
            output_data = decompress(input_data)
        except ValueError as e:
            print(f"Decompression error: {e}", file=sys.stderr)
            sys.exit(1)
    
    process_time = time.time()
    
    # Write output file
    write_file(output_path, output_data)
    write_time = time.time()
    
    total_time = write_time - start_time
    
    # Print verbose stats if requested
    if args.verbose:
        if args.compress:
            print_stats(
                len(input_data), len(output_data), total_time,
                args.input, output_path
            )
        else:
            print(f"Input file:  {args.input}")
            print(f"Output file: {output_path}")
            print(f"Compressed size: {len(input_data):,} bytes")
            print(f"Decompressed size: {len(output_data):,} bytes")
            print(f"Processing time:   {total_time:.3f} seconds")
            
            # Verify round-trip integrity
            if len(input_data) > 0:
                try:
                    verify_compressed = compress(output_data)
                    if input_data == verify_compressed:
                        print("Round-trip verification: PASSED")
                    else:
                        print("Round-trip verification: FAILED")
                except Exception:
                    print("Round-trip verification: ERROR")
    else:
        # Simple confirmation
        print(f"Successfully {mode[:-1]}ed '{args.input}' to '{output_path}'")


if __name__ == '__main__':
    main()
