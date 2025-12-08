# API Testing with PalantírRequest

**PalantírRequest** provides a fluent API for testing REST and GraphQL endpoints with automatic distributed tracing. Inspired by Mocha/Chai, Cypress HTTP interception, and Garmin's DataDog APM patterns.

---

## Quick Start

### Simple GET Request

```cpp
NEXUS_TEST(FMyAPITest, "MyGame.API.HealthCheck", ETestPriority::Normal)
{
    FPalantirResponse Res = FPalantirRequest::Get("https://api.mygame.com/health")
        .WithTimeout(5.0f)
        .ExpectStatus(200)
        .ExecuteBlocking();
    
    if (!Res.IsSuccess())
    {
        UE_LOG(LogPalantirTrace, Error, TEXT("Health check failed: HTTP %d"), Res.StatusCode);
        return false;
    }
    
    return true;
}
```

### POST with JSON Validation

```cpp
NEXUS_TEST(FMyAPITest, "MyGame.API.CreatePlayer", ETestPriority::Normal)
{
    FString Body = TEXT("{\"username\": \"test_user\", \"level\": 1}");
    
    FPalantirResponse Res = FPalantirRequest::Post("https://api.mygame.com/players", Body)
        .WithHeader("Authorization", "Bearer <token>")
        .ExpectStatus(201)  // Created
        .ExpectJSON("username", "test_user")
        .ExpectJSON("level", "1")
        .ExecuteBlocking();
    
    return Res.IsSuccess();
}
```

### GraphQL Query

```cpp
NEXUS_TEST(FMyAPITest, "MyGame.API.FetchPlayer", ETestPriority::Normal)
{
    FString Query = TEXT("{ player(id: 123) { name level inventory { items } } }");
    
    TMap<FString, FString> Variables;
    Variables.Add(TEXT("playerId"), TEXT("123"));
    
    FPalantirResponse Res = FPalantirRequest::GraphQL(
        "https://api.mygame.com/graphql",
        Query,
        Variables
    )
    .ExpectStatus(200)
    .ExpectBodyContains("player")
    .ExecuteBlocking();
    
    return Res.IsSuccess();
}
```

---

## Automatic Trace ID Propagation

Every PalantírRequest automatically injects the current test's trace ID into HTTP headers:

```
X-Trace-ID: nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e
User-Agent: NexusTest/nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e
```

This enables **end-to-end distributed tracing** across your game client, backend services, and game industry observability tools:

**Game Industry Standards:**
- **Sentry** (error tracking, performance monitoring) — used by Epic Games, Unity
- **PlayFab** (Microsoft Azure Gaming) — telemetry, analytics, LiveOps
- **GameAnalytics** — player behavior tracking, monetization metrics
- **Unreal Insights** — native UE5 profiling with custom trace channels
- **Backtrace** (Unity) — crash reporting with breadcrumb trails
- **AWS GameLift** — multiplayer server telemetry

**Backend correlation example:**

```csharp
// PlayFab CloudScript / Azure Functions backend
[FunctionName("GetPlayer")]
public static async Task<IActionResult> GetPlayer(
    [HttpTrigger(AuthorizationLevel.Function, "get")] HttpRequest req)
{
    string traceId = req.Headers["X-Trace-ID"];
    log.LogInformation($"[{traceId}] Fetching player {playerId}");
    
    // Report to PlayFab telemetry
    PlayFabServerAPI.WritePlayerEvent(new WriteServerPlayerEventRequest {
        EventName = "test_api_call",
        Body = new { trace_id = traceId }
    });
    // ... handle request ...
}
```

Now you can grep a single trace ID across:
- Unreal Engine client logs
- PlayFab/Azure backend logs
- GameAnalytics events
- Sentry error reports
- Unreal Insights trace channels

---

## Fluent Assertion API

### Status Code Validation

```cpp
// Expect exact status
.ExpectStatus(200)

// Expect status range (any 2xx)
.ExpectStatusRange(200, 299)

// Expect error status
.ExpectStatus(404)
```

### Header Validation

```cpp
.ExpectHeader("Content-Type", "application/json")
.ExpectHeader("X-Rate-Limit-Remaining", "99")
```

### JSON Path Validation

PalantírRequest supports simplified JSONPath queries using dot notation:

```cpp
// Response body: {"user": {"name": "John", "stats": {"level": 50}}}

.ExpectJSON("user.name", "John")
.ExpectJSON("user.stats.level", "50")
```

