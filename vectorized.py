#!/usr/bin/env python

import ast
import types

import meta

def fcn(index):
    print "ONE", index
    print "TWO", index
    print "THREE", index

def callexec(code, name):
    env = {}
    exec(code, env)
    return env[name]

def instrument(fcn):
    def makeyield(x):
        out = ast.Expr(ast.Yield(ast.Num(x)))
        out.lineno, out.col_offset = 1, 0
        out.value.lineno, out.value.col_offset = 1, 0
        out.value.value.lineno, out.value.value.col_offset = 1, 0
        return out

    text = []
    def recurse(node):
        if isinstance(node, list):
            out = []
            for x in node:
                out.append(makeyield(len(text)))
                out.append(x)
                text.append(meta.dump_python_source(x).strip())
            out.append(makeyield(len(text)))
            node = out

        elif isinstance(node, ast.Module):
            node.body = recurse(node.body)

        return node

    code = fcn.__code__
    shellfcn = ast.parse("def fcn({0}): pass".format(", ".join(code.co_varnames[:code.co_argcount]))).body[0]
    shellfcn.body = recurse(meta.decompile(code)).body
    instrumented = callexec(compile(ast.Module([shellfcn]), "<vectorized>", "exec"), "fcn").__code__

    return len(text), text, types.FunctionType(instrumented, fcn.__globals__, fcn.__name__, fcn.__defaults__, fcn.__closure__)

def vectorize(fcn, numitems, *args, **kwds):
    numsteps, text, instfcn = instrument(fcn)
    iters = [instfcn(i, *args, **kwds) for i in range(numitems)]
    steps = [next(x) for x in iters]
    while True:
        leading = max(steps)
        atleading = sum(1 if x == leading else 0 for x in steps)
        if leading == len(text):
            break
        else:
            print("leading step {0} ({1}% at leading): {2}".format(leading, int(round(100.0*atleading/len(steps))), text[leading]))

        while atleading < len(steps):
            for i in range(numitems):
                if steps[i] < leading:
                    steps[i] = next(iters[i])
            atleading = sum(1 if x == leading else 0 for x in steps)

        for i in range(numitems):
            if steps[i] == leading:
                steps[i] = next(iters[i])

vectorize(fcn, 10)
