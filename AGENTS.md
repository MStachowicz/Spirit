# Project Architecture Guidelines

## Editor and Game Boundary

Treat the Editor as the game-authoring layer, separate in responsibility from the game engine that runs the simulation. The Editor provides user-authored edits, scene/level authoring workflows (including saving and loading), and tools such as the inspector, console, command registry, undo history, and planned MCP interface. Engine systems provide runtime capabilities such as ECS, scenes, physics, input, rendering, and assets.

Keep the dependency direction one-way: the Editor may use engine services, but engine systems must not depend on Editor classes, UI, commands, or undo history. `Application` may construct and connect both layers. This is the intended architecture; current code has transitional dependencies, so address them incrementally rather than broadening them.

Do not split executables or compile the Editor, command system, or MCP server out of builds as part of routine changes. That is a possible future distribution goal, not a current requirement. Keep ownership and dependencies compatible with that direction without adding build configuration prematurely.

## Commands, MCP, and Simulation

- Treat commands as requests to perform authoring operations, not as the mechanism for per-frame simulation updates.
- Keep command execution, the registry, MCP request queue, and undo history under Editor ownership. Route external requests through one execution path so validation and mode restrictions cannot be bypassed.
- Reject mutating commands while the Editor is in Play mode, regardless of whether they came from the UI, console, or MCP. Check the mode when executing a queued request, not only when receiving it. Read-only requests may be allowed during Play.
- Process queued MCP requests independently of drawing the Editor UI; the UI is hidden during Play, but requests still need a defined response.
- Simulation systems must not record undo history. Record user-authored changes made through Editor tools and commands. Undo and redo are themselves mutating commands but must not create new history entries.
- Keep stable command IDs and typed arguments suitable for console and MCP use. Avoid making engine systems depend on command-registry details.

## Undo and Scene Ownership

The Editor owns the undo history because it represents user intent. Store before/after state and the target identity needed to restore edits; do not make engine simulation systems write to the history.

Keep scene identity/lifetime in mind when recording entity edits. Clear or otherwise invalidate history when loading/replacing a scene, and do not replay an edit against an unrelated current scene. Play-mode simulation changes are not authoring edits and must not enter the Editor's history.

## Adding a Command

The generic pieces live in [Command.hpp](source/Utility/Command.hpp) and the history in [UndoRedo.hpp](source/Utility/UndoRedo.hpp). Editor commands, `CommandExecutor` and the text registry live in [EditorCommands.hpp](source/UI/EditorCommands.hpp) and [EditorCommands.cpp](source/UI/EditorCommands.cpp). Follow the existing `ResetScale` command as a complete example. MCP integration is planned, not implemented yet.

1. **Declare the command** as a plain struct in `UI::Commands` in `EditorCommands.hpp`, and define its functions in `EditorCommands.cpp`:

   ```cpp
   struct ResetRotation
   {
       static constexpr Utility::CommandInfo info{"entity.transform.reset_rotation", "Reset rotation", "Clear an entity's rotation.", Utility::Access::Mutating};
       struct Args // Omit entirely for commands without arguments, do not write an empty struct.
       {
           EntityID entity;
           static constexpr auto fields = std::tuple{Utility::Field{"entity", &Args::entity, "ID of an entity with a Transform."}};
       };
       struct Change // What one undo step stores. Omit, with undo and redo, for commands that record nothing.
       {
           EntityID entity;
           glm::quat before;
           glm::quat after;
       };
       static Utility::CommandResult execute(EditorContext& p_context, const Args& p_args); // execute(EditorContext&) without Args.
       static void undo(EditorContext& p_context, const Change& p_change);
       static void redo(EditorContext& p_context, const Change& p_change);
   };
   ```

   - `info.id` is the stable name used by the console and MCP, in the form `area.object.action`. Renaming it breaks scripts.
   - Use `Access::Mutating` for anything that changes state, so the executor rejects it in Play mode. Only `Access::ReadOnly` commands may run during Play.
   - List every `Args` member in `fields`; text input requires all of them. Members must have a `parse_value` overload in `Command.hpp`, currently `bool` and integers (including `EntityID`). Add an overload there to support a new argument type in every command.
   - Declaring `Change`, `undo` and `redo` is what makes a command undoable: `history.record<Command>` requires all three (`Utility::Is_Undoable`).
