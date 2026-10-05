#!/bin/sh
# Run .github/workflows/test.yml locally with act (see scripts/wsl-setup.sh). Extra arguments are passed to act.
# The runner image only approximates GitHub's ubuntu-latest; a pass here does not prove a pass on GitHub.
# --use-gitignore=false: the allow-list .gitignore (/*) would otherwise keep .git out of the container,
# and the workflow's `git diff` check needs it.
set -eu

cd "$(dirname "$0")/.."
exec act -W .github/workflows/test.yml -P ubuntu-latest=catthehacker/ubuntu:act-latest --use-gitignore=false "$@"
