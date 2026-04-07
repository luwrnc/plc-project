#!/usr/bin/env python3
"""
Tests for LZ78 compression algorithm.

Core correctness tests and image round-trip tests.
"""

import os
import pytest
from lz78 import compress, decompress, compress_ratio

IMAGES_DIR = os.path.join(os.path.dirname(__file__), "images")


class TestLZ78Core:
    """Essential correctness tests for LZ78."""

    def test_empty_input(self):
        data = b''
        assert decompress(compress(data)) == data

    def test_simple_string(self):
        data = b'Hello, World!'
        assert decompress(compress(data)) == data

    def test_null_bytes(self):
        data = b'\x00\x01\x02\x00\x01\x02\x00'
        assert decompress(compress(data)) == data

    def test_repetitive_data(self):
        data = b'a' * 1000
        compressed = compress(data)
        assert decompress(compressed) == data
        assert compress_ratio(data, compressed) < 0.5

    def test_all_byte_values(self):
        data = bytes(range(256))
        assert decompress(compress(data)) == data


class TestImageRoundTrip:
    """Round-trip compression tests on the 2 JPEG images."""

    def get_image_paths(self):
        images = []
        for f in sorted(os.listdir(IMAGES_DIR)):
            if f.lower().endswith(('.jpeg', '.jpg', '.png')):
                images.append(os.path.join(IMAGES_DIR, f))
        return images

    def test_image_round_trip(self):
        images = self.get_image_paths()
        assert len(images) >= 2, f"Expected at least 2 images in {IMAGES_DIR}"

        for img_path in images:
            with open(img_path, 'rb') as f:
                original = f.read()

            compressed = compress(original)
            decompressed = decompress(compressed)

            assert decompressed == original, f"Round-trip failed for {os.path.basename(img_path)}"

            ratio = compress_ratio(original, compressed)
            print(f"\n{os.path.basename(img_path)}: "
                  f"{len(original):,} → {len(compressed):,} bytes "
                  f"(ratio: {ratio:.3f})")


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
