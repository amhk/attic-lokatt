#!/usr/bin/python3
import re

class Transition:
    def __init__(self):
        self.clear()

    def clear(self):
        self.src = None
        self.dest = None
        self.sigma = list()
        self.operations = list()

    def to_dot(self):
        label = ''
        if self.sigma:
            label = '<' + ',\\n'.join(self.sigma) + '>'
        if self.operations:
            label += '\\n' + ',\\n'.join(self.operations)
        return '"{}" -> "{}" [label="{}"];'.format(self.src, self.dest, label)

def main():
    re_step_fn = re.compile(r'^static int step_([a-z_]+)')
    re_case = re.compile(r'^\s*case SIGMA_([A-Z_]+):|^\s*(default):')
    re_op_goto_state = re.compile(r'^\s*goto_state\(ctx, STATE_([A-Z_]+)')
    re_error = re.compile(r'^\s*error')
    re_op_other = re.compile(r'^\s*(push|token)')
    re_return = re.compile(r'^\s*return (0|-1);')
    re_done = re.compile(r'^static int.*dispatch_table')

    t = Transition()

    print('''digraph G {
    node [shape="circle" margin=0.1 style="filled" fillcolor="grey90"];
    edge [fontsize="10"];
    rankdir="LR";
    "BEGIN" [shape="doublecircle"];
    "END" [shape="doublecircle"];
    "ERROR" [fillcolor="orangered"];
    "END" -> "ERROR";
    ''')
    with open('filter-lexer.c') as fd:
        for line in fd:
            m = re_step_fn.match(line)
            if m:
                t.clear()
                t.src = m.group(1).upper()
                continue

            m = re_case.match(line)
            if m:
                if m.group(1):
                    t.sigma.append(m.group(1))
                continue

            m = re_op_goto_state.match(line)
            if m:
                t.dest = m.group(1)
                continue

            m = re_error.match(line)
            if m:
                t.dest = 'ERROR'

            m = re_op_other.match(line)
            if m:
                t.operations.append(m.group(1))

            m = re_return.match(line)
            if m:
                src = t.src
                if t.dest is None:
                    t.dest = t.src
                print(t.to_dot())
                t.clear()
                t.src = src
                continue

            m = re_done.match(line)
            if m:
                break
    print('}')

if __name__ == '__main__':
    main()
