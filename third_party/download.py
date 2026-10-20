#!/usr/bin/env python3
"""Download the pinned BudouX models and json.h into third_party/.

The commit hashes are read from third_party/budoux_hash.txt and third_party/json_h_hash.txt.
Only the model JSONs, json.h and the LICENSE files are kept.  Archives are staged in _tmp/staging.
"""
import io
import pathlib
import shutil
import urllib.request
import zipfile

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent
STAGING = ROOT / "_tmp" / "staging"


def fetch(url, dest):
    print(f"download {url}")
    with urllib.request.urlopen(url) as r:
        zipfile.ZipFile(io.BytesIO(r.read())).extractall(dest)


def copy(src, dst_dir):
    dst_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst_dir / src.name)


json_h_hash = (HERE / "json_h_hash.txt").read_text().strip()
budoux_hash = (HERE / "budoux_hash.txt").read_text().strip()

shutil.rmtree(STAGING, ignore_errors=True)
fetch(f"https://github.com/sheredom/json.h/archive/{json_h_hash}.zip", STAGING)
fetch(f"https://github.com/google/budoux/archive/{budoux_hash}.zip", STAGING)
json_h = STAGING / f"json.h-{json_h_hash}"
budoux = STAGING / f"budoux-{budoux_hash}"

for d in ("json.h", "budoux"):
    shutil.rmtree(HERE / d, ignore_errors=True)

copy(json_h / "LICENSE", HERE / "json.h")
copy(json_h / "json.h", HERE / "json.h")
copy(budoux / "LICENSE", HERE / "budoux")
for model in sorted((budoux / "budoux" / "models").glob("*.json")):
    copy(model, HERE / "budoux" / "budoux" / "models")

shutil.rmtree(STAGING)
