# Agent Instructions

## AI Assistance and Attribution

**AI assistance is permitted in CustFunc. Attribution is required.**

Using an AI coding assistant — Claude, GitHub Copilot, Cursor, ChatGPT, or
anything comparable — to help write code, documentation, tests, or data in this
repository is allowed and needs no prior approval. What is not optional is
saying so.

### When attribution is required

Attribute whenever an AI tool **generated or materially shaped** something that
lands in the repository: source code, comments, documentation, test cases, fluid
data, build/CI configuration, or the commit and PR text itself.

Attribution is **not** required when the assistant never touched what you
committed — editor autocomplete of a symbol name, asking a model to explain
existing code, or using one to search the codebase or the literature.

The line to apply: *would a reviewer reading this diff want to know an AI wrote
it?* When in doubt, attribute. Over-attribution costs nothing; undisclosed AI
authorship is the thing this policy exists to prevent.

### How to attribute

**1. Commit trailer (required).** Every commit whose content an AI **generated
or materially shaped** carries a `Co-authored-by:` trailer naming the tool, in
the trailer block at the end of the commit message after a blank line:

```
update(CoolProp_EN.xml): Update CoolProp XML function reference

Co-authored-by: Claude Sonnet 5 <noreply@anthropic.com>
```

Name the model or tool. Including the version is encouraged but not mandatory —
`Claude Sonnet 5.5`, `Claude`, `GitHub Copilot`, and `Cursor` are all acceptable
identities; an unattributed AI-written commit is not. Use the tool's documented
noreply address where it has one (Anthropic's is `noreply@anthropic.com`). If
several tools contributed, use one trailer line per tool.

This is already the de facto convention in this repository — `git log` shows
`Co-authored-by: Claude ...` on AI-assisted merges going back many releases.
This section makes it a rule rather than a habit.

**2. PR description (required).** In the pull request body, state briefly what
the AI did and what you verified yourself. The **Verification Process** section
of the [PR template](.github/PULL_REQUEST_TEMPLATE.md) is the natural place for
it. For example:

> Claude Opus 5 drafted the keyboard hook modifications and its unit tests.
> chose the key binding by hand, updated the About Box text, and verified
> that the new functionality works within the Mathcad Prime interface.

A commit trailer is invisible in the GitHub review UI; the prose note is what
the reviewer actually sees.

### What attribution does not change

Attribution is a disclosure, not a disclaimer. The human contributor is the
author of record and remains fully responsible for the change:

- **Correctness.** You must understand the code you submit and be able to
  explain and defend it in review. "The model wrote it" is not an answer to a
  review question.
- **Testing.** AI-generated code is held to the same standard as any other: it
  needs tests, and it must pass the project's quality gates.
- **Provenance and licensing.** By submitting, you assert the contribution is
  yours to give under CustFunc's MIT license. Verify that generated code is not
  a verbatim reproduction of incompatibly licensed source.

Maintainers may request changes on, or close, a pull request that appears to be
unreviewed AI output. Review capacity is the scarce resource here; generated
volume that a human has not read first spends it without adding value.

## Issue Tracking

This project uses GitHub Issues for manual issue tracking.

