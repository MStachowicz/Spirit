#pragma once

// Generic building blocks for commands: named, described operations that can be run from typed code (menus, tools) or from text
// (console, later MCP).
//
// Design decisions:
// - A command is a plain struct with a static CommandInfo, an optional Args struct and a static execute function. No base class,
//   virtual calls or allocation, and typed callsites get compile-time checked arguments.
// - Arguments are an aggregate Args struct with a constexpr fields table rather than function parameters. The table names and
//   describes each argument in one place, which is what text parsing, help and MCP input schemas need, while typed callsites stay
//   readable with designated initialisers: execute<ResetScale>({.entity = id}). Revisit once C++26 reflection is usable.
// - Commands without arguments just omit Args. Args_Of and invoke_command hide the difference so nobody writes an empty struct.
// - Nothing here knows about the Editor, undo or Play mode. The Context is a template parameter and policy lives in the executor
//   that owns it. This keeps Utility free of UI dependencies and lets parsing be unit tested without a window.
// - Text parsing has one parse_value overload per argument type, not per command, so adding a command adds no parsing code.
//   Every field is required so a typo can't silently default an argument, e.g. an entity ID to 0.

#include <array>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>

namespace Utility
{
	// Whether a command changes state. Executors use it to reject Mutating commands when edits are not allowed, e.g. in Play mode.
	enum class Access : std::uint8_t
	{
		ReadOnly,
		Mutating
	};

	// Static description of a command. Exists so commands can be found, listed and described without knowing their types:
	// the console looks commands up by id, menus and a command palette show the name, help text and MCP show the description.
	struct CommandInfo
	{
		std::string_view id;          // Stable lookup key for the console and MCP, e.g. "entity.transform.reset_scale". Renaming breaks scripts.
		std::string_view name;        // Human readable, for menus and a command palette.
		std::string_view description; // One line, for help text and MCP tool descriptions.
		Access access;
	};

	// The outcome of running a command. Exists so every caller gets the same feedback: the console prints the message, MCP returns it
	// and executors only keep a command's edits if it succeeded.
	// Owns its message as commands usually build it with std::format, a string_view would dangle.
	struct CommandResult
	{
		bool success = true;
		std::string message;

		static CommandResult ok(std::string p_message = {}) { return {true, std::move(p_message)}; }
		static CommandResult fail(std::string p_message)    { return {false, std::move(p_message)}; }
	};

	// Describes one data member of a command's Args struct. Used by parse_args to map "name=value" text onto the member, and later
	// by help text and MCP input schemas to describe it.
	// e.g. Field{"entity", &Args::entity, "Entity ID"}, the template arguments are deduced.
	template <typename Class, typename T>
	struct Field
	{
		std::string_view name;
		T Class::* member;
		std::string_view description;
	};

	// Stands in for the Args of commands that declare none, so generic code can treat every command as taking an Args struct.
	struct NoArgs
	{
		static constexpr auto fields = std::tuple{};
	};

	template <typename Command>
	concept Has_Args = requires { typename Command::Args; };

	namespace detail
	{
		template <typename Command>
		struct Args_Of { using type = NoArgs; };
		template <Has_Args Command>
		struct Args_Of<Command> { using type = typename Command::Args; };

		// Calls p_function(index, field) for every Field in a fields tuple, in order.
		//@param p_fields An Args::fields tuple.
		//@param p_function Invoked as p_function(std::size_t, const Field<Args, T>&).
		template <typename Fields, typename Function>
		constexpr void for_each_field([[maybe_unused]] const Fields& p_fields, [[maybe_unused]] Function&& p_function)
		{
			[&]<std::size_t... Indices>(std::index_sequence<Indices...>)
			{
				(p_function(Indices, std::get<Indices>(p_fields)), ...);
			}(std::make_index_sequence<std::tuple_size_v<Fields>>{});
		}
	} // namespace detail

	// The Args struct of Command, or NoArgs if it declares none.
	template <typename Command>
	using Args_Of = typename detail::Args_Of<Command>::type;

