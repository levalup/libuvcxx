#!/usr/bin/env python3
# -*- coding: utf-8 -*

"""
Check if the api of `libuv` are mentioned in `libuvcxx`.
"""

import os
import re
import json
import glob
import urllib.request
from typing import List, Tuple, Dict
from collections import OrderedDict


def html_get(url: str) -> str:
    request = urllib.request.Request(url, method='GET')
    response = urllib.request.urlopen(request)
    content = (response.read()).decode('utf-8')
    return content


def get_libuv_toc(url: str) -> List[str]:
    content = html_get(url)

    sections = re.findall(
        r'<section id="api-documentation">(.*?)</section>',
        content,
        re.M | re.S)
    assert len(sections) > 0

    section = sections[0]

    toc = re.findall(
        r'<li class="toctree-l1"><a class="reference internal" href="(.*?)">.*?</a></li>',
        section)

    parent = os.path.dirname(url)

    return [f'{parent}/{s}' for s in toc]


def get_libuv_functions(url: str) -> Tuple[str, List[str]]:
    content = html_get(url)

    h1 = re.findall(r'<h1>(.*?)</h1>', content)[0]
    h1 = re.sub(r'<a [^<]*?>#</a>', '', h1)
    h1 = h1.replace('—', '-')
    h1 = re.sub(r'<.*?>', '', h1)
    name = h1

    sections = re.findall(
        r'<section id="(?:.*?)">(.*?)</section>',
        content,
        re.M | re.S)

    result = []

    for section in sections:
        functions = re.findall(r'<dl class="c function">(.*?)</dl>', section, re.M | re.S)
        for function in functions:
            names = re.findall(
                r'<span class="sig-name descname"><span class="n"><span class="pre">(.*?)</span></span></span>',
                function)
            assert len(names) > 0
            result.append(names[0])

    return name, result


def cache_libuv_api(root: str, url: str, ignore_cache: bool = False) -> str:
    cache_json = 'libuv_api.json'
    cache_path = os.path.join(root, cache_json)
    if not ignore_cache and os.path.isfile(cache_path):
        print(f'[INFO] Cache from {cache_path}')
        return cache_path

    sections = []

    print(f'[INFO] Cache API from {url}')
    toc = get_libuv_toc(url)
    for section_url in toc:
        print(f'[INFO] Cache API from {section_url}')
        name, functions = get_libuv_functions(section_url)
        section = OrderedDict()
        section['name'] = name
        section['functions'] = functions
        sections.append(section)

    cache = OrderedDict()
    cache['sections'] = sections

    os.makedirs(root, exist_ok=True)
    with open(cache_path, 'w', encoding='utf-8') as f:
        json.dump(cache, f, indent=4)

    print(f'[INFO] Cache into {cache_path}')

    return cache_path


def get_libuvcxx_source(root: str):
    headers = []

    for header in glob.glob(os.path.join(root, '*.h')):
        with open(header, 'r', encoding='utf-8') as header_file:
            headers.append(header_file.read())

    return '\n'.join(headers)


def calculate_coverage(libuv_api: Dict, libuvcxx_source: str):
    function_count = 0
    covered_count = 0

    sections: List = libuv_api['sections']
    for section in sections:
        name: str = section['name']
        functions: List[str] = section['functions']

        print(f'[INFO] Section "{name}" ... ', end='')

        section_function_count = len(functions)
        section_covered_count = 0
        section_missing: List[str] = []

        for function in functions:
            match_usage = re.findall(rf'[^_A-Za-z]({function})[^_A-Za-z]', libuvcxx_source)
            if match_usage:
                section_covered_count += 1
            else:
                section_missing.append(function)

        function_count += section_function_count
        covered_count += section_covered_count

        if section_covered_count == section_function_count:
            print('[OK]')
        else:
            section_coverage = int(section_covered_count / section_function_count * 100)
            print(f'[{section_coverage}%]')
            for i, miss in enumerate(section_missing):
                prefix = '      ' if i else 'Miss: '
                print(f'[WARN] {prefix}  - {miss}')

    coverage = int(covered_count / function_count * 100)
    print(f'[INFO] Total API coverage [{coverage}%] - [{covered_count}/{function_count}]')


def main():
    scripts_root = os.path.abspath(os.path.dirname(__file__))
    libuvcxx_root = os.path.join(scripts_root, '..', 'include', 'uvcxx')

    libuv_api = cache_libuv_api(
        scripts_root, 'https://docs.libuv.org/en/v1.x/api.html',
        ignore_cache=False)

    libuvcxx_source = get_libuvcxx_source(libuvcxx_root)

    with open(libuv_api, 'r', encoding='utf-8') as api_file:
        api_json = json.load(api_file)
        calculate_coverage(api_json, libuvcxx_source)


if __name__ == '__main__':
    main()
