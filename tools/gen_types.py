#!/usr/bin/env python3
"""Generate a LuaCATS types.lua from C++ sol2 binding sources.

Scans C++ files for Doxygen-style doc comments tagging the Lua API surface:

    /*
     * @namespace Instance
     * Object registry.
     */
    sol::table instance_namespace = lua["Instance"].get_or_create<sol::table>();

    /*
     * @field spawn
     * Creates a new object.
     */
    instance_namespace["spawn"] = [&registry]() { return (uint32_t)registry.create(); };

and emits LuaCATS annotations consumed by the sumneko Lua language server:

    ---@class Instance
    Instance = {}

    ---Creates a new object.
    ---@return integer
    function Instance.spawn() end

Tags:
    @namespace Name   (alias @class)  a Lua global table
    @field name [type]                a member on the current namespace
    @type function|field              kind of the member (default: function)
    @param name type  (optional)      override a parameter's type
    @return type      (optional)      override the return type

@type field emits a LuaCATS `---@field` annotation on the class; its Lua type
comes from `@field name <type>`, else `@return`, else is inferred from a
`sol::property` getter / value assignment. @type function (the default) emits a
`function` stub, inferring params/return from the lambda or `sol::constructors`.
When @param/@return are absent, types are inferred from the signature.

Usage:
    python tools/gen_types.py [files...] [-o OUTPUT]

With no files, scans src/lua/lib/*.cpp. With no -o, writes to stdout.
"""

import argparse
import glob
import re
import sys

CPP_TO_LUA = {
    "void": None,
    "bool": "boolean",
    "float": "number",
    "double": "number",
    "int": "integer",
    "unsigned": "integer",
    "char": "integer",
    "short": "integer",
    "long": "integer",
    "int8_t": "integer",
    "int16_t": "integer",
    "int32_t": "integer",
    "int64_t": "integer",
    "uint8_t": "integer",
    "uint16_t": "integer",
    "uint32_t": "integer",
    "uint64_t": "integer",
    "size_t": "integer",
    "std::string": "string",
    "string": "string",
    "entt::entity": "integer",
}

COMMENT_BLOCK = re.compile(r"/\*(.*?)\*/", re.DOTALL)


def map_type(cpp):
    cpp = cpp.strip()
    is_ptr = cpp.endswith("*") or "char *" in cpp or "char*" in cpp
    base = cpp.replace("const", "").replace("&", "").replace("*", "").strip()
    base = re.sub(r"\s+", " ", base)
    if base in ("char", "char ") and is_ptr:
        return "string"
    if "char" in base and is_ptr:
        return "string"
    return CPP_TO_LUA.get(base, "any" if base else None)


def split_top_level(arglist):
    depth = 0
    cur = ""
    parts = []
    for ch in arglist:
        if ch in "<([":
            depth += 1
        elif ch in ">)]":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur)
            cur = ""
        else:
            cur += ch
    if cur.strip():
        parts.append(cur)
    return [p.strip() for p in parts]


def parse_params(arglist):
    arglist = arglist.strip()
    if not arglist or arglist == "void":
        return []
    params = []
    for part in split_top_level(arglist):
        m = re.match(r"^(.*?)([A-Za-z_]\w*)\s*$", part)
        if not m:
            params.append((part, "any"))
            continue
        cpp_type, name = m.group(1).strip(), m.group(2)
        lua = map_type(cpp_type) or "any"
        params.append((name, lua))
    return params


def parse_params_unnamed(arglist):
    """Split a type-only argument list (e.g. a constructor signature), naming
    each positionally as arg1, arg2, ..."""
    arglist = arglist.strip()
    if not arglist or arglist == "void":
        return []
    return [(f"arg{i + 1}", map_type(p) or "any") for i, p in enumerate(split_top_level(arglist))]


def infer_field_type(code_after):
    """Infer a @type field's Lua type from a sol::property getter (`-> T {`) or a
    direct value assignment."""
    m = re.search(r"\[[^\]]*\]\s*\([^)]*\)\s*->\s*([A-Za-z_][\w:<>\*&\s]*?)\s*\{", code_after)
    if m:
        return map_type(m.group(1))
    m = re.search(r"=\s*(true|false)\b", code_after)
    if m:
        return "boolean"
    m = re.search(r'=\s*"', code_after)
    if m:
        return "string"
    m = re.search(r"=\s*-?\d", code_after)
    if m:
        return "number"
    return None


def infer_signature(code_after):
    """Given source immediately following a @field comment, infer (params, ret).

    Handles a sol2 constructor list (`sol::constructors<Class(args)>`), a free
    function definition (`RetType name(args) { ... }`) or a sol2 lambda
    assignment (`["key"] = [captures](args) { ... }`).
    """
    ctor = re.match(r"\s*sol::constructors\s*<\s*([A-Za-z_]\w*)\s*\(([^)]*)\)\s*>", code_after)
    if ctor:
        return parse_params_unnamed(ctor.group(2)), ctor.group(1)

    fn = re.match(r"\s*([A-Za-z_][\w:<>\*&\s]*?)\b(\w+)\s*\(([^)]*)\)", code_after)
    if fn and "=" not in code_after[: fn.start(2)]:
        params = parse_params(fn.group(3))
        ret = map_type(fn.group(1))
        return params, ret

    m = re.search(r"=\s*\[[^\]]*\]\s*\(([^)]*)\)", code_after)
    if not m:
        return None, None
    params = parse_params(m.group(1))
    rest = code_after[m.end():]
    ret = None
    cast = re.search(r"return\s*\(\s*([A-Za-z_][\w:<> ]*?)\s*\)", rest)
    if cast:
        ret = map_type(cast.group(1))
    elif re.search(r"\breturn\b\s*[^;]", rest):
        ret = "any"
    return params, ret


