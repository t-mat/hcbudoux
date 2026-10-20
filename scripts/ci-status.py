#!/usr/bin/env python3
"""Print the GitHub Actions runs, jobs and steps for a commit (default: HEAD) of origin's repository.

usage: ci-status.py [--wait] [commit]
  --wait  re-check until every run of the commit has completed.  Gives up if the commit is not on
          any origin branch, or if no run appears within NO_RUN_TIMEOUT seconds.
Set GITHUB_TOKEN (or GH_TOKEN) to raise the API rate limit from 60 to 5000 requests per hour.
Exit status: 0 if every run succeeded, 1 if any did not, 2 if there is no completed result,
3 if the GitHub API request failed.
"""
import json
import os
import re
import subprocess
import sys
import time
import urllib.error
import urllib.request

NO_RUN_TIMEOUT = 120

os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
args = sys.argv[1:]
wait = "--wait" in args
args = [a for a in args if a != "--wait"]
token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
# Unauthenticated requests are limited to 60 per hour.
interval = 15 if token else 90


def git(*a):
    return subprocess.check_output(["git", *a], text=True).strip()


def get(url):
    headers = {"Accept": "application/vnd.github+json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    try:
        with urllib.request.urlopen(urllib.request.Request(url, headers=headers)) as r:
            return json.load(r)
    except urllib.error.HTTPError as e:
        msg = f"GitHub API error: {e.code} {e.reason}: {url}"
        if e.headers.get("x-ratelimit-remaining") == "0":
            reset = int(e.headers.get("x-ratelimit-reset", "0"))
            msg += f"\nrate limit exceeded; resets at {time.strftime('%H:%M:%S', time.localtime(reset))}"
        print(msg, file=sys.stderr)
        sys.exit(3)
    except urllib.error.URLError as e:
        print(f"GitHub API error: {e.reason}: {url}", file=sys.stderr)
        sys.exit(3)


sha = git("rev-parse", args[0] if args else "HEAD")
repo = re.sub(r"\.git$", "", re.sub(r"^.*github\.com[:/]", "", git("remote", "get-url", "origin")))
api = f"https://api.github.com/repos/{repo}/actions"

if wait:
    subprocess.check_call(["git", "fetch", "-q", "origin"])
    if not git("branch", "-r", "--contains", sha):
        print(f"{sha} is not on any origin branch; no run to wait for", file=sys.stderr)
        sys.exit(2)

start = time.monotonic()
while True:
    runs = get(f"{api}/runs?head_sha={sha}")["workflow_runs"]
    done = bool(runs) and all(r["status"] == "completed" for r in runs)
    if done or not wait:
        break
    if not runs and time.monotonic() - start > NO_RUN_TIMEOUT:
        print(f"no run appeared for {sha} within {NO_RUN_TIMEOUT} seconds", file=sys.stderr)
        break
    time.sleep(interval)

print(repo, sha)
for r in runs:
    print(f"run {r['name']}: {r['status']} {r['conclusion']} {r['html_url']}")
    for j in get(f"{api}/runs/{r['id']}/jobs")["jobs"]:
        print(f"  job {j['name']}: {j['status']} {j['conclusion']}")
        for s in j.get("steps") or []:
            print(f"    {s['name']}: {s['conclusion']}")

if not done:
    sys.exit(2)
sys.exit(0 if all(r["conclusion"] == "success" for r in runs) else 1)
