#!/usr/bin/env python3
"""Print the version in include/hcbudoux.h as MAJOR.MINOR.PATCH."""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent


def version():
    text = (ROOT / "include" / "hcbudoux.h").read_text(encoding="utf-8")
    parts = []
    for name in ("major", "minor", "patch"):
        m = re.search(rf"^\s*hcbudoux_version_{name}\s*=\s*(\d+)\s*,", text, re.MULTILINE)
        if not m:
            sys.exit(f"hcbudoux_version_{name} not found in include/hcbudoux.h")
        parts.append(m.group(1))
    return ".".join(parts)


if __name__ == "__main__":
    print(version())
