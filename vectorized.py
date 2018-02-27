#!/usr/bin/env python

import ast
import types

import meta

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
    def statement(node):
        if isinstance(node, list):
            out = []
            for x in node:
                t = ("\n" + meta.dump_python_source(x).strip()).replace("\n", "\n    ")
                out.append(makeyield(len(text)))
                text.append(t)
                out.append(statement(x))
            node = out

        elif isinstance(node, ast.Module):
            node.body = statement(node.body)

        elif isinstance(node, (ast.ClassDef, ast.Exec, ast.FunctionDef, ast.Import, ast.Return)):
            raise SyntaxError("{0} statements are not allowed".format(node.__class__.__text__))

        elif isinstance(node, ast.Expr):
            node.value = expression(node.value)

        elif isinstance(node, ast.Assert):
            node.test = expression(node.test)

        elif isinstance(node, ast.Assign):
            node.targets = [expression(x) for x in node.targets]
            node.value = expression(node.value)

        elif isinstance(node, ast.AugAssign):
            node.target = expression(node.target)
            node.value = expression(node.value)

        elif isinstance(node, ast.Delete):
            node.targets = [expression(x) for x in node.targets]

        elif isinstance(node, ast.ExceptHandler):
            node.body = statement(node.body)
        
        elif isinstance(node, ast.For):
            node.iter = expression(node.iter)
            node.body = statement(node.body)
            node.orelse = statement(node.orelse)

        elif isinstance(node, ast.If):
            node.test = expression(node.test)
            node.body = statement(node.body)
            node.orelse = statement(node.orelse)

        elif isinstance(node, ast.Print):
            node.values = expression(node.values)

        elif isinstance(node, ast.TryExcept):
            node.body = statement(node.body)
            node.orelse = statement(node.orelse)

        elif isinstance(node, ast.TryFinally):
            node.body = statement(node.body)
            node.finalbody = statement(node.finalbody)

        elif isinstance(node, ast.Try):
            node.body = statement(node.body)
            node.orelse = statement(node.orelse)
            node.finalbody = statement(node.finalbody)

        elif isinstance(node, ast.While):
            node.test = expression(node.test)
            node.body = statement(node.body)
            node.orelse = statement(node.orelse)

        elif isinstance(node, ast.With):
            node.body = statement(node.body)
            if hasattr(node, "items"):
                for x in node.items:
                    x.context_expr = expression(x.context_expr)
            else:
                node.context_expr = expression(node.context_expr)

        return node

    def expression(node):
        if isinstance(node, list):
            node = [expression(x) for x in node]

        elif isinstance(node, (ast.DictComp, ast.GeneratorExp, ast.IfExp, ast.Lambda, ast.ListComp, ast.SetComp, ast.Yield)):
            raise SyntaxError("{0} expressions are not allowed".format(node.__class__.__text__))

        elif isinstance(node, ast.AST):
            for n in node._fields:
                setattr(node, n, expression(getattr(node, n)))

        return node

    code = fcn.__code__
    shellfcn = ast.parse("def fcn({0}): pass".format(", ".join(code.co_varnames[:code.co_argcount]))).body[0]
    shellfcn.body = statement(meta.decompile(code)).body
    shellfcn.body.append(makeyield(len(text)))

    instrumented = callexec(compile(ast.Module([shellfcn]), "<vectorized>", "exec"), "fcn").__code__

    return len(text), text, types.FunctionType(instrumented, fcn.__globals__, fcn.__name__, fcn.__defaults__, fcn.__closure__)

def vectorize(fcn, numitems, *args, **kwds):
    numsteps, text, instfcn = instrument(fcn)
    def donext(x):
        try:
            return next(x)
        except StopIteration:
            return numsteps

    iters = [instfcn(i, *args, **kwds) for i in range(numitems)]
    steps = [donext(x) for x in iters]
    numiterations = 1
    while True:
        leading = max(steps)
        atleading = sum(1 if x == leading else 0 for x in steps)
        if leading == len(text):
            break
        else:
            print("leading step {0} ({1}% at leading): {2}".format(leading, round(100.0*atleading/len(steps), 2), text[leading]))

        while atleading < len(steps):
            for i in range(numitems):
                if steps[i] < leading:
                    steps[i] = donext(iters[i])
            print("    ...catching up {0} ({1}% at leading)".format(numiterations, round(100.0*atleading/len(steps), 2)))
            numiterations += 1
            atleading = sum(1 if x == leading else 0 for x in steps)

        print("    ...advancing {0}\n".format(numiterations))
        for i in range(numitems):
            if steps[i] == leading:
                steps[i] = donext(iters[i])
        numiterations += 1

    return numiterations
