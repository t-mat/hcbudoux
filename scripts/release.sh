#!/bin/sh
# Tag the current main with v<version in include/hcbudoux.h> and push the tag.
# .github/workflows/release.yml then creates the GitHub Release.
set -eu

cd "$(dirname "$0")/.."
tag="v$(sh scripts/version.sh)"

test "$(git rev-parse --abbrev-ref HEAD)" = main || { echo "not on main"; exit 1; }
test -z "$(git status --porcelain)" || { echo "working tree is not clean"; exit 1; }
git fetch origin main
test "$(git rev-parse HEAD)" = "$(git rev-parse origin/main)" || { echo "main differs from origin/main"; exit 1; }
! git rev-parse -q --verify "refs/tags/${tag}" >/dev/null || { echo "${tag} already exists"; exit 1; }

git tag -a "${tag}" -m "${tag}"
git push origin "${tag}"
