// Definitions for the Editor's commands, and the list of commands that can be run from text.
// Kept out of EditorCommands.hpp so the header does not need the complete Editor, SceneSystem or component types.

#include "EditorCommands.hpp"
#include "Editor.hpp"

#include "Component/Transform.hpp"
#include "Platform/Core.hpp"
#include "System/SceneSystem.hpp"

#include <array>
#include <format>
#include <string>

namespace UI
{
	ECS::Storage& EditorContext::entities() const
	{
		return scene_system.get_current_scene_entities();
	}

	namespace Commands
	{
		Utility::CommandResult Undo::execute(EditorContext& p_context)
		{
			if (!p_context.history.can_undo())
				return Utility::CommandResult::fail("Nothing to undo.");

			p_context.history.undo(p_context);
			return Utility::CommandResult::ok();
		}
		Utility::CommandResult Redo::execute(EditorContext& p_context)
		{
			if (!p_context.history.can_redo())
				return Utility::CommandResult::fail("Nothing to redo.");

			p_context.history.redo(p_context);
			return Utility::CommandResult::ok();
		}

		Utility::CommandResult SetDarkMode::execute(EditorContext& p_context, const Args& p_args)
		{
			auto& theme = Platform::Core::s_theme;
			if (theme.dark_mode == p_args.enabled)
				return Utility::CommandResult::ok(); // Already in this theme: nothing changed, so nothing to undo.

			p_context.history.record<SetDarkMode>({.before = theme.dark_mode, .after = p_args.enabled});
			theme.dark_mode = p_args.enabled;
			p_context.editor.initialiseStyling();
			return Utility::CommandResult::ok();
		}
		void SetDarkMode::undo(EditorContext& p_context, const Change& p_change)
		{
			Platform::Core::s_theme.dark_mode = p_change.before;
			p_context.editor.initialiseStyling();
		}
		void SetDarkMode::redo(EditorContext& p_context, const Change& p_change)
		{
			Platform::Core::s_theme.dark_mode = p_change.after;
			p_context.editor.initialiseStyling();
		}

		Utility::CommandResult ResetScale::execute(EditorContext& p_context, const Args& p_args)
		{
			auto& entities = p_context.entities();
			if (!entities.has_components<Component::Transform>(p_args.entity))
				return Utility::CommandResult::fail(std::format("Entity {} does not exist or has no Transform.", p_args.entity));

			auto& transform = entities.get_component<Component::Transform>(p_args.entity);
			if (transform.m_scale == glm::vec3(1.f))
				return Utility::CommandResult::ok(); // Already 1: nothing changed, so nothing to undo.

			p_context.history.record<ResetScale>({.entity = p_args.entity, .before = transform.m_scale, .after = glm::vec3(1.f)});
			transform.m_scale = glm::vec3(1.f);
			return Utility::CommandResult::ok();
		}
		void ResetScale::undo(EditorContext& p_context, const Change& p_change)
		{
			auto& entities = p_context.entities();
			if (entities.has_components<Component::Transform>(p_change.entity))
				entities.get_component<Component::Transform>(p_change.entity).m_scale = p_change.before;
		}
		void ResetScale::redo(EditorContext& p_context, const Change& p_change)
		{
			auto& entities = p_context.entities();
			if (entities.has_components<Component::Transform>(p_change.entity))
				entities.get_component<Component::Transform>(p_change.entity).m_scale = p_change.after;
		}

		Utility::CommandResult TakeScreenshot::execute(EditorContext& p_context, const Args& p_args)
		{
			return p_context.editor.request_screenshot(p_args.include_ui);
		}
	} // namespace Commands

	std::optional<Utility::CommandResult> CommandExecutor::check_allowed(const Utility::CommandInfo& p_info) const
	{
		if (p_info.access == Utility::Access::Mutating && m_context.editor.is_playing())
			return Utility::CommandResult::fail(std::format("'{}' changes the scene and cannot run in Play mode.", p_info.id));

		return std::nullopt;
	}

	namespace
	{
		// The CommandEntry::execute_text of Command. Parses p_args into Command's Args and runs it through p_executor.
		//@param p_executor Runs the command so the same policy applies as for typed callers.
		//@param p_args The space separated name=value arguments, without the command id.
		template <typename Command>
		Utility::CommandResult execute_text(CommandExecutor& p_executor, std::string_view p_args)
		{
			Utility::Args_Of<Command> args{};
			if (auto error = Utility::parse_args(p_args, args))
				return Utility::CommandResult::fail(std::move(*error));

			return p_executor.execute<Command>(args);
		}
		template <typename... CommandTypes>
		constexpr auto make_registry()
		{
			return std::array{CommandEntry{CommandTypes::info, &execute_text<CommandTypes>}...};
		}

		// Add new commands here to make them available to the console and MCP.
		constexpr auto registry = make_registry<
			Commands::Undo,
			Commands::Redo,
			Commands::SetDarkMode,
			Commands::ResetScale,
			Commands::TakeScreenshot>();
	} // namespace

	std::span<const CommandEntry> command_registry()
	{
		return registry;
	}

	Utility::CommandResult CommandExecutor::execute(std::string_view p_line) noexcept
	{
		const auto space            = p_line.find(' ');
		const std::string_view id   = p_line.substr(0, space);
		const std::string_view args = space == std::string_view::npos ? std::string_view{} : p_line.substr(space + 1);

		for (const auto& entry : command_registry())
		{
			if (entry.info.id == id)
				return entry.execute_text(*this, args);
		}
		return Utility::CommandResult::fail(std::format("Unknown command '{}'.", id));
	}
} // namespace UI
