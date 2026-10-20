#!/bin/sh
# Install the tools used by this repository on Ubuntu/Debian under WSL.
#   make, gcc/g++          : make / make all / codegen / test / examples
#   clang/clang++          : make all-clang
#   clang-format/clang-tidy: make clang-format / make clang-tidy
#   git                    : header up-to-date check (git diff)
#   python3                : scripts/*.py, third_party/download.py
#   docker.io, act         : scripts/act.py (run .github/workflows/test.yml locally)
#   curl                   : the act installer below
set -eu

sudo apt-get update
sudo apt-get install -y \
  build-essential \
  clang \
  clang-format \
  clang-tidy \
  git \
  python3 \
  curl \
  docker.io

# Let the current user talk to the Docker daemon without sudo (effective from the next login).
sudo usermod -aG docker "$(id -un)"

# act is not packaged in apt; use the upstream installer.
curl -fsSL https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash -s -- -b /usr/local/bin
