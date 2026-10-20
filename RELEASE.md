Release procedure
=================

A release is a `v<MAJOR>.<MINOR>.<PATCH>` tag on `main`.
Pushing the tag runs `.github/workflows/release.yml`, which creates the GitHub Release with `include/hcbudoux.h` attached.

The commands are written for Linux. On Windows, run `make` through WSL (`wsl -e make ...`) and the Python scripts with `python` instead of `python3`.


1. Update the version
---------------------

Edit the `hcbudoux_version_*` enum in `codegen/hcbudoux.template.h`, then regenerate the header and run the tests.

```sh
make all
make all-clang
python3 scripts/version.py  # prints the new version, e.g. 0.10.1
```

If the release updates BudouX, follow "Updating third-party dependencies" in `CLAUDE.md` first, including the BudouX version row in `README.md`.


2. Commit and push `main`
-------------------------

```sh
git commit -a -m "bump version to v<version>"
git push origin main
```


3. Wait for CI on `main`
------------------------

```sh
python3 scripts/ci-status.py --wait
```

Continue only if the exit status is 0 (every run succeeded).
Exit status 1 means a run failed, 2 means no completed result, 3 means the GitHub API request failed.


4. Tag and push the tag
-----------------------

```sh
python3 scripts/release.py
```

`scripts/release.py` stops unless `main` is checked out, the working tree is clean, `HEAD` equals `origin/main`, and the tag does not exist yet.
It then creates the annotated tag `v<version>` and pushes it.


5. Check the release
--------------------

```sh
python3 scripts/ci-status.py --wait
```

The tag points at the commit that step 3 already checked, so `--wait` can return before the `release` run appears.
Run it again until the list includes a `release` run and that run has completed.
The `release` run must succeed. It fails if the tag does not match the version in `include/hcbudoux.h`, or if the regenerated header differs from the committed one.
Then check that https://github.com/t-mat/hcbudoux/releases has `v<version>` with `hcbudoux.h` attached.


If something goes wrong
-----------------------

- CI fails in step 3: fix, commit, push, and repeat step 3. No tag has been published yet.
- The `release` run fails in step 5: fix the cause on `main`, then delete the tag (`git push origin :refs/tags/v<version>` and `git tag -d v<version>`) and repeat from step 2.
  Delete a tag only if no GitHub Release was created for it.
