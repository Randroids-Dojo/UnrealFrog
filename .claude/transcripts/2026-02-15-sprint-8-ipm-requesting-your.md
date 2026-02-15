# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `247322c9-2ef2-44f0-a3c1-77b896924763` |
| Model | claude-opus-4-6 |
| Tokens | ~2,573,818 input, ~296 output |
| Duration | 2m 0s |

---

## User (17:53 UTC)

<teammate-message teammate_id="xp-coach" color="blue" summary="PlayUnreal architecture questions for Sprint 8">
Sprint 8 IPM — requesting your input on PlayUnreal architecture.

I've drafted a sprint plan (sent to team-lead). Key questions for you:

1. **PlayUnreal C++ server (UPlayUnrealServer):** The stakeholder review settled on "C++ HTTP server on localhost + Python client." How would you implement this in UE 5.7? Options:
   - FHttpServerModule (engine's built-in HTTP server)
   - Raw FSocket TCP listener
   - FTcpListener + custom HTTP parsing
   
   What's the LOC estimate for the server subsystem with 4 endpoints (get_state, hop, screenshot, reset_game)?

2. **Input injection for /hop endpoint:** The stakeholder review mentioned Automation Driver (`FAutomationDriverModule::Get().GetDriver()->PressKey()`). Is that viable in headless/PIE mode? Or should we call `AFrogCharacter::RequestHop()` directly from the server handler?

3. **Screenshot for /screenshot endpoint:** `FScreenshotRequest` + `UGameViewportClient::OnScreenshotCaptured`? Or viewport read pixels?

4. **Phase 2 VFX/HUD fixes:** You estimated ~15 LOC for score pop, ~30 LOC for VFX visibility. Can these be done independently from PlayUnreal (i.e., committed before PlayUnreal is ready, then verified afterward)?

5. **Risk:** What's the biggest unknown? I'm concerned about the HTTP server approach being 200+ LOC of new C++ infrastructure. Is there a simpler path?

Your answers will shape the final sprint plan. I want to get the phasing and estimates right before committing to scope.
</teammate-message>

## Assistant (17:53 UTC)

Let me answer these questions precisely. Let me first verify a few things about the engine's HTTP module availability.

- **Used Grep** for `FHttpServerModule` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source`

- **Used Grep** for `HttpServerModule` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source`

- **Used Grep** for `FHttpServerModule` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime`

- **Used Read** on `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer/Public/HttpServerModule.h`

- **Used Glob** for `*.h` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer/Public`

- **Used Read** on `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer/Public/IHttpRouter.h`

- **Used Read** on `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer/Public/HttpRequestHandler.h`

- **Used Read** on `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer/Public/HttpServerResponse.h`

- **Used Read** on `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer/Public/HttpServerRequest.h`

## Assistant (17:53 UTC)

Now let me check the HTTPServer module name for Build.cs dependency and look at how the screenshot API works.

- **Used Grep** for `module.*name.*=.*\"HTTPServer\"` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer`

- **Used Glob** for `*.Build.cs` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer`

- **Used Read** on `Source/UnrealFrog/UnrealFrog.Build.cs`

- **Used Read** on `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Online/HTTPServer/HttpServer.Build.cs`

## Assistant (17:53 UTC)

Good. The module is named `HTTPServer`. Now let me check FScreenshotRequest API.

- **Used Grep** for `FScreenshotRequest` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Engine/Classes`

- **Used Grep** for `class.*FScreenshotRequest` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime`

- **Used Grep** for `FScreenshotRequest` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime`

- **Used Grep** for `RequestScreenshot|TakeScreenshot|SaveScreenshot` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Engine/Classes`

- **Used Grep** for `FScreenshotRequest"` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Engine/Public/UnrealClient.h`

- **Used Grep** for `FScreenshotRequest` in `/Users/Shared/Epic Games/UE_5.7/Engine/Source/Runtime/Engine/Public/UnrealClient.h`

## Assistant (17:54 UTC)

Now I have everything I need. Let me send the detailed technical answers to xp-coach.

- **Used SendMessage**

## Assistant (17:55 UTC)

Both messages sent to xp-coach. I have provided:

1. **Full Sprint 8 proposal** covering all 7 tasks in my domain with LOC estimates, dependencies, driver assignments, and a recommendation to defer GameCoordinator to Sprint 9.

2. **Detailed PlayUnreal architecture answers** for all 5 questions:
   - **Q1:** Use `FHttpServerModule` (verified in UE 5.7 source). ~150 LOC total for the server subsystem. Module name is `"HTTPServer"`.
   - **Q2:** Call `RequestHop()` directly, not Automation Driver. Thread-safe via `AsyncTask(ENamedThreads::GameThread, ...)`.
   - **Q3:** `FScreenshotRequest::RequestScreenshot()` with async file save. Works in `-game` mode, black in NullRHI.
   - **Q4:** VFX/HUD fixes are fully independent from PlayUnreal. Phase 1 (fixes) can commit before Phase 2 (server).
   - **Q5:** Biggest risk is whether `FHttpServerModule` initializes in `-game` mode. Recommended a 20-line spike test before full implementation.

Now waiting for xp-coach to finalize the sprint plan and assign tasks.
