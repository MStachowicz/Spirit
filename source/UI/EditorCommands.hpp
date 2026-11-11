#pragma once

// The Editor's command layer: the context commands run against, the commands themselves and the executor that every caller (menus,
// console, later MCP) goes through.
//
// Design decisions:
// - CommandExecutor is the only way to run a command, so the Play mode rule and undo grouping cannot be bypassed by any caller.
//   It checks Play mode when a command runs rather than when it is requested, so requests queued by MCP can't slip past it later.
// - The executor brackets each command with history.begin_command and end_command: everything a command records is one undo step, and
//   a command that fails has what it recorded undone. Commands report failure by returning fail, never by throwing: execute is noexcept,
//   so an exception from a command is treated as a bug and ends the program.
// - Undoable commands own their undo. Each declares a Change holding what one undo step stores, plus undo and redo next to execute
//   (see Utility::Is_Undoable), so a command's edit and its undo can be read in one place. A command making the same kind of edit as
//   another may record that command's Change, e.g. a multi-entity reset recording a ResetScale::Change per entity.
// - The text registry is an explicit list in EditorCommands.cpp rather than static self-registration. Self-registering objects in a
//   static library are discarded by the linker when nothing references their translation unit, and one list is easy to scan.
//   Typed callers never need the registry, it only exists for text lookup (console, MCP, a command palette).
// - These types live in the Editor, not next to the systems they edit, so engine code never depends on commands or undo history.

#include "ECS/Storage.hpp"
#include "Utility/Command.hpp"
#include "Utility/UndoRedo.hpp"

#include "glm/vec3.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

namespace System
{
	class SceneSystem;
}
namespace UI
{
	class Editor;
	struct EditorContext;
	using EditorHistory = Utility::UndoRedo<EditorContext>;

	// Everything commands may touch. Exists so they depend on a small explicit surface rather than the whole Editor.
	// Owned by the Editor and handed to every command by the CommandExecutor, and to their undo and redo functions by EditorHistory.
	struct EditorContext
	{
		Editor& editor;
		System::SceneSystem& scene_system;
		EditorHistory& history;

		// The entities of the scene being edited. Resolved on every call as the current scene changes on load and on entering Play.
		ECS::Storage& entities() const;
	};

	// Every command the Editor provides. See Utility::Is_Command for the shape of a command and Utility::Is_Undoable for undo support.
	// Functions are defined in EditorCommands.cpp to keep this header light.
	namespace Commands
	{
		struct Undo
		{
			static constexpr Utility::CommandInfo info{"edit.undo", "Undo", "Undo the last edit.", Utility::Access::Mutating};
			static Utility::CommandResult execute(EditorContext& p_context);
		};
		struct Redo
		{
			static constexpr Utility::CommandInfo info{"edit.redo", "Redo", "Redo the last undone edit.", Utility::Access::Mutating};
			static Utility::CommandResult execute(EditorContext& p_context);
		};
		struct SetDarkMode
		{
			static constexpr Utility::CommandInfo info{"editor.theme.dark_mode", "Dark mode", "Switch between the dark and light editor theme.", Utility::Access::Mutating};
			struct Args
			{
				bool enabled;
				static constexpr auto fields = std::tuple{Utility::Field{"enabled", &Args::enabled, "true for the dark theme, false for light."}};
			};
			// The theme before and after the switch.
			struct Change
			{
				bool before;
				bool after;
			};
			static Utility::CommandResult execute(EditorContext& p_context, const Args& p_args);
			static void undo(EditorContext& p_context, const Change& p_change);
			static void redo(EditorContext& p_context, const Change& p_change);
		};
		struct ResetScale
		{
			static constexpr Utility::CommandInfo info{"entity.transform.reset_scale", "Reset scale", "Set an entity's scale to 1 on every axis.", Utility::Access::Mutating};
			struct Args
			{
				EntityID entity;
				static constexpr auto fields = std::tuple{Utility::Field{"entity", &Args::entity, "ID of an entity with a Transform."}};
			};
			// One entity's scale before and after the reset. Stores the EntityID rather than the Transform, as the Transform can move in
			// memory or be deleted before the step is undone.
			struct Change
			{
				EntityID entity;
				glm::vec3 before;
				glm::vec3 after;
			};
			static Utility::CommandResult execute(EditorContext& p_context, const Args& p_args);
			// undo and redo skip an entity that was deleted or lost its Transform after the step was recorded.
			static void undo(EditorContext& p_context, const Change& p_change);
			static void redo(EditorContext& p_context, const Change& p_change);
		};
		// Shared by F2 and the viewport's Save as PNG action. Capturing does not edit authored state or add undo history, so full-window
		// screenshots remain available in Play mode. The Editor rejects viewport-only captures while its viewport is not being rendered.
		struct TakeScreenshot
		{
			static constexpr Utility::CommandInfo info{"editor.screenshot", "Take screenshot", "Save the full window or editor viewport as a PNG.", Utility::Access::ReadOnly};
			// Makes the capture target explicit for typed callers and future text clients without separate commands for each UI action.
			struct Args
			{
				bool include_ui;
				static constexpr auto fields = std::tuple{Utility::Field{"include_ui", &Args::include_ui, "true for the full window including UI, false for the editor viewport only."}};
			};
			static Utility::CommandResult execute(EditorContext& p_context, const Args& p_args);
		};
	} // namespace Commands