### Body Substring Validation

```cpp
.ExpectBodyContains("success")
.ExpectBodyContains("player_id")
```

---

## Response Handling

### Synchronous (Blocking)

```cpp
FPalantirResponse Res = FPalantirRequest::Get("https://api.example.com/data")
    .ExecuteBlocking();

// Access response fields
UE_LOG(LogTemp, Display, TEXT("Status: %d"), Res.StatusCode);
UE_LOG(LogTemp, Display, TEXT("Duration: %.1fms"), Res.DurationMs);
UE_LOG(LogTemp, Display, TEXT("Body: %s"), *Res.Body);

// Parse JSON
TSharedPtr<FJsonObject> JSON = Res.GetJSON();
if (JSON.IsValid())
{
    FString Username = JSON->GetStringField("username");
}

// Get JSON value by path
FString PlayerName = Res.GetJSONValue("player.name");
```

### Asynchronous (Non-Blocking)

```cpp
FPalantirRequest::Get("https://api.example.com/data")
    .ExecuteAsync([](const FPalantirResponse& Res)
    {
        if (Res.IsSuccess())
        {
            UE_LOG(LogTemp, Display, TEXT("Async request completed: %d"), Res.StatusCode);
        }
    });
```

---

## Error Handling & Retries

### Automatic Retries with Exponential Backoff

```cpp
FPalantirResponse Res = FPalantirRequest::Get("https://api.flaky-endpoint.com/data")
    .WithRetry(3, 1.0f)  // 3 retries, starting with 1s delay
    .ExpectStatus(200)
    .ExecuteBlocking();

// Retry delays: 1s, 2s, 4s (exponential backoff)
```

### Timeout Configuration

```cpp
.WithTimeout(10.0f)  // 10 second timeout
```

### Validation Failures

If expectations fail, PalantírRequest logs detailed error messages:

```
LogPalantirTrace: Warning: Expected status 200, got 404
LogPalantirTrace: Warning: Expected JSON path user.name=John, got Jane
LogPalantirTrace: Error: Request failed after 3 attempts: Expected status 200, got 503
```

---

## Convenience Macros

For common patterns, use these shorthand macros:

### PALANTIR_ASSERT_GET_OK

Assert GET request returns 200 OK:

```cpp
NEXUS_TEST(FMyAPITest, "MyGame.API.QuickCheck", ETestPriority::Normal)
{
    PALANTIR_ASSERT_GET_OK("https://api.mygame.com/health");
    return true;
}
```

### PALANTIR_ASSERT_POST_CREATED

Assert POST request returns 201 Created:

```cpp
PALANTIR_ASSERT_POST_CREATED("https://api.mygame.com/players", "{\"username\":\"test\"}");
```

### PALANTIR_ASSERT_HEALTH_CHECK

Assert health check endpoint responds with any 2xx:

```cpp
PALANTIR_ASSERT_HEALTH_CHECK("https://api.mygame.com/status");
```

---

## Integration Patterns

### Backend API Contract Testing

Validate your game's backend APIs from integration tests:

```cpp
NEXUS_TEST(FBackendAPITest, "Backend.Players.CRUD", ETestPriority::High)
{
    // Create player
    FString CreateBody = TEXT("{\"username\":\"test_player\",\"email\":\"test@example.com\"}");
    FPalantirResponse CreateRes = FPalantirRequest::Post("https://api.mygame.com/players", CreateBody)
        .ExpectStatus(201)
        .ExpectJSON("username", "test_player")
        .ExecuteBlocking();
    
    if (!CreateRes.IsSuccess()) return false;
    
    // Extract player ID
    FString PlayerID = CreateRes.GetJSONValue("id");
    
    // Fetch player
    FPalantirResponse GetRes = FPalantirRequest::Get(FString::Printf(TEXT("https://api.mygame.com/players/%s"), *PlayerID))
        .ExpectStatus(200)
        .ExpectJSON("id", PlayerID)
        .ExecuteBlocking();
    
    if (!GetRes.IsSuccess()) return false;
    
    // Delete player
    FPalantirResponse DeleteRes = FPalantirRequest::Delete(FString::Printf(TEXT("https://api.mygame.com/players/%s"), *PlayerID))
        .ExpectStatus(204)  // No Content
        .ExecuteBlocking();
    
    return DeleteRes.IsSuccess();
}
```

