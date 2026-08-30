#!/usr/bin/env python3
import sys
import subprocess
import re
import os

def run_cmd(cmd):
    return subprocess.check_output(cmd, shell=True).decode('utf-8').strip()

def main():
    if len(sys.argv) > 1:
        git_range = sys.argv[1]
        tags = git_range.split("..")
        if len(tags) == 2:
            latest = tags[1]
        else:
            latest = run_cmd("git tag --sort=-v:refname | head -n 1")
    else:
        tags = run_cmd("git tag --sort=-v:refname | head -n 2").splitlines()
        if len(tags) < 2:
            latest = tags[0] if tags else "HEAD"
            git_range = "HEAD"
        else:
            latest = tags[0]
            git_range = f"{tags[1]}..{tags[0]}"

    print(f"Using range: {git_range}")

    log_output = run_cmd(f'git log --pretty=format:"- %s" {git_range}')
    
    sections = {
        'feat': [],
        'fix': [],
        'improve': [],
        'docs': []
    }

    for line in log_output.splitlines():
        if "conductor" in line.lower() or "checkpoint" in line.lower():
            continue
        
        match = re.match(r'- (\w+)(\([^)]+\))?:\s*(.*)', line)
        if not match:
            continue
            
        type_str, scope, msg = match.groups()
        msg = msg.strip()
        msg = msg[0].upper() + msg[1:] if msg else msg
        
        if scope:
            scope = scope[1:-1] # remove parens
            entry = f"- `{scope}`: {msg}"
        else:
            entry = f"- {msg}"

        if type_str == 'feat':
            sections['feat'].append(entry)
        elif type_str == 'fix':
            sections['fix'].append(entry)
        elif type_str in ['refactor', 'perf', 'style']:
            sections['improve'].append(entry)
        elif type_str == 'docs':
            sections['docs'].append(entry)

    summary = []
    if sections['feat']:
        summary.append("### 🚀 **New Features**\n\n" + "\n".join(sections['feat']) + "\n")
    if sections['fix']:
        summary.append("### 🐛 **Bug Fixes**\n\n" + "\n".join(sections['fix']) + "\n")
    if sections['improve']:
        summary.append("### ✨ **Improvements**\n\n" + "\n".join(sections['improve']) + "\n")
    if sections['docs']:
        summary.append("### 📝 **Documentation**\n\n" + "\n".join(sections['docs']) + "\n")
        
    url_range = git_range.replace("..", "...")
    summary.append(f"**Full Changelog**: https://github.com/nicholaswilde/cyd-weather-station/compare/{url_range}\n")
    
    notes = "\n".join(summary)
    
    with open('/tmp/release_notes.md', 'w') as f:
        f.write(notes)
        
    print(f"Updating draft release for {latest}...")
    try:
        url = run_cmd(f"gh release edit {latest} --draft -F /tmp/release_notes.md")
        print(f"Draft release updated: {url}")
    except Exception as e:
        print(f"Failed to update release: {e}")
    finally:
        if os.path.exists('/tmp/release_notes.md'):
            os.remove('/tmp/release_notes.md')

if __name__ == "__main__":
    main()
