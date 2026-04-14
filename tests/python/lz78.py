#!/usr/bin/env python3
"""
LZ78 compression algorithm implementation.

This module provides compress() and decompress() functions for the LZ78 algorithm.
The output format includes a magic header and uses binary encoding for efficiency.
"""

import struct
from typing import List, Tuple, Dict

# Magic header: "LZ78" + version byte (v2 = fixed null-byte handling)
MAGIC_HEADER = b'LZ78\x02'
MAX_DICT_SIZE = 2**20  # ~1M entries


def compress(data: bytes) -> bytes:
    """
    Compress data using LZ78 algorithm.
    
    Binary format:
      MAGIC_HEADER (5 bytes)
      num_full_entries  (4 bytes, little-endian uint32)
      has_tail          (1 byte, 0 or 1)
      full entries      (each: index 4 bytes + byte 1 byte = 5 bytes)
      tail entry        (if has_tail: index 4 bytes)
    
    Args:
        data: Input bytes to compress
        
    Returns:
        Compressed bytes with magic header
    """
    if not data:
        return MAGIC_HEADER + struct.pack('<IB', 0, 0)
    
    dictionary: Dict[bytes, int] = {}
    # Index 0 is reserved for "empty prefix"
    # Indices 1..256 are single bytes 0x00..0xFF
    dict_size = 257
    for i in range(256):
        dictionary[bytes([i])] = i + 1  # 1-indexed
    
    w = b''
    full_entries: List[Tuple[int, int]] = []  # (parent_index, next_byte)
    
    for b in data:
        wc = w + bytes([b])
        if wc in dictionary:
            w = wc
        else:
            if dict_size < MAX_DICT_SIZE:
                dictionary[wc] = dict_size
                dict_size += 1
            w_index = dictionary[w] if w else 0
            full_entries.append((w_index, b))
            w = b''
    
    # Tail: leftover w that is already in dictionary
    has_tail = 1 if w else 0
    tail_index = dictionary[w] if w else 0
    
    # Encode
    out = bytearray(MAGIC_HEADER)
    out += struct.pack('<IB', len(full_entries), has_tail)
    for idx, byte in full_entries:
        out += struct.pack('<IB', idx, byte)
    if has_tail:
        out += struct.pack('<I', tail_index)
    
    return bytes(out)


def decompress(data: bytes) -> bytes:
    """
    Decompress LZ78 compressed data.
    
    Args:
        data: Compressed bytes with magic header
        
    Returns:
        Original uncompressed bytes
    """
    if not data or not data.startswith(MAGIC_HEADER):
        raise ValueError("Invalid LZ78 compressed data")
    
    offset = len(MAGIC_HEADER)
    
    if offset + 5 > len(data):
        raise ValueError("Corrupted compressed data")
    
    num_entries, has_tail = struct.unpack('<IB', data[offset:offset+5])
    offset += 5
    
    if num_entries == 0 and has_tail == 0:
        return b''
    
    # Rebuild dictionary: 0 = empty, 1..256 = single bytes
    dictionary: Dict[int, bytes] = {0: b''}
    dict_size = 257
    for i in range(256):
        dictionary[i + 1] = bytes([i])
    
    chunks: List[bytes] = []
    
    for _ in range(num_entries):
        if offset + 5 > len(data):
            raise ValueError("Corrupted compressed data")
        
        idx, byte = struct.unpack('<IB', data[offset:offset+5])
        offset += 5
        
        if idx not in dictionary:
            raise ValueError(f"Invalid dictionary index: {idx}")
        
        entry = dictionary[idx] + bytes([byte])
        chunks.append(entry)
        
        if dict_size < MAX_DICT_SIZE:
            dictionary[dict_size] = entry
            dict_size += 1
    
    if has_tail:
        if offset + 4 > len(data):
            raise ValueError("Corrupted compressed data")
        idx = struct.unpack('<I', data[offset:offset+4])[0]
        if idx not in dictionary:
            raise ValueError(f"Invalid dictionary index: {idx}")
        chunks.append(dictionary[idx])
    
    return b''.join(chunks)


def compress_ratio(original: bytes, compressed: bytes) -> float:
    """Calculate compression ratio (compressed/original)."""
    if not original:
        return 1.0
    return len(compressed) / len(original)


if __name__ == '__main__':
    # Simple test
    test_data = b"aaaaaaaaaaaaaaaaaaaaaaaaaaaaforehuigrehfbuifguifuhqbjifebuwheiuwf3qvr3ubracadabra"
    compressed = compress(test_data)
    decompressed = decompress(compressed)
    
    print(f"Original: {test_data}")
    print(f"Compressed: {len(compressed)} bytes")
    print(f"Decompressed: {decompressed}")
    print(f"Match: {test_data == decompressed}")
    print(f"Ratio: {compress_ratio(test_data, compressed):.3f}")