### GraphQL Schema Validation

Test GraphQL queries for breaking changes:

```cpp
NEXUS_TEST(FGraphQLTest, "Backend.GraphQL.PlayerSchema", ETestPriority::High)
{
    FString Query = TEXT("{ player(id: 1) { __typename id name level inventory { items { name } } } }");
    
    FPalantirResponse Res = FPalantirRequest::GraphQL("https://api.mygame.com/graphql", Query)
        .ExpectStatus(200)
        .ExpectBodyContains("__typename")
        .ExpectBodyContains("Player")
        .ExecuteBlocking();
    
    if (!Res.IsSuccess()) return false;
    
    // Validate schema structure
    TSharedPtr<FJsonObject> Data = Res.GetJSON();
    if (!Data.IsValid()) return false;
    
    TSharedPtr<FJsonObject> PlayerData = Data->GetObjectField("data")->GetObjectField("player");
    
    // Assert required fields exist
    if (!PlayerData->HasField("id")) return false;
    if (!PlayerData->HasField("name")) return false;
    if (!PlayerData->HasField("inventory")) return false;
    
    return true;
}
```

### Third-Party API Integration

Test external service integrations (payment, analytics, etc.):

```cpp
NEXUS_TEST(FThirdPartyTest, "ThirdParty.PaymentAPI.ProcessTransaction", ETestPriority::Normal)
{
    FString Body = TEXT("{\"amount\":9.99,\"currency\":\"USD\",\"userId\":\"test_user\"}");
    
    FPalantirResponse Res = FPalantirRequest::Post("https://api.payment-provider.com/v1/transactions", Body)
        .WithHeader("Authorization", "Bearer <test_api_key>")
        .WithHeader("Idempotency-Key", FGuid::NewGuid().ToString())
        .ExpectStatus(200)
        .ExpectJSON("status", "approved")
        .ExecuteBlocking();
    
    return Res.IsSuccess();
}
```

---

## Best Practices

### 1. Mark Network Tests as OnlineOnly

```cpp
NEXUS_TEST(FMyAPITest, "MyGame.API.Test", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
    // Test requires internet connectivity
}
```

This allows CI to skip network tests in air-gapped environments.

### 2. Use Test-Specific API Keys

Never hardcode production credentials:

```cpp
// ❌ BAD
.WithHeader("Authorization", "Bearer prod_key_12345")

// ✅ GOOD
FString TestAPIKey = FPlatformMisc::GetEnvironmentVariable(TEXT("TEST_API_KEY"));
.WithHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *TestAPIKey))
```

### 3. Validate Critical Contract Fields

Don't just check status codes—validate the shape of the response:

```cpp
// ❌ BAD (too shallow)
.ExpectStatus(200)

// ✅ GOOD (contract validation)
.ExpectStatus(200)
.ExpectJSON("user.id", ExpectedUserID)
.ExpectJSON("user.permissions.canEdit", "true")
.ExpectHeader("X-API-Version", "v2")
```

### 4. Use Retries for Flaky Endpoints

External APIs can be unreliable—build resilience into your tests:

```cpp
FPalantirResponse Res = FPalantirRequest::Get("https://api.external-service.com/data")
    .WithRetry(2, 1.0f)  // Tolerate transient failures
    .WithTimeout(10.0f)
    .ExpectStatusRange(200, 299)
    .ExecuteBlocking();
```

### 5. Leverage Breadcrumbs for Debugging

Request/response lifecycle is automatically tracked:

```
[0.000s] HttpRequest: GET https://api.mygame.com/players/123
[0.234s] HttpResponse: 200 in 234.1ms
```

Use `FPalantirTrace::ExportToJSON()` to export full timeline for DataDog/ELK.

---

## Real-World Example: Multiplayer Backend Test

