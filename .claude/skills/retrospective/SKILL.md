---
name: retrospective
description: Structured protocol for team retrospectives that produce concrete process improvements
context: fork
---

## Retrospective Protocol

Run this protocol after each feature completion, or when the team encounters significant friction.

### Step 1: Gather Data

Review the evidence since the last retrospective:

```bash
# Commits since last retro (look for the [retro] tag)
git log --oneline --since="$(git log --all --grep='[retro]' -1 --format='%ci' 2>/dev/null || echo '1 week ago')"

# Failed builds or reverted commits
git log --oneline --all --grep='revert\|fix\|broken\|oops'

# Files with the most churn (potential design issues)
git log --pretty=format: --name-only | sort | uniq -c | sort -rn | head -20
```

### Step 2: Identify Friction

Categorize observations into:

**Process Issues** (how we work)
- Driver rotation unclear?
- Tasks too large or too vague?
- Communication gaps between agents?
- Agreements not being followed?

**Technical Issues** (what we build)
- Repeated build failures?
- Flaky tests?
- Asset pipeline errors?
- Merge conflicts?

**Quality Issues** (what we ship)
- Bugs found after "done"?
- Missing tests?
- Inconsistent code style?
- Performance regressions?

### Step 3: Propose Changes

For each issue, write a specific, actionable change:

**Bad**: "We should communicate better"
**Good**: "Before any agent starts implementing, post a 3-sentence plan in the team channel and wait for acknowledgment from at least one other agent"

**Bad**: "Tests should be better"
**Good**: "Every gameplay test must verify: input registered, state changed, visual feedback triggered, audio feedback triggered"

### Step 4: Apply Changes

1. Update `.team/agreements.md` with new or modified rules
2. Update agent profiles in `.claude/agents/` if role boundaries need adjustment
3. Update skills in `.claude/skills/` if technical patterns need refinement
4. Add hooks in `.claude/settings.json` if automated enforcement is needed

### Step 5: Log the Retrospective

Append to `.team/retrospective-log.md`:

```markdown
## Retrospective [DATE] - After [FEATURE/MILESTONE]

### What Went Well
- [Specific positive observation]

### What Caused Friction
- [Issue]: [Root cause]

### Changes Made
- [File modified]: [What changed and why]

### Action Items
- [ ] [Specific follow-up task]
```

### Step 6: Commit

```bash
git add .team/ .claude/
git commit -m "refactor: update team agreements from retrospective [retro]"
```

## When to Trigger

- After completing any feature (mandatory)
- After 3+ failed builds in a session
- After any reverted commit
- When any agent reports feeling "stuck" or confused about process
- At minimum, after every 2 hours of work

## Anti-Patterns to Watch For

- **Retrospective theater**: Going through motions without making real changes
- **Blame-oriented**: Focus on what went wrong in the process, not who did it
- **Too many changes at once**: Pick the top 1-3 friction points. Don't rewrite everything.
- **Ignoring previous retro items**: Check that last retro's action items were completed
