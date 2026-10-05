#!/bin/sh
# Install the tools used by this repository on Ubuntu/Debian under WSL.
#   make, gcc/g++          : make / make all / codegen / test / examples
#   clang/clang++          : make all-clang
#   clang-format/clang-tidy: make clang-format / make clang-tidy
#   git                    : header up-to-date check (git diff)
#   curl, unzip            : third_party/download.sh
#   docker.io, act         : scripts/act.sh (run .github/workflows locally)
set -eu

sudo apt-get update
sudo apt-get install -y \
  build-essential \
  clang \
  clang-format \
  clang-tidy \
  git \
  curl \
  unzip \
  docker.io

# Let the current user talk to the Docker daemon without sudo (effective from the next login).
sudo usermod -aG docker "$(id -un)"

# act is not packaged in apt; use the upstream installer.
curl -fsSL https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash -s -- -b /usr/local/bin
