# Development workflow

## Local workflow

1. Inspect `git status`, branch, history, and references.
2. Read the relevant legacy/core implementation before changing behavior.
3. Implement one focused feature and update its documentation.
4. Run the smallest available browser or C++ validation.
5. Review `git diff --check` and stage only related files.
6. Commit with a Conventional Commit message.

## Conventions

Use focused modules, explicit state transitions, and ASCII by default. Keep
core rules independent of presentation. Do not silently swallow errors or
claim tests/builds that were not run.

## Git safety

Do not use destructive reset/checkout/clean commands, force push, or amend
existing commits. For conflicts, inspect both sides, resolve deliberately,
verify unmerged paths are empty, and document the result. The repository's
three-file conflict lab is documented in the conflict report.