	// A command is a struct providing:
	//   static constexpr CommandInfo info{...};
	//   struct Args { ...; static constexpr auto fields = std::tuple{Field{...}, ...}; }; // Optional.
	//   static CommandResult execute(Context&, const Args&);                              // Or execute(Context&) without Args.
	template <typename Command, typename Context>
	concept Is_Command = std::convertible_to<decltype(Command::info), CommandInfo>
		&& ((Has_Args<Command> && requires(Context& p_context, const Args_Of<Command>& p_args)
			{
				{ Command::execute(p_context, p_args) } -> std::same_as<CommandResult>;
			})
		|| (!Has_Args<Command> && requires(Context& p_context)
			{
				{ Command::execute(p_context) } -> std::same_as<CommandResult>;
			}));

	// Calls Command::execute, passing p_args only if the command declares Args. Applies no policy, executors call this.
	//@param p_context What the command operates on, e.g. the EditorContext.
	//@param p_args The command's arguments, NoArgs for commands without Args.
	template <typename Command, typename Context>
		requires Is_Command<Command, Context>
	CommandResult invoke_command(Context& p_context, [[maybe_unused]] const Args_Of<Command>& p_args)
	{
		if constexpr (Has_Args<Command>)
			return Command::execute(p_context, p_args);
		else
			return Command::execute(p_context);
	}

	// Converts the text of one argument value. Add an overload to support a new argument type in every command.
	// Floating point is not supported yet as some standard libraries (Apple libc++) lack floating point std::from_chars.
	//@param p_text The value text, e.g. "true" from "enabled=true".
	//@param p_out Receives the value, unspecified if parsing fails.
	//@return Whether all of p_text was a valid value.
	inline bool parse_value(std::string_view p_text, bool& p_out)
	{
		if (p_text == "true" || p_text == "1")
		{
			p_out = true;
			return true;
		}
		if (p_text == "false" || p_text == "0")
		{
			p_out = false;
			return true;
		}
		return false;
	}
	template <std::integral T>
	bool parse_value(std::string_view p_text, T& p_out)
	{
		const char* end = p_text.data() + p_text.size();
		const auto [last_parsed, error] = std::from_chars(p_text.data(), end, p_out);
		return error == std::errc{} && last_parsed == end;
	}

	// Parses space separated "name=value" pairs into p_args using Args::fields. Pairs may be in any order and every field is required.
	//@param p_text The arguments without the command id, e.g. "entity=4 enabled=true".
	//@param p_args Receives the parsed values, partially written if parsing fails.
	//@return A message describing the first problem found, or nullopt on success.
	template <typename Args>
	std::optional<std::string> parse_args(std::string_view p_text, Args& p_args)
	{
		[[maybe_unused]] std::array<bool, std::tuple_size_v<decltype(Args::fields)>> provided{};

		while (!p_text.empty())
		{
			const auto space = p_text.find(' ');
			const std::string_view pair = p_text.substr(0, space);
			p_text = space == std::string_view::npos ? std::string_view{} : p_text.substr(space + 1);
			if (pair.empty()) // Repeated spaces.
				continue;

			const auto equals = pair.find('=');
			if (equals == std::string_view::npos)
				return std::format("Expected name=value, got '{}'.", pair);

			const std::string_view name  = pair.substr(0, equals);
			const std::string_view value = pair.substr(equals + 1);
			bool found = false;
			bool valid = false;
			detail::for_each_field(Args::fields, [&](std::size_t p_index, const auto& p_field)
			{
				if (p_field.name == name)
				{
					found = true;
					valid = parse_value(value, p_args.*(p_field.member));
					provided[p_index] = true;
				}
			});
			if (!found)
				return std::format("Unknown argument '{}'.", name);
			if (!valid)
				return std::format("Invalid value '{}' for '{}'.", value, name);
		}

		std::optional<std::string> error;
		detail::for_each_field(Args::fields, [&](std::size_t p_index, const auto& p_field)
		{
			if (!error && !provided[p_index])
				error = std::format("Missing argument '{}'.", p_field.name);
		});
		return error;
	}
} // namespace Utility
