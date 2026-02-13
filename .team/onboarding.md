# Onboarding New Specialist Agents

## When to Add a Specialist

Add a new agent when the team identifies a knowledge gap that existing agents can't cover. Examples:
- "We need a shader specialist for water rendering"
- "We need a networking expert for multiplayer"
- "We need a UI/UX specialist for the menu system"

## How to Add

1. **Identify the gap** during a retrospective or team discussion
2. **Define the role** — what does this agent own? What expertise does it bring?
3. **Research the inspiration** — find a real industry expert whose methodology fits
4. **Create the agent file** in `.claude/agents/[name].md` with:
   - YAML frontmatter (name, description, tools, model, skills)
   - Expert-inspired principles (3-5 core beliefs)
   - Ownership list (what files/systems they own)
   - Standards they follow
   - Memory file reference
5. **Create their memory directory** at `.claude/agent-memory/[name]/MEMORY.md`
6. **Update the roster** in `.team/roster.md`
7. **Update role boundaries** to avoid overlap with existing agents
8. **The new agent reads**:
   - `CLAUDE.md` (project overview)
   - `.team/agreements.md` (team norms)
   - Their own agent profile
   - Relevant skills

## Retiring an Agent

If a role is no longer needed:
1. Move the agent file to `.claude/agents/retired/`
2. Update the roster
3. Reassign any owned files/systems to remaining agents
4. Log the change in the retrospective log

## Template

```markdown
---
name: [agent-name]
description: [One-line role description]. Inspired by [Expert Name]
tools: Read, Write, Edit, Bash, Grep, Glob
model: opus
skills:
  - [relevant-skill-1]
  - [relevant-skill-2]
---

You are the [Role Name], responsible for [domain]. Your philosophy draws from [Expert Name].

## Core Principles
1. [Principle from expert's methodology]
2. [Principle from expert's methodology]
3. [Principle from expert's methodology]

## Ownership
- [Files/systems this agent owns]

## Before Writing Code
1. Read `.team/agreements.md` — confirm you are the current driver
2. [Role-specific pre-work]

## Memory
Read and update `.claude/agent-memory/[agent-name]/MEMORY.md` each session.
```
