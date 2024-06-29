#!/usr/bin/env python3
# -*- coding: utf-8 -*

"""
Merge headers to single file.
"""

import os
import re
import sys
from collections import OrderedDict
from typing import List, Dict

scripts_root = os.path.abspath(os.path.dirname(__file__))
include_root = os.path.join(scripts_root, '..', 'include')


class HeaderSource(object):
    def __init__(self):
        self.includes: List[str] = []
        self.refs: List[str] = []
        self.conditional_includes: List[str] = []
        self.codes: List[str] = []


def read_header(path: str) -> HeaderSource:
    if not os.path.isabs(path):
        path = os.path.join(include_root, path)

    lines = []
    with open(path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # cut include control
    while len(lines):
        line = lines[0]
        del lines[0]
        if re.match(r'^#define .*', line):
            break

    while len(lines):
        line = lines[-1]
        del lines[-1]
        if re.match(r'^#endif .*', line):
            break

    src = HeaderSource()

    # find std include part
    cursor = 0
    while cursor < len(lines):
        line = lines[cursor]
        if not re.match(r'^(\s*|\s*#include\s+<\S+>\s*)$', line):
            break
        cursor += 1

    # store std include part
    src.includes = lines[:cursor]
    del lines[:cursor]

    # find local include part
    cursor = 0
    while cursor < len(lines):
        line = lines[cursor]
        if not re.match(r'^(\s*|\s*#include\s+"\S+"\s*)$', line):
            break
        cursor += 1

    # store local include part
    src.refs = lines[:cursor]
    del lines[:cursor]

    # find conditional include part
    cursor = 0
    found_cursor = 0
    while cursor < len(lines):
        # find #if prefix
        found = False
        while cursor < len(lines):
            line = lines[cursor]
            line = line.strip()
            if not line:
                cursor += 1
                continue
            if re.match(r'^#if', line):
                cursor += 1
                found = True
            break
        if not found:
            break
        # find #include content
        found = False
        while cursor < len(lines):
            line = lines[cursor]
            line = line.strip()
            if not line:
                cursor += 1
                continue
            if re.match(r'^#include\s+', line):
                cursor += 1
                found = True
                continue
            break
        if not found:
            break
        # find #endif suffix
        found = False
        while cursor < len(lines):
            line = lines[cursor]
            line = line.strip()
            if not line:
                cursor += 1
                continue
            if re.match(r'^#endif', line):
                cursor += 1
                found = True
            break
        if not found:
            break
        # find next
        found_cursor = cursor
    cursor = found_cursor

    # store conditional include part
    src.conditional_includes = lines[:cursor]
    del lines[:cursor]

    # store codes
    src.codes = lines

    # modify refs
    path_root = os.path.split(path)[0]

    def include_path(inc: str) -> str:
        inc = inc.strip()
        if not inc:
            return ""
        ref = re.match(r'^#include\s+"(.*?)"$', inc)[1]
        local_ref = os.path.join(path_root, ref)
        if os.path.isfile(local_ref):
            return os.path.relpath(local_ref, include_root)
        global_ref = os.path.join(include_root, ref)
        if os.path.isfile(global_ref):
            return os.path.relpath(global_ref, include_root)
        raise RuntimeError(f'can not resolve {inc} in {path}')

    local_includes = [include_path(s) for s in src.refs]
    src.refs = [s for s in local_includes if s]

    src.includes = [s for s in src.includes if len(s.strip())]

    def strip_list(ss: List[str]):
        while len(ss) and not ss[0].strip():
            del ss[0]
        while len(ss) and not ss[-1].strip():
            del ss[-1]

    strip_list(src.conditional_includes)
    strip_list(src.codes)

    return src


def list_header_files():
    prefix = 'uvcxx'
    uvcxx = os.path.join(include_root, prefix)
    dirs = ['utils', 'cxx', 'inner', '']
    headers = ['uvcxx/utils/standard.h']  # < first header should be standard
    for d in dirs:
        files = os.listdir(os.path.join(uvcxx, d))
        for file in files:
            ext = os.path.splitext(file)[-1]
            if ext.lower() != '.h':
                continue
            headers.append(os.path.join(prefix, d, file))
    return headers


def cache_read_header(cache: Dict[str, HeaderSource], path: str) -> HeaderSource:
    if path in cache:
        return cache.get(path)
    src = read_header(path)
    cache[path] = src
    return src


def store_header_sources(
        store: Dict[str, HeaderSource],
        cache: Dict[str, HeaderSource],
        path: str):
    if path in cache:
        return
    src = cache_read_header(cache, path)

    for ref in src.refs:
        if ref not in store:
            store_header_sources(store, cache, ref)

    store[path] = src


def read_license() -> str:
    path = os.path.join(scripts_root, '..', 'LICENSE')
    with open(path, 'r', encoding='utf-8') as f:
        return f.read()


def clear_multi_new_lines(ss: List[str]) -> List[str]:
    ss = list(ss)
    while len(ss) and not ss[0].strip():
        del ss[0]
    while len(ss) and not ss[-1].strip():
        del ss[-1]
    i = 0
    while i < len(ss) - 1:
        if not ss[i].strip() and not ss[i + 1].strip():
            del ss[i]
            continue
        i += 1
    return ss


def clear_empty_macro_if(ss: List[str]) -> List[str]:
    lines = list(ss)
    ss = []

    cursor = 0
    while cursor < len(lines):
        if_cursor = 0
        endif_cursor = 0
        # find #if prefix
        found = False
        while cursor < len(lines):
            line = lines[cursor]
            line = line.strip()
            if not line:
                cursor += 1
                continue
            if re.match(r'^#if', line):
                if_cursor = cursor
                cursor += 1
                found = True
            break
        if not found:
            break
        # find #endif suffix
        non_empty_line = 0
        found = False
        while cursor < len(lines):
            line = lines[cursor]
            line = line.strip()
            if re.match(r'^#endif', line):
                endif_cursor = cursor
                cursor += 1
                found = True
                break
            if line:
                non_empty_line += 1
            cursor += 1
        if not found:
            break

        if non_empty_line > 0:
            ss.extend(lines[if_cursor:endif_cursor + 1])

    return ss


def comment_license(lic: str) -> List[str]:
    content = '\n * '.join(lic.split('\n'))
    return [f'/**\n * {content}\n */\n']


sign = """
/**
 * Created by Levalup.
 * L.eval: Let programmer get rid of only work jobs.
 * See https://github.com/levalup/libuvcxx for `libuvcxx` documentation.
 * See https://github.com/libuv/libuv#documentation for `libuv` documentation.
 */
"""

def merge_header() -> str:
    macro_control = 'UVCXX_H'
    version_control_header = 'uvcxx/cxx/version.h'

    store: Dict[str, HeaderSource] = OrderedDict()
    cache: Dict[str, HeaderSource] = {}
    headers = list_header_files()
    for header in headers:
        store_header_sources(store, cache, header)

    # merge include part
    merge_includes = []
    for src in store.values():
        merge_includes.extend([s for s in src.includes if not re.match(r'#include\s+<uv\.h>', s)])
    merge_includes = list(set(merge_includes))
    merge_includes.sort()

    # merge conditional include
    merge_conditional_includes = []
    for src in store.values():
        if merge_conditional_includes and src.conditional_includes:
            merge_conditional_includes.append('\n')
        merge_conditional_includes.extend(src.conditional_includes)

    merge_conditional_includes = [s for s in merge_conditional_includes if s not in merge_includes]
    merge_conditional_includes = clear_multi_new_lines(merge_conditional_includes)
    merge_conditional_includes = clear_empty_macro_if(merge_conditional_includes)

    def append_source(codes: List[str], name: str, source: HeaderSource = None):
        if source is None:
            source = store[name]

        codes.append(f'// {"-" * 77}\n')
        codes.append(f'// #include "{name}"\n')
        codes.append(f'// {"-" * 77}\n')
        codes.append('\n')

        codes.extend(source.codes)
        codes.append('\n')

    merge_source = []
    for header, src in store.items():
        if header == version_control_header:
            continue
        append_source(merge_source, header, src)

    lines = comment_license(read_license())

    lines.append(sign)
    lines.append('\n')

    lines.append(f'#ifndef {macro_control}\n')
    lines.append(f'#define {macro_control}\n')
    lines.append('\n')

    lines.extend(merge_includes)
    lines.append('\n')

    lines.append('#include <uv.h>\n')
    lines.append('\n')

    append_source(lines, version_control_header)

    lines.append(f'// {"-" * 77}\n')
    lines.append(f'// Header files used for version compatibility.\n')
    lines.append(f'// {"-" * 77}\n')
    lines.append('\n')

    lines.extend(merge_conditional_includes)
    lines.append('\n')

    lines.extend(merge_source)

    lines.append(f'#endif // {macro_control}\n')

    return ''.join(lines)


def main():
    if len(sys.argv) > 1:
        output_merge_header = sys.argv[1]
    else:
        output_merge_header = os.path.join(include_root, 'uvcxx-single.h')
    output_merge_header = os.path.realpath(output_merge_header)

    merge_content = merge_header()

    if os.path.exists(output_merge_header):
        with open(output_merge_header, 'r', encoding='utf-8') as f:
            origin_content = f.read()
        if origin_content == merge_content:
            print(f'[INFO] The header "{output_merge_header}" is already up-to-date and will not be modified.')
            return

    with open(output_merge_header, 'w', encoding='utf-8') as f:
        f.write(merge_content)

    print(f'[INFO] The merged header file has been written to "{output_merge_header}".')


if __name__ == '__main__':
    main()