```cpp
NEXUS_TEST(FMultiplayerTest, "Multiplayer.Matchmaking.JoinLobby", ETestPriority::High)
{
    // Step 1: Create lobby
    FString CreateBody = TEXT("{\"gameMode\":\"deathmatch\",\"maxPlayers\":8}");
    FPalantirResponse CreateRes = FPalantirRequest::Post("https://api.mygame.com/lobbies", CreateBody)
        .WithHeader("Authorization", GetTestAuthToken())
        .ExpectStatus(201)
        .ExpectJSON("gameMode", "deathmatch")
        .ExpectJSON("currentPlayers", "1")  // Creator auto-joins
        .ExecuteBlocking();
    
    if (!CreateRes.IsSuccess())
    {
        UE_LOG(LogPalantirTrace, Error, TEXT("Failed to create lobby"));
        return false;
    }
    
    FString LobbyID = CreateRes.GetJSONValue("id");
    PALANTIR_BREADCRUMB(TEXT("LobbyCreated"), LobbyID);
    
    // Step 2: Second player joins
    FString JoinBody = TEXT("{}");
    FPalantirResponse JoinRes = FPalantirRequest::Post(
        FString::Printf(TEXT("https://api.mygame.com/lobbies/%s/join"), *LobbyID),
        JoinBody
    )
    .WithHeader("Authorization", GetSecondPlayerAuthToken())
    .ExpectStatus(200)
    .ExpectJSON("currentPlayers", "2")
    .ExecuteBlocking();
    
    if (!JoinRes.IsSuccess())
    {
        UE_LOG(LogPalantirTrace, Error, TEXT("Failed to join lobby"));
        return false;
    }
    
    PALANTIR_BREADCRUMB(TEXT("PlayerJoined"), LobbyID);
    
    // Step 3: Verify lobby state
    FPalantirResponse GetRes = FPalantirRequest::Get(
        FString::Printf(TEXT("https://api.mygame.com/lobbies/%s"), *LobbyID)
    )
    .ExpectStatus(200)
    .ExpectJSON("currentPlayers", "2")
    .ExpectJSON("maxPlayers", "8")
    .ExecuteBlocking();
    
    return GetRes.IsSuccess();
}
```

**Trace timeline output:**

```json
{
  "trace_id": "nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e",
  "breadcrumbs": [
    {"timestamp": "2025-12-07T14:32:15.000Z", "event": "TestStart", "data": "Multiplayer.Matchmaking.JoinLobby"},
    {"timestamp": "2025-12-07T14:32:15.234Z", "event": "HttpRequest", "data": "POST https://api.mygame.com/lobbies"},
    {"timestamp": "2025-12-07T14:32:15.456Z", "event": "HttpResponse", "data": "201 in 222.1ms"},
    {"timestamp": "2025-12-07T14:32:15.457Z", "event": "LobbyCreated", "data": "lobby_abc123"},
    {"timestamp": "2025-12-07T14:32:15.678Z", "event": "HttpRequest", "data": "POST https://api.mygame.com/lobbies/lobby_abc123/join"},
    {"timestamp": "2025-12-07T14:32:15.891Z", "event": "HttpResponse", "data": "200 in 213.4ms"},
    {"timestamp": "2025-12-07T14:32:15.892Z", "event": "PlayerJoined", "data": "lobby_abc123"},
    {"timestamp": "2025-12-07T14:32:16.123Z", "event": "HttpRequest", "data": "GET https://api.mygame.com/lobbies/lobby_abc123"},
    {"timestamp": "2025-12-07T14:32:16.234Z", "event": "HttpResponse", "data": "200 in 111.2ms"},
    {"timestamp": "2025-12-07T14:32:16.235Z", "event": "TestEnd", "data": "PASSED"}
  ]
}
```

---

## Comparison to Other Tools

| Feature | PalantírRequest | Cypress | Mocha/Chai | Unreal HTTP |
|---------|----------------|---------|------------|-------------|
| **Fluent API** | ✅ | ✅ | ✅ | ❌ |
| **Automatic Tracing** | ✅ | ❌ | ❌ | ❌ |
| **JSON Path Validation** | ✅ (dot notation) | ✅ | ✅ (with plugin) | ❌ |
| **GraphQL Support** | ✅ | ✅ | ✅ (with plugin) | ❌ |
| **Retry Logic** | ✅ | ✅ | ❌ | ❌ |
| **Async Support** | ✅ | ✅ | ✅ | ✅ |
| **Unreal Engine Native** | ✅ | ❌ | ❌ | ✅ |

---

## Further Reading

- **[PALANTIR.md](PALANTIR.md)** — Complete Palantír tracing guide
- **[PALANTIR_ARCHITECTURE.md](PALANTIR_ARCHITECTURE.md)** — Architectural overview
- **Unreal HTTP Module:** [Official Docs](https://docs.unrealengine.com/5.0/en-US/API/Runtime/HTTP/)
