# Git conflict resolution lab

## Purpose

This lab validates the repository's Git integration workflow without creating
conflicts in production source. It used the isolated temporary repository
`C:\Users\MINH\AppData\Local\Temp\block-bloom-conflict-lab`, which was removed
after verification.

## Procedure

1. Created a baseline commit containing three configuration fixtures.
2. Created `feature-a` and changed the same lines in all three files.
3. Created `feature-b` from the baseline and changed those lines differently.
4. Merged `feature-a` into `feature-b`.
5. Verified the three files were unmerged, then resolved each file with a
   deliberate combined configuration.
6. Staged the resolutions and created the merge commit.

The important commands were:

```text
git init
git switch -c feature-a
git switch -c feature-b
git merge feature-a --no-edit
git status --short
git diff --name-only --diff-filter=U
git ls-files -u
git add game-rules.conf input-controls.conf ui-theme.conf
git commit -m "merge: resolve configuration conflicts"
```

## Conflicts and resolutions

| Fixture | Feature A | Feature B | Resolution |
| --- | --- | --- | --- |
| `game-rules.conf` | 420 ms gravity | 460 ms gravity | 440 ms compromise, preserving the original 15-column board and scoring table |
| `input-controls.conf` | Arrow keys and Space | J/L/I/K | Arrow keys and Space, matching the browser UI and documented controls |
| `ui-theme.conf` | coral accent, 10px radius | violet accent, 8px radius | coral accent and 10px radius, matching the Block Bloom visual system |

## Verification

The merge produced exactly **3 real content conflicts**:

```text
game-rules.conf
input-controls.conf
ui-theme.conf
```

Before resolution, `git ls-files -u` reported 9 index entries (three stages
for each file). After resolution:

- `git diff --name-only --diff-filter=U` returned no paths.
- `git status --short` returned no changes.
- The resolved files contained no `<<<<<<<`, `=======`, or `>>>>>>>` markers.
- The merge commit was `ccd447090132e88d0c928aee5932c8564b35c794`.

The temporary repository and its branches were not added to the main project.
