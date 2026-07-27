# /release

Automates the versioning, tagging, and deployment process for the project.

## Description
This skill handles the release process: determining the next version, validating the code quality, committing metadata updates, tagging, and pushing atomically to the remote repository.

## Protocol

1. **Extract and Calculate Version:**
   - Detect the latest tag from git:
     `git tag --sort=-v:refname | head -n 1`
   - Calculate the new patch version by incrementing the last digit (e.g., `0.1.3` -> `0.1.4`).

2. **Pre-release Validation:**
   - Run `task check` to ensure static analysis / cppcheck is clean.
   - Run `task test` to guarantee all native unit tests pass before making the release.

3. **Update Version File and Commit:**
   - Update `version.txt` with the new version: `echo "v<new_version>" > version.txt`
   - Stage the file: `git add version.txt`
   - Commit the change: `git commit -m "chore(release): bump version to v<new_version>"`

4. **Verify Git State:**
   - Check `git status --porcelain` to ensure there are no unexpected local modifications.
   - Run `git pull --rebase` to ensure the local branch is synchronized with `origin main`.

5. **Tag the Release:**
   - Create an annotated git tag: `git tag -a v<new_version> -m "Release v<new_version>"`
   - **NOTE:** Use non-interactive command flags (e.g., `git tag -a -m`) to prevent terminal prompts or editor spawning.

6. **Atomic Push:**
   - Push the branch and the new tag atomically:
     `git push --atomic origin main v<new_version>`

7. **Error Handling:**
   - If any step fails, stop immediately, do not push, and report the detailed error to the user.
