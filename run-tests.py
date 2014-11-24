#!/usr/bin/python
import argparse
import hashlib
import os
import shlex
import subprocess
import sys
import xml.etree.ElementTree as ET

ANSI_RED = '\033[31m'
ANSI_YELLOW = '\033[33m'
ANSI_RESET = '\033[0m'

def _print_stack(stack):
    def get(root, name, default):
        element = root.find(name)
        if element is None:
            return default
        return element.text

    for i, frame in enumerate(stack.findall('frame')):
        ip = get(frame, 'ip', '??')
        obj = get(frame, 'obj', '??')
        func = get(frame, 'fn', '??')
        path = get(frame, 'file', '??')
        line = get(frame, 'line', '??')
        prefix = 'at' if i == 0 else 'by'

        location = obj
        if path != '??' and line != '??':
            location = '{}:{}'.format(path, line)

        print '        {} {:10s} {:20s} {}'.format(prefix, ip, func, location)

def _print_error(error):
    kind = error.find('kind')
    print '{}==== {} ===={}'.format(ANSI_RED, kind.text, ANSI_RESET)

    for child in error:
        if child.tag == 'what' or child.tag == 'auxwhat':
            print '    {}{}{}'.format(ANSI_YELLOW, child.text, ANSI_RESET)
        elif child.tag == 'xwhat':
            print '    {}{}{}'.format(ANSI_YELLOW, child.find('text').text, ANSI_RESET)
        elif child.tag == 'stack':
            _print_stack(child)

def _parse_valgrind_output(raw):
    retval = 0
    errors_seen = set()
    begin = 0
    end = 0
    while True:
        begin = raw.find('<error>', end)
        if begin == -1:
            return retval
        end = raw.find('</error>', begin)
        if end == -1:
            return retval
        end += len('</error>')
        xml = raw[begin:end]

        root = ET.fromstring(xml)
        sha1 = hashlib.sha1(xml).hexdigest()
        if sha1 in errors_seen:
            continue
        errors_seen.add(sha1)
        _print_error(root)
        retval += 1

def _execute_test(executable, testcase='', valgrind=False):
    if not valgrind:
        cmdline = '{} {}'.format(executable, testcase)
        return subprocess.call(shlex.split(cmdline))

    pipe_r, pipe_w = os.pipe()
    cmdline = 'valgrind --trace-children=yes --leak-check=full --track-origins=yes --xml=yes --xml-fd={} {} {}'.format(pipe_w, executable, testcase)
    retval = subprocess.call(shlex.split(cmdline))
    os.close(pipe_w)
    with os.fdopen(pipe_r) as fin:
        retval += _parse_valgrind_output(fin.read())
    return retval

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--valgrind', action='store_true', default=False, help='run tests under Valgrind memcheck')
    parser.add_argument('testspec', nargs='*', help='test specification: executable[:testcase]')
    args = parser.parse_args()

    retval = 0
    for arg in args.testspec:
        if ':' in arg:
            executable, testcase = arg.split(':')
        else:
            executable = arg
            testcase = ''
        retval += _execute_test(os.path.abspath(executable), testcase, args.valgrind)
    sys.exit(retval)

if __name__ == '__main__':
    main()
