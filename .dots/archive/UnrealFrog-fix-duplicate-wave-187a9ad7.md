---
title: Fix duplicate wave-complete detection in HandleHopCompleted vs TryFillHomeSlot
status: closed
priority: 0
issue-type: task
created-at: "\"\\\"2026-02-15T08:57:15.874772-06:00\\\"\""
closed-at: "2026-02-15T09:20:11.396715-06:00"
close-reason: Removed duplicate wave-complete path from HandleHopCompleted. TryFillHomeSlot→OnWaveComplete is now the single authority. Existing AllHomeSlotsFilled test covers the consolidated flow.
---

Engine Architect. Consolidate wave-complete detection paths. Both HandleHopCompleted and TryFillHomeSlot trigger wave-complete logic. 15-min refactor. BLOCKS play-test.
