# Mini-Git

Mini-Git is a small version control system written in C++. Its interactive command line lets you create repositories, build commit histories, and follow a change from a feature branch through a pull request into `main`.

Repositories and users live in memory. Each run starts with a fresh state, so you can experiment with different branching and merge workflows.

## What you can do

- **Manage repositories.** Register users, create repositories, fork a repository with its branch histories, and transfer ownership.
- **Work with commits and branches.** Push commits, create a branch from the current `main` or a specific commit, and inspect the branch log. Commits have SHA-1 identifiers that the program displays in log output.
- **Open and merge pull requests.** Propose changes between branches in the same repository or across repositories. Choose a merge commit, rebase merge, or squash merge.

## Build and run

You need Git to clone the repository, Make to build it, and a compiler that supports C++11. The Makefile uses Clang on macOS and GCC on Linux.

```sh
git clone https://github.com/Kjust-dot/git-code.git
cd git-code
make
./main
```

## Merge a feature branch

At the `vcs2011>` prompt, enter these commands. They create a repository for `alice`, add a commit to the `feature-notes` branch, and squash-merge it into `main`.

```text
register alice
repocreate alice demo 2026-01-01T10:00:00Z
branchcreate alice/demo feature-notes alice
push alice/demo:feature-notes alice "Add project notes" 2026-01-01T10:05:00Z
prcreate alice alice/demo:feature-notes alice/demo:main "Add project notes"
prmerge alice/demo 1 2 2026-01-01T10:10:00Z
prinfo alice/demo 1
branchinfo alice/demo:main
```

`prinfo` should show `Status: Merged`. The `main` log should contain `Add project notes (#1)` above the initial commit. Enter `quit` to exit.

## Command reference

Use `owner/repository` to identify a repository and `owner/repository:branch` to identify a branch. Write timestamps in UTC as `YYYY-MM-DDTHH:MM:SSZ` and put double quotes around multiword commit messages and pull request titles.

| Command | What it does |
| --- | --- |
| `register <user>` | Create a user. |
| `repocreate <owner> <name> <timestamp>` | Create a repository with an initial commit. |
| `branchcreate <repository> <branch> <creator> [commit]` | Create a branch from `main` or from the specified SHA-1 commit ID. |
| `push <branch-ref> <author> <message> <timestamp>` | Add a commit to a branch. |
| `prcreate <author> <source> <target> <title>` | Open a pull request between two branch references. |
| `prmerge <repository> <pr-number> <mode> <timestamp>` | Merge a pull request. Use `0` for a merge commit, `1` for rebase, or `2` for squash. |
| `repoinfo`, `branchinfo`, `prinfo` | Inspect a repository, a branch log, or a pull request. |
| `help` | Show every command and its arguments. |

## Source layout

| File | Responsibility |
| --- | --- |
| [`main.cpp`](main.cpp) | Runs the interactive shell and dispatches commands. |
| [`commands.cpp`](commands.cpp) | Parses command arguments and calls the version control implementation. |
| [`version_control.cpp`](version_control.cpp) | Implements user, repository, branch, commit, fork, and pull request operations. |
| [`structures.h`](structures.h) | Defines the in-memory data structures for users, repositories, branches, commits, and pull requests. |
| [`sha1.cpp`](sha1.cpp) | Implements SHA-1 hashing for commit identifiers. |
