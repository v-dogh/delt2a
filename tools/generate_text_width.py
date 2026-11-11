#!/usr/bin/env python3
"""Generate the small width tables from Unicode 16.0.0 UCD files."""
import pathlib
import sys


def ranges(path, properties):
    result = []
    for line in pathlib.Path(path).read_text().splitlines():
        fields = line.split('#')[0].strip().split(';')
        if len(fields) < 2 or fields[1].strip() not in properties:
            continue
        ends = fields[0].strip().split('..')
        result.append((int(ends[0], 16), int(ends[-1], 16)))
    merged = []
    for begin, end in sorted(result):
        if merged and begin <= merged[-1][1] + 1:
            merged[-1] = (merged[-1][0], max(end, merged[-1][1]))
        else:
            merged.append((begin, end))
    return merged


eaw, emoji = sys.argv[1:]
print('// Generated from Unicode 16.0.0 EastAsianWidth.txt and emoji/emoji-data.txt.')
print('// Copyright 2024 Unicode, Inc. Unicode and the Unicode Logo are registered trademarks of Unicode, Inc.')
print('// See UNICODE-LICENSE.txt. Regenerate with tools/generate_text_width.py.')
print('#pragma once\n#include <array>\n#include <utility>\n\nnamespace d2::text::unicode\n{')
for name, path, properties in [('wide', eaw, {'W', 'F'}),
                               ('emoji', emoji, {'Emoji'}),
                               ('emoji_presentation', emoji, {'Emoji_Presentation'})]:
    data = ranges(path, properties)
    print(f'    inline constexpr std::array<std::pair<char32_t, char32_t>, {len(data)}> {name}{{{{')
    for begin, end in data:
        print(f'        {{0x{begin:x}, 0x{end:x}}},')
    print('    }};')
print('}')