	// Runs commands against the EditorContext. The single execution path for menus, the console and later MCP, so policy applies
	// wherever a command came from. Owned by the Editor.
	class CommandExecutor
	{
	public:
		explicit CommandExecutor(EditorContext& p_context) noexcept
			: m_context{p_context}
		{}

		// Runs a command that declares no Args, e.g. execute<Commands::Undo>().
		template <typename Command>
			requires Utility::Is_Command<Command, EditorContext> && (!Utility::Has_Args<Command>)
		Utility::CommandResult execute() noexcept
		{
			return execute<Command>(Utility::NoArgs{});
		}
		// Runs Command as one undo step. If it fails, what it recorded is undone, so failed commands leave no partial edits.
		// Mutating commands are rejected in Play mode.
		// noexcept as commands report failure by returning fail. An exception is a bug, so it ends the program here rather than letting a
		// caller catch it and carry on with the history left mid-command.
		//@param p_args The command's arguments, e.g. execute<Commands::ResetScale>({.entity = id}).
		template <typename Command>
			requires Utility::Is_Command<Command, EditorContext>
		Utility::CommandResult execute(const Utility::Args_Of<Command>& p_args) noexcept
		{
			if (auto rejection = check_allowed(Command::info))
				return std::move(*rejection);

			const std::size_t mark = m_context.history.begin_command();
			Utility::CommandResult result = Utility::invoke_command<Command>(m_context, p_args);
			m_context.history.end_command(m_context, mark, result.success);
			return result;
		}
		// Looks up a registered command by id, parses its arguments and runs it. For the console and MCP.
		//@param p_line The command id followed by its space separated name=value arguments, e.g. "entity.transform.reset_scale entity=4".
		Utility::CommandResult execute(std::string_view p_line) noexcept;

	private:
		// Returns a failed result if the command may not run in the Editor's current state, e.g. Mutating commands in Play mode.
		// Not a template so the Editor only needs to be complete in EditorCommands.cpp.
		//@param p_info The command's info.
		std::optional<Utility::CommandResult> check_allowed(const Utility::CommandInfo& p_info) const;

		EditorContext& m_context;
	};

	// A type erased command that can be run from text. Exists so commands can be listed and looked up by id without knowing their types.
	// Used by CommandExecutor::execute(std::string_view) and the console's completion list, later MCP and a command palette.
	struct CommandEntry
	{
		Utility::CommandInfo info;
		Utility::CommandResult (*execute_text)(CommandExecutor& p_executor, std::string_view p_args);
	};
	// Every command that can be run from text. To register a command add it to the list in EditorCommands.cpp.
	std::span<const CommandEntry> command_registry();
} // namespace UI
