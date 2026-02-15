---
title: Cache VFXManager and AudioManager pointers in BeginPlay
status: closed
priority: 1
issue-type: task
created-at: "\"\\\"2026-02-15T08:57:16.452186-06:00\\\"\""
closed-at: "2026-02-15T09:22:46.632929-06:00"
close-reason: Cached VFXManager and AudioManager pointers in BeginPlay. Replaced per-frame subsystem lookups in Tick and TickTimer with cached pointers.
---

Engine Architect. VFXManager and AudioManager lookups every frame in GameMode::Tick. Cache pointers in BeginPlay instead. 10-min cleanup.