2. **Write `execute`:**
   - Validate arguments first and return `CommandResult::fail` with a message. Console and MCP input is untrusted; for example, check `has_components` before using an entity ID.
   - If the request changes nothing (for example, the rotation is already identity), return `CommandResult::ok()` without recording, so the user has no empty undo step.
   - Otherwise record a Change, then make the edit yourself: `p_context.history.record<ResetRotation>({.entity = p_args.entity, .before = transform.m_orientation, .after = glm::identity<glm::quat>()})`. `record` only stores the Change; it never applies it. `Access::Mutating` enforces policy; it does not automatically record undo. Undo and redo themselves move through history without adding entries.
   - Return `CommandResult::ok()` on success. Do not group records yourself: the executor makes everything a command records one undo step, and returning `fail` undoes what it recorded. Unrecorded side effects cannot be rolled back by the history.
   - Report every failure by returning `fail`, never by throwing. `CommandExecutor::execute` is `noexcept`, so an exception from a command is treated as a bug and ends the program. Where an expected failure throws, such as file IO, catch it inside `execute` and return `fail`.
3. **Write `undo` and `redo`**, which restore a Change's `before` and `after` values. The history calls them on undo and redo, and calls `undo` to roll back a failed command. Their rules:
   - `redo` must reproduce the recorded result from the Change. Do not redo by running `execute` again: it re-validates and can produce a different result, such as a new entity ID.
   - They must not throw or record history (recording is asserted), and must skip a target that no longer exists.
   - Repeat every side effect of the edit in `execute`, for example resyncing physics or restyling, as only `undo` and `redo` run on undo and redo.
   - Change members are serialised into history, so store IDs and values, not pointers or references. Use `EntityID`, not `ECS::Entity`.
   - A Change must not have constructors or default member initialisers, as `Serialise.hpp` only copies trivial types as raw bytes. Give it `serialise` and `deserialise` if it must hold non-trivial values such as a whole `Transform`.
   - A command making the same kind of edit as another may record that command's Change instead of declaring its own, for example a multi-entity reset recording a `ResetScale::Change` per entity.
4. **Register the command** in the `make_registry<...>` list in `EditorCommands.cpp`, so the console (and later MCP) can find it by id. Typed callers do not need registration.
5. **Call it through the executor:** `m_commands.execute<Commands::ResetRotation>({.entity = id})`, or `execute<Command>()` without Args. The console form is `entity.transform.reset_rotation entity=4`. Never call `Command::execute` directly, as that bypasses the Play-mode check and undo grouping. Surface failed results to the caller.
6. **Interactive tools** that preview every frame, such as gizmo drags and sliders, must not run a command per frame. When wiring these tools into history, capture the before state when the interaction starts, then record one Change when it ends, for example `history.record<ResetRotation>({.entity = id, .before = before, .after = after})`. Outside a command each record is its own undo step. `record` only stores the Change; it does not call `undo` or `redo` or run their side effects. Keep previews subject to the same Play-mode and scene-lifetime rules.
7. **Tests:** cover new parsing or concept behaviour in [CommandTester.cpp](source/Test/Tests/CommandTester.cpp) and new history behaviour in [UndoRedoTester.cpp](source/Test/Tests/UndoRedoTester.cpp), explaining why each case exists. The Test target does not currently link UI; verify Editor commands through typed and console calls, including invalid input, no-op edits, undo/redo and Play-mode rejection for mutating commands.

## Deferred Architecture Work

These are staged follow-ups, not requirements to perform all at once:

1. **Remove reverse dependencies:** replace Logger's direct Editor sink with a generic callback or equivalent interface, so Utility no longer needs to link against UI.
2. **Include inspector edits in undo:** component and geometry `draw_UI` methods currently mutate data directly. Add an Editor-side way to capture a before/after snapshot and group continuous interactions such as slider drags into one undo step.
3. **Clarify Editor-owned state:** move Editor presentation preferences such as theme out of platform-wide/global state where practical; pass engine-facing values such as clear color into the renderer rather than having the renderer read Editor theme state.
4. **Improve separation when justified:** consider extracting a non-ImGui authoring/session layer if command, undo, or MCP tests need to run without constructing a window and renderer. Do not introduce this split solely in anticipation.
5. **Consider build separation later:** if distribution needs it, make the Editor and MCP optional targets while retaining Debug and optimized configurations for both editor and game builds.

When touching a transitional dependency, avoid adding new engine-to-Editor dependencies and prefer changes that move the code toward the boundary above. Do not undertake unrelated migrations just to make the architecture perfectly separated in one change.
