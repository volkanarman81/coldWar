# Modifications

This repository is a modified version of the Bohemia Interactive source release
[BohemiaInteractive/CWR](https://github.com/BohemiaInteractive/CWR) (release 3.05,
commit `ffc6183`). It is not the original program and is not affiliated with or
endorsed by Bohemia Interactive. The changes are listed below, as the GPL and the
Section 7 additional terms in `LICENSE` require.

## 2026-09-27: `saveString` / `loadString` script commands

Two new scripting commands let missions keep plain text on disk between sessions.

| Command | Syntax | Returns |
|---------|--------|---------|
| `saveString` | `"name" saveString "text"` | `true` if written, `false` on an invalid name, oversized text or a write error |
| `loadString` | `loadString "name"` | the saved text, or `""` if there is no such save |

```sqf
// save
_ok = "campaign_state.txt" saveString format ["%1", [score player, daytime]];

// load
_raw = loadString "campaign_state.txt";
if (_raw != "") then { _state = call _raw };
```

Rules:

- Files are stored in the player's profile directory under `Saves/`, e.g.
  `<user dir>/Saves/campaign_state.txt`. Saving again replaces the old content.
- `name` must be a plain file name: 1 to 64 characters from `A-Z a-z 0-9 _ - .`,
  not starting with `.`. Windows device names such as `con` or `nul.txt` are
  rejected. Any path separator, drive letter or other character is rejected.
- Text is limited to 1 MiB per file, and a file larger than that loads as `""`.
- Every mission uses the same `Saves/` folder, so pick distinctive names.
- In multiplayer only the server keeps saves: the dedicated server or the player
  hosting the game. On a client connected to someone else's game, `saveString`
  returns `false` and `loadString` returns `""`. Single player is not affected.

Changed files:

- `engine/Poseidon/Game/Commands/GameStateExtWorldConfig.cpp`: implementation
  (`SavedStringFullName`, `StringSave`, `StringLoadSaved`)
- `engine/Poseidon/Game/Commands/GameStateExt.cpp`: command registration
- `engine/Evaluator/Validate.cpp`: the standalone validator skips these commands
- `tests/unit/engine/Poseidon/Game/test_game_state_ext.cpp`: unit tests

## 2026-09-27: `date`, `overcast`, `fog`, `rain` getters

The engine had `setDate`, `setOvercast`, `setFog` and `setRain` but no way to read
the current values back, so world state could not be saved. New commands:

| Command | Returns |
|---------|---------|
| `date` | `[year, month, day, hour, minute]`, the same layout `setDate` takes |
| `overcast` | current overcast, 0..1 |
| `fog` | current fog, 0..1 |
| `rain` | current rain density, 0..1 |

A mission variable with one of these names still takes precedence over the command,
so older missions that use them as variable names keep working.

Changed files:

- `engine/Poseidon/Game/Commands/GameStateExtWorldConfig.cpp`: `GetDate`, `GetOvercast`,
  `GetFog`, `GetRain`
- `engine/Poseidon/Game/Commands/GameStateExt.cpp`: command registration
- `engine/Poseidon/World/World.hpp`: `GetActualOvercast()` / `GetActualFog()` accessors
- `tests/unit/engine/Poseidon/Game/test_game_state_ext.cpp`: registration checks