def parse_file(path):
    with open(path, "r", encoding="utf-8") as f:
        src = f.read()

    by_name = {}
    order = []
    pending = []
    current = None

    def add_field(ns, fld):
        if any(existing["name"] == fld["name"] for existing in ns["fields"]):
            return
        ns["fields"].append(fld)

    for block in COMMENT_BLOCK.finditer(src):
        lines = [re.sub(r"^\s*\*+\s?", "", ln).rstrip() for ln in block.group(1).splitlines()]
        tag = None
        name = None
        kind = None
        field_decl_type = None
        desc = []
        params = {}
        ret = None
        for ln in lines:
            t = ln.strip()
            mt = re.match(r"@(\w+)\s*(.*)$", t)
            if mt:
                key, val = mt.group(1), mt.group(2).strip()
                if key in ("namespace", "class"):
                    tag, name = "namespace", val.split()[0] if val else None
                elif key == "field":
                    tag = "field"
                    toks = val.split()
                    name = toks[0] if toks else None
                    field_decl_type = toks[1] if len(toks) > 1 else None
                elif key == "type":
                    kind = val.split()[0] if val else None
                elif key == "param":
                    pm = re.match(r"(\w+)\s+(\S+)\s*(.*)$", val)
                    if pm:
                        params[pm.group(1)] = (pm.group(2), pm.group(3).strip())
                elif key == "return":
                    rm = re.match(r"(\S+)\s*(.*)$", val)
                    if rm:
                        ret = (rm.group(1), rm.group(2).strip())
            elif t:
                desc.append(t)

        if tag == "namespace" and name:
            if name not in by_name:
                by_name[name] = {"name": name, "desc": " ".join(desc), "fields": []}
                order.append(name)
            elif desc:
                by_name[name]["desc"] = " ".join(desc)
            current = by_name[name]
            for fld in pending:
                add_field(current, fld)
            pending = []
        elif tag == "field" and name:
            code_after = src[block.end():block.end() + 400]
            if kind == "field":
                ftype = field_decl_type or (ret[0] if ret else None) or infer_field_type(code_after)
                fld = {
                    "name": name,
                    "kind": "field",
                    "desc": " ".join(desc),
                    "type": ftype or "any",
                }
            else:
                inferred_params, inferred_ret = infer_signature(code_after)
                fld = {
                    "name": name,
                    "kind": "function",
                    "desc": " ".join(desc),
                    "params": params,
                    "ret": ret,
                    "inferred_params": inferred_params,
                    "inferred_ret": inferred_ret,
                }
            if current is not None:
                add_field(current, fld)
            else:
                pending.append(fld)

    if pending and order:
        for fld in pending:
            add_field(by_name[order[0]], fld)

    return [by_name[n] for n in order]


def emit(namespaces):
    out = ["---@meta", ""]
    for ns in namespaces:
        props = [f for f in ns["fields"] if f.get("kind") == "field"]
        funcs = [f for f in ns["fields"] if f.get("kind") != "field"]
        if ns["desc"]:
            out.append(f"---{ns['desc']}")
        out.append(f"---@class {ns['name']}")
        for prop in props:
            line = f"---@field {prop['name']} {prop['type']}"
            if prop["desc"]:
                line += f" {prop['desc']}"
            out.append(line)
        out.append(f"{ns['name']} = {{}}")
        out.append("")
        for fld in funcs:
            param_names = []
            inferred = {n: t for n, t in (fld["inferred_params"] or [])}
            order = [n for n, _ in (fld["inferred_params"] or [])]
            for n in fld["params"]:
                if n not in order:
                    order.append(n)

            if fld["desc"]:
                out.append(f"---{fld['desc']}")
            for n in order:
                param_names.append(n)
                if n in fld["params"]:
                    ptype, pdesc = fld["params"][n]
                else:
                    ptype, pdesc = inferred.get(n, "any"), ""
                line = f"---@param {n} {ptype}"
                if pdesc:
                    line += f" {pdesc}"
                out.append(line)

            ret_type, ret_desc = None, ""
            if fld["ret"]:
                ret_type, ret_desc = fld["ret"]
            elif fld["inferred_ret"]:
                ret_type = fld["inferred_ret"]
            if ret_type:
                line = f"---@return {ret_type}"
                if ret_desc:
                    line += f" {ret_desc}"
                out.append(line)

            out.append(f"function {ns['name']}.{fld['name']}({', '.join(param_names)}) end")
            out.append("")
    return "\n".join(out).rstrip() + "\n"


def main():
    ap = argparse.ArgumentParser(description="Generate LuaCATS types.lua from C++ sol2 bindings")
    ap.add_argument("files", nargs="*", help="C++ source files (default: src/lua/lib/*.cpp)")
    ap.add_argument("-o", "--output", help="output file (default: stdout)")
    args = ap.parse_args()

    files = args.files or sorted(glob.glob("src/lua/lib/*.cpp"))
    if not files:
        print("no input files", file=sys.stderr)
        return 1

    namespaces = []
    for path in files:
        namespaces.extend(parse_file(path))

    text = emit(namespaces)
    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(text)
        print(f"wrote {args.output} ({len(namespaces)} namespaces)", file=sys.stderr)
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
