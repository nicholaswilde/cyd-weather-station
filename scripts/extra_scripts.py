import subprocess
import os

Import("env")

def get_git_version():
    try:
        # Check if we are in a git repository
        if os.path.exists(os.path.join(env.subst("$PROJECT_DIR"), ".git")):
            result = subprocess.run(
                ["git", "describe", "--dirty", "--always", "--tags"],
                capture_output=True,
                text=True,
                check=True
            )
            return result.stdout.strip()
    except Exception as e:
        print(f"Git version check failed: {e}")
        
    # Fallback to reading version.txt (e.g. for users downloading the source ZIP)
    try:
        version_file = os.path.join(env.subst("$PROJECT_DIR"), "version.txt")
        with open(version_file, "r") as f:
            return f.read().strip()
    except Exception:
        return "unknown"

firmware_version = get_git_version()
print(f"Firmware Version: {firmware_version}")

# Write to version.h
project_dir = env.subst("$PROJECT_DIR")
header_path = os.path.join(project_dir, 'include', 'version.h')
with open(header_path, 'w') as f:
    f.write(f'#pragma once\n#define APP_VERSION "{firmware_version}"\n')
print(f"Generated version.h with APP_VERSION '{firmware_version}'")
