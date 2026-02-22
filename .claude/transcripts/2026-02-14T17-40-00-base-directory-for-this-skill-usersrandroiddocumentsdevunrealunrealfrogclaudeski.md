# Conversation: 2026-02-14

| Field | Value |
|-------|-------|
| Date | 2026-02-14 |
| Session ID | `339b0de4-e57f-45a6-95db-bed50f5e150c` |
| Model | claude-opus-4-6 |
| Tokens | ~2,032,600 input, ~136 output |
| Duration | 3m 0s |

---

## User (17:40 UTC)

Base directory for this skill: /Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/retrospective

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

## Assistant (17:40 UTC)

🤖️

I'll run the retrospective protocol. Let me start by gathering data.

- **Used Bash**: Commits since last retro

- **Used Bash**: Failed builds or reverted commits since last retro

- **Used Bash**: Files with most churn since last retro

## Assistant (17:40 UTC)

The last retro commit is recent enough that the since-filter returned nothing. Let me broaden the search.

- **Used Bash**: Last 30 commits

- **Used Bash**: Find all retro commits

## Assistant (17:40 UTC)

No commits have the `[retro]` tag, but I can see the retro marker at `8f5c72f`. Let me gather data since that Sprint 2 retrospective.

- **Used Bash**: Commits since Sprint 2 retro

- **Used Bash**: Fix/revert commits since Sprint 2 retro

- **Used Bash**: Most churned files since Sprint 2 retro

## Assistant (17:40 UTC)

Good. Now let me review the current state of the working tree (uncommitted changes), the existing retrospective log, and the current agreements.

- **Used Bash**: Uncommitted changes overview

- **Used Read** on `.team/retrospective-log.md`

- **Used Bash**: Full commit messages since Sprint 2 retro

## Assistant (17:40 UTC)

Now let me look at the detail of each commit since the Sprint 2 retro (the post-hotfix work) and the current uncommitted changes.

- **Used Bash**: Detailed commit stats since Sprint 2 retro

- **Used Bash**: Uncommitted changes stats

- **Used Bash**: Uncommitted changed file names

## Assistant (17:40 UTC)

Now let me look at the current state of the key files and the uncommitted changes more closely. Let me also check how many tests we have now and if there are any open action items from the last retro.

- **Used Bash**: Count test classes/specs

- **Used Bash**: Count test assertions

- **Used Glob** for `Source/UnrealFrog/Tests/**/*.cpp` in `.`

## Assistant (17:41 UTC)

Now let me read the current agent profiles to see what was changed, and check the open action items from the last retro.

- **Used Read** on `.claude/agents/engine-architect.md`

- **Used Read** on `.claude/agents/qa-lead.md`

- **Used Read** on `.claude/agents/devops-engineer.md`

## Assistant (17:41 UTC)

Now I have a comprehensive picture. Let me compile the retrospective. Let me first check the open action items from the last retro and see which ones were addressed.

- **Used Bash**: List commit subjects since retro

## Assistant (17:41 UTC)

Good. Now I have all the data I need. Let me write the retrospective entry and update the relevant files.

Let me check which action items from the Post-Sprint 2 Hotfix were completed.

- **Used Grep** for `GetOrCreateFlatColorMaterial|LoadObject.*M_FlatColor` in `Source`

- **Used Grep** for `AddDynamic` in `Source`

- **Used Grep** for `TActorIterator` in `Source`

## Assistant (17:41 UTC)

Now let me assess the post-Sprint 2 action items status:

1. "P0: Build PlayUnreal E2E harness" -- `run-tests.sh` was improved with `--functional`, `--report`, `--timeout`, but full E2E with inputs/state/screenshots is not done yet.
2. "Create persistent M_FlatColor .uasset" -- `FlatColorMaterial.h` now tries LoadObject first, and a Python script exists. Partially done.
3. "Add integration tests that verify delegate bindings exist" -- `DelegateWiringTest.cpp` exists with 9 delegate verifications. DONE.
4. "Add a wiring smoke test" -- `IntegrationTest.cpp` has full-chain tests. DONE.

Let me also check the open questions from Sprint 2 retro.

- **Used Glob** for `Source/UnrealFrog/Tests/Functional/*.cpp` in `.`

- **Used Read** on `Tools/PlayUnreal/run-tests.sh`

## Assistant (17:41 UTC)

Excellent. Now I have all the data. Let me write the retrospective entry. Here is my analysis:

**Since Sprint 2 Retro (6 commits):**

