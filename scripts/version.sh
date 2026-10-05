#!/bin/sh
# Print the version in include/hcbudoux.h as MAJOR.MINOR.PATCH.
set -eu

cd "$(dirname "$0")/.."
v() { sed -n "s/^ *hcbudoux_version_$1 *= *\([0-9][0-9]*\),.*/\1/p" include/hcbudoux.h | tr -d '\r'; }
echo "$(v major).$(v minor).$(v patch)"
