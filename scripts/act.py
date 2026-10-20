#!/usr/bin/env python3
"""Run .github/workflows/test.yml locally with act (https://github.com/nektos/act). Extra arguments go to act.

The runner image only approximates GitHub's ubuntu-latest; a pass here does not prove a pass on GitHub.
--use-gitignore=false: the allow-list .gitignore (/*) would otherwise keep .git out of the container,
and the workflow's `git diff` check needs it.
"""
import os
import subprocess
import sys

os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
cmd = [
    "act",
    "-W", ".github/workflows/test.yml",
    "-P", "ubuntu-latest=catthehacker/ubuntu:act-latest",
    "--use-gitignore=false",
    *sys.argv[1:],
]
try:
    sys.exit(subprocess.call(cmd))
except FileNotFoundError:
    sys.exit("act not found; see scripts/wsl-setup.sh")