1. `dcbca11` - fix: 4 critical gameplay bugs (materials, overlaps, delegates, HUD)
2. `610c4a9` - test: delegate wiring + integration smoke tests
3. `e2af0a1` - feat: functional test framework (5 AFunctionalTest actors)
4. `0c0b72a` - chore: FlatColor .uasset fallback + run-tests.sh improvements
5. `97c601b` - feat: pause + turtle submerge + 7 gameplay tests
6. `cd64812` - fix: river hop snapping + home slot completion

**2 fix commits out of 6** (33% were bug fixes found during/after play-testing).

Let me now write the retrospective log entry and update agreements.

- **Used Read** on `.team/agreements.md`

## Assistant (17:41 UTC)

Now I have all the data. Let me write the retrospective entry and update the agreements.

- **Used Edit** on `.team/retrospective-log.md`

## Assistant (17:42 UTC)

Now update the agreements file with the three changes identified.

- **Used Edit** on `.team/agreements.md`

- **Used Edit** on `.team/agreements.md`

- **Used Edit** on `.team/agreements.md`

## Assistant (17:42 UTC)

Now add the new section 15 about moving platform hop convention.

- **Used Edit** on `.team/agreements.md`

## Assistant (17:43 UTC)

Now let me verify the files look correct.

- **Used Read** on `.team/agreements.md` (offset=1, limit=30)

- **Used Read** on `.team/retrospective-log.md` (offset=239)

## Assistant (17:43 UTC)

Both files look correct. Let me also verify the agreements section 15 was added properly.

- **Used Read** on `.team/agreements.md` (offset=155)

## Assistant (17:43 UTC)

Everything looks good. Here is the retrospective summary.

---

## Retrospective 3 -- Post-Sprint 2 Hotfix through Sprint 2 Completion

### Data Gathered

**Commits analyzed:** 6 commits from `dcbca11` through `cd64812` (after the Sprint 2 retrospective at `8f5c72f`).

**Fix ratio:** 2 of 6 commits (33%) were bug fixes discovered during play-testing rather than during TDD. Both were integration-level bugs at the seams between systems.

**Most churned files:** `FrogCharacter.cpp`, `UnrealFrogGameMode.cpp`, `HazardBase.cpp`, `FroggerHUD.cpp`, and `IntegrationTest.cpp` -- each touched in 2 commits.

**Test count:** 102 test macros across 20 test files, plus 5 AFunctionalTest actors. Up from ~81 at the start of this period.

### Top 3 Friction Points Identified

1. **Tests verify systems in isolation but not their seams.** Hopping works. Platforms work. Hopping from a moving platform broke because nobody tested that specific interaction. Similarly, the home slot completion flow was missing because the frog system and the game-over system each worked independently.

2. **PlayUnreal E2E is still not complete.** This was the P0 action item from the last retro. The test runner improved (`--functional`, `--report`, `--timeout`) and 5 functional test actors were created, but agents still cannot launch the game, send inputs, and query state programmatically. The stakeholder remains the only true play-tester.

3. **Functional tests cannot run headlessly.** The AFunctionalTest actors are the most valuable tests we have (they would have caught Sprint 2's critical bugs), but they require `--functional` mode with a rendering context, which means they cannot run in CI or automated pipelines.

### Changes Applied

Three changes to `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.team/agreements.md`:

1. **Section 2 (TDD is Mandatory):** Added seam test requirement -- for every pair of interacting systems, at least one test must exercise their boundary. Isolation tests are necessary but not sufficient.

2. **Section 5a (Definition of Done):** Added "Seam tests exist for all new system interactions introduced this sprint" to the checklist.

3. **New Section 15 (Moving Platform Hop Convention):** Codifies the rule that `StartHop` must use actual world position when on a moving platform, and `FinishHop` must back-calculate `GridPosition` from the landing location.

Retrospective entry appended to `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.team/retrospective-log.md` as "Retrospective 3."

### Sprint 3 Action Items (Prioritized)

| Priority | Item | Owner |
|----------|------|-------|
| P0 | Complete PlayUnreal E2E harness (launch, inputs, state query, assertions) | DevOps Engineer |
| P0 | Add seam tests: hop-from-moving-platform, die-on-submerged-turtle, pause-during-river-ride | Engine Architect |
| P1 | Generate M_FlatColor.uasset via Python script | DevOps Engineer |
| P1 | Investigate running functional tests headlessly (Xvfb or Gauntlet) | DevOps Engineer |
| P2 | Platform-hop regression test | Engine Architect |
