# Contributing to FoundationCore

This document describes how we work together on FoundationCore. It is intentionally written in a direct and explanatory
style so that expectations are clear, especially for contributors who are still gaining experience with collaborative
systems programming projects.

FoundationCore is not a typical application project. It is a low-level, performance-sensitive system written in C, where
small inconsistencies in structure or assumptions can propagate into subtle bugs. Because of this, how we work matters
just as much as what we build.

## How Work Flows Through the Repository

The repository follows a two-branch model. There is a `develop` branch where all active work happens, and a `main`
branch that represents stable, release-ready code.

In practice, this means you should think of `develop` as the place where the system is being assembled, piece by piece.
It may be incomplete or temporarily unstable, and that is acceptable. The `main` branch, on the other hand, is not a
working area. It is where code arrives only after it has been validated and stabilized.

A typical piece of work starts as a branch created from `develop`, evolves through commits, is reviewed via a Pull
Request, and is eventually merged back into `develop`. When a set of changes reaches a stable point, it is promoted
through a release branch into `main`.

## Starting a Task

All work begins with an issue. This is not optional. The issue is the place where the intent of the change is recorded,
and it ensures that we do not drift into unstructured or overlapping work.

A good issue describes a concrete piece of the system. For example, implementing a protocol primitive, defining a buffer
abstraction, or building a connection pipeline. It should be specific enough that someone else can understand what needs
to be done without guessing.

Once an issue exists, you create a branch from `develop`. The name of the branch should reflect both the type of change
and the area of the system it touches. For example, a feature related to protocol encoding might be named
`feature/protocol-varint`, while a bug fix in networking might be `fix/network-disconnect`.

This naming is not cosmetic. It allows anyone looking at the repository to immediately understand what is being worked
on.

## Implementing Changes

While working on a branch, the expectation is not just to "make it work," but to make it consistent with the rest of the
system.

FoundationCore places strong emphasis on clarity and explicitness. Memory ownership should be obvious. Control flow
should be easy to follow. Errors should be handled explicitly rather than ignored or hidden.

If you find yourself inventing a new pattern or deviating from existing conventions, pause and ask. Early alignment is
significantly cheaper than later refactoring.

Work should remain scoped to the issue. If you discover additional problems, they should be recorded as new issues
rather than silently folded into the current work.

## Pull Requests and Review

Once the work reaches a coherent state, you open a Pull Request targeting `develop`.

A Pull Request is not just a formality. It is the primary mechanism by which we validate changes. The description should
explain what was implemented, why it was implemented that way, and any trade-offs that were made.

Before a Pull Request can be merged, a few conditions must be satisfied. The code must build and pass all automated
checks. At least one other person must review and approve the changes. Any comments raised during review must be
addressed or explicitly resolved.

For changes that target `main`, the bar is higher. These merges require at least two approvals and should only happen
through a release or controlled stabilization process.

## Commits

Commits should be small, focused, and readable. Each commit should represent a single logical change.

The message should follow a simple structure where the scope of the change is stated first, followed by a concise
description. For example, a commit might read "protocol: implement VarInt decoder" or "core: add buffer reserve logic."

Large commits that mix unrelated changes make review difficult and should be avoided. If a change feels too large, it
likely needs to be split.

## Releases and Versioning

Development progresses continuously on `develop`. As the system evolves, we tag pre-releases such as `v0.1.0-alpha.1`,
`v0.1.0-alpha.2`, and so on. These tags represent checkpoints rather than final states.

When a milestone reaches a level of stability that is acceptable for a release, a dedicated branch is created, for
example `release/0.1.0`. At that point, the focus shifts from building new features to fixing issues and stabilizing
behavior.

Once the release branch is stable, it is merged into `main` and tagged with the final version, such as `v0.1.0`. The
release branch is then merged back into `develop` to ensure no fixes are lost.

## Things to Avoid

There are a few practices that will cause problems if not controlled.

Direct pushes to `main` or `develop` bypass review and must not be done. Similarly, merging your own Pull Request
without review defeats the purpose of collaboration and should be avoided.

Introducing new dependencies or changing architectural boundaries without discussion can create long-term maintenance
issues. These changes must be deliberate and agreed upon.

## When You Are Blocked

Getting stuck is expected, especially in a project of this nature. What matters is how you handle it.

When you are blocked, describe the problem clearly, explain what you have tried, and ask for input. Doing this early
prevents wasted time and keeps the team aligned.

Working in isolation for too long on a problem that is unclear tends to produce fragile solutions.

## Final Notes

FoundationCore is both a learning project and a serious engineering effort. The goal is not just to produce working
code, but to build a system that is understandable, consistent, and maintainable.

Mistakes are part of the process. Unspoken assumptions are not. When something is unclear, it is always better to ask or
open a draft Pull Request than to proceed silently.
