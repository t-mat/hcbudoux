#!/usr/bin/env python3
"""Tag the current main with v<version in include/hcbudoux.h> and push the tag.

.github/workflows/release.yml then creates the GitHub Release.  See RELEASE.md.
"""
import os
import subprocess
import sys

sys.dont_write_bytecode = True  # keep scripts/__pycache__ out of the working tree
from version import version  # noqa: E402

os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))


def git(*a):
    return subprocess.check_output(["git", *a], text=True).strip()


def fail(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)


tag = "v" + version()

if git("rev-parse", "--abbrev-ref", "HEAD") != "main":
    fail("not on main")
if git("status", "--porcelain"):
    fail("working tree is not clean")
subprocess.check_call(["git", "fetch", "origin", "main"])
if git("rev-parse", "HEAD") != git("rev-parse", "origin/main"):
    fail("main differs from origin/main")
if subprocess.run(["git", "rev-parse", "-q", "--verify", f"refs/tags/{tag}"], stdout=subprocess.DEVNULL).returncode == 0:
    fail(f"{tag} already exists")

subprocess.check_call(["git", "tag", "-a", tag, "-m", tag])
subprocess.check_call(["git", "push", "origin", tag])
