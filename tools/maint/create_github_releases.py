#!/usr/bin/env python3
# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Script to create GitHub releases for old tags.

Assumes 'gh' CLI is installed and authenticated.
"""

import argparse
import json
import os
import re
import subprocess
import sys


def get_tags():
  """Returns a sorted list of all semver tags."""
  tags = subprocess.check_output(['git', 'tag']).decode('utf-8').splitlines()
  semver_tags = []
  for tag in tags:
    if re.match(r'^\d+\.\d+\.\d+$', tag):
      semver_tags.append(tag)

  # Sort semver tags descending (newest first)
  semver_tags.sort(key=lambda x: list(map(int, x.split('.'))), reverse=True)
  return semver_tags


def get_releases(repo):
  """Returns a set of tags that already have GitHub releases."""
  try:
    api_cmd = ['gh', 'api', f'repos/{repo}/releases', '--paginate', '-q', '.[].tag_name']
    api_output = subprocess.check_output(api_cmd).decode('utf-8')
    releases = api_output.splitlines()
  except subprocess.CalledProcessError as e:
    print(f"Failed to get releases via API: {e}", file=sys.stderr)
    sys.exit(1)

  return set(releases)


def get_changelog_notes(version):
  script_dir = os.path.dirname(os.path.abspath(__file__))
  root_dir = os.path.dirname(os.path.dirname(script_dir))
  changelog_path = os.path.join(root_dir, 'ChangeLog.md')

  try:
    with open(changelog_path, encoding='utf-8') as f:
      lines = f.readlines()
  except OSError as e:
    print(f"Failed to read ChangeLog.md: {e}", file=sys.stderr)
    return None

  start_idx = -1
  for i, line in enumerate(lines):
    if re.match(rf'^{re.escape(version)}(\s|:|$)', line):
      if i + 1 < len(lines) and re.match(r'^-------+$', lines[i + 1].strip()):
        start_idx = i + 2
        break

  if start_idx == -1:
    return None

  notes = []
  for i in range(start_idx, len(lines)):
    line = lines[i]
    if i + 1 < len(lines) and re.match(r'^-------+$', lines[i + 1].strip()):
      if re.match(r'^\d+\.\d+\.\d+(\s|:|$)', line):
        break
    notes.append(line)

  while notes and not notes[-1].strip():
    notes.pop()

  return ''.join(notes).strip()


def get_new_contributors(repo, tag):
  """Returns the 'New Contributors' section from auto-generated release notes."""
  try:
    cmd = ['gh', 'api', '--method', 'POST', f'/repos/{repo}/releases/generate-notes', '-f', f'tag_name={tag}']
    output = subprocess.check_output(cmd).decode('utf-8')
    data = json.loads(output)
    body = data.get('body', '')

    # Extract "New Contributors" section
    match = re.search(r'(## New Contributors\s+.*?)(?:\s*\*\*Full Changelog\*\*|$)', body, re.DOTALL)
    if match:
      return match.group(1).strip()
  except (subprocess.CalledProcessError, json.JSONDecodeError) as e:
    print(f"Warning: Failed to generate/parse notes for {tag} to extract contributors: {e}", file=sys.stderr)
  return None


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('count', type=int, help='Number of releases to create')
  parser.add_argument('--dry-run', action='store_true', help='Show what would be done')
  parser.add_argument('--repo', default='emscripten-core/emscripten', help='GitHub repository (default: emscripten-core/emscripten)')
  parser.add_argument('--draft', action='store_true', help='Create release as draft')
  args = parser.parse_args()

  all_tags = get_tags()
  existing_releases = get_releases(args.repo)

  # Only consider tags >= 2.0.0 for creating releases
  tags_to_consider = [t for t in all_tags if int(t.split('.')[0]) >= 2]
  missing_releases = [t for t in tags_to_consider if t not in existing_releases]

  print(f"Found {len(tags_to_consider)} semver tags >= 2.0.0 (out of {len(all_tags)} total)")
  print(f"Found {len(existing_releases)} existing releases")
  print(f"Found {len(missing_releases)} missing releases")

  to_create = missing_releases[:args.count]
  print(f"Will create {len(to_create)} releases: {to_create}")

  for tag in to_create:
    print(f"Creating release for {tag}...")
    notes = get_changelog_notes(tag)
    if notes:
      notes = f"## What's Changed\n\n{notes}"
    else:
      print(f"Warning: No changelog notes found for {tag}. Using empty notes.")
      notes = ""

    # Add New Contributors if found
    contributors = get_new_contributors(args.repo, tag)
    if contributors:
      if notes:
        notes += f"\n\n{contributors}"
      else:
        notes = contributors

    # Add Full Changelog link
    try:
      idx = all_tags.index(tag)
      if idx + 1 < len(all_tags):
        prev_tag = all_tags[idx + 1]
        compare_link = f"https://github.com/{args.repo}/compare/{prev_tag}...{tag}"
        if notes:
          notes += f"\n\n**Full Changelog**: {compare_link}"
        else:
          notes = f"**Full Changelog**: {compare_link}"
    except ValueError:
      pass

    extra_flags = []
    if args.draft:
      extra_flags.append('--draft')

    if args.dry_run:
      extra_flags_str = ' '.join(extra_flags)
      if extra_flags_str:
        extra_flags_str = ' ' + extra_flags_str
      print(f"DRY RUN: gh release create {tag} -t {tag} -F - --latest=false --verify-tag{extra_flags_str} -R {args.repo}")
      print("--- Notes ---")
      print(notes)
      print("-------------")
    else:
      cmd = ['gh', 'release', 'create', tag, '-t', tag, '-F', '-', '--latest=false', '--verify-tag', *extra_flags, '-R', args.repo]
      try:
        subprocess.run(cmd, input=notes.encode('utf-8'), check=True)
      except subprocess.CalledProcessError as e:
        print(f"Failed to create release for {tag}: {e}", file=sys.stderr)
        return 1

  return 0


if __name__ == '__main__':
  sys.exit(main())
