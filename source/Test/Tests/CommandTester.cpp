// Unit tests for Utility/Command.hpp.
//
// Commands here run against a small Counter context instead of the Editor so the generic pieces can be tested without a window.
// Most tests cover parse_args, as it handles untrusted text typed into the console or sent over MCP and must reject anything that
// would otherwise be silently misread. Editor command metadata is also tested without constructing or linking the Editor.

#include "CommandTester.hpp"

#include "UI/EditorCommands.hpp"
#include "Utility/Command.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Test
{
	namespace
	{
		// The Context commands run against, standing in for the EditorContext.
		struct Counter
		{
			int value = 0;
		};

		struct Increment // A command without Args.
		{
			static constexpr Utility::CommandInfo info{"test.increment", "Increment", "Add one to the counter.", Utility::Access::Mutating};
			static Utility::CommandResult execute(Counter& p_counter)
			{
				++p_counter.value;
				return Utility::CommandResult::ok();
			}
		};
		struct Add // A command with Args covering every type parse_value supports.
		{
			static constexpr Utility::CommandInfo info{"test.add", "Add", "Add an amount to the counter.", Utility::Access::Mutating};
			struct Args
			{
				int amount;
				std::size_t times;
				bool report;
				static constexpr auto fields = std::tuple{
					Utility::Field{"amount", &Args::amount, "Signed amount to add."},
					Utility::Field{"times",  &Args::times,  "How many times to add it."},
					Utility::Field{"report", &Args::report, "Whether to report the new value."}};
			};
			static Utility::CommandResult execute(Counter& p_counter, const Args& p_args)
			{
				p_counter.value += p_args.amount * static_cast<int>(p_args.times);
				return Utility::CommandResult::ok(p_args.report ? std::to_string(p_counter.value) : std::string{});
			}
		};

		// Compile time checks that Is_Command accepts both command shapes and rejects the mistakes it is meant to catch, so they fail
		// where the command is registered or executed rather than deep inside the executor.
		struct MissingInfo
		{
			static Utility::CommandResult execute(Counter&) { return {}; }
		};
		struct WrongReturnType
		{
			static constexpr Utility::CommandInfo info{"test.wrong_return", "", "", Utility::Access::ReadOnly};
			static bool execute(Counter&) { return true; }
		};
		struct IgnoresArgs
		{
			static constexpr Utility::CommandInfo info{"test.ignores_args", "", "", Utility::Access::ReadOnly};
			struct Args
			{
				int value;
				[[maybe_unused]] static constexpr auto fields = std::tuple{Utility::Field{"value", &Args::value, ""}};
			};
			static Utility::CommandResult execute(Counter&) { return {}; }
		};

		static_assert(Utility::Is_Command<Increment, Counter>);
		static_assert(Utility::Is_Command<Add, Counter>);
		static_assert(!Utility::Is_Command<Increment, int>,             "execute must accept the Context");
		static_assert(!Utility::Is_Command<MissingInfo, Counter>,       "Commands must provide a CommandInfo");
		static_assert(!Utility::Is_Command<WrongReturnType, Counter>,   "execute must return a CommandResult");
		static_assert(!Utility::Is_Command<IgnoresArgs, Counter>,       "Commands declaring Args must accept them in execute");
		static_assert(std::is_same_v<Utility::Args_Of<Increment>, Utility::NoArgs>, "Commands without Args take NoArgs");
		static_assert(std::is_same_v<Utility::Args_Of<Add>, Add::Args>,              "Commands with Args take their Args");

		// Captures must use the shared executor without being classified as authored edits, so F2 still works in Play mode.
		static_assert(Utility::Is_Command<UI::Commands::TakeScreenshot, UI::EditorContext>);
		static_assert(UI::Commands::TakeScreenshot::info.access == Utility::Access::ReadOnly);

		// Parses p_text into a fresh Add::Args.
		//@param p_text The arguments without the command id.
		//@return The parsed Args and the error message, if any.
		std::pair<Add::Args, std::optional<std::string>> parse_add(std::string_view p_text)
		{
			Add::Args args{};
			auto error = Utility::parse_args(p_text, args);
			return {args, std::move(error)};
		}
		bool mentions(const std::optional<std::string>& p_error, std::string_view p_text)
		{
			return p_error.has_value() && p_error->find(p_text) != std::string::npos;
		}
	} // namespace

	void CommandTester::run_unit_tests()
	{SCOPE_SECTION("Command")
		// Executors run every command through invoke_command. Commands without Args must still be callable with NoArgs, and Args must
		// reach execute unchanged.
		{SCOPE_SECTION("invoke_command")
			Counter counter;
			const auto increment = Utility::invoke_command<Increment>(counter, Utility::NoArgs{});
			CHECK_TRUE(increment.success, "A command without Args runs");
			CHECK_EQUAL(counter.value, 1, "A command without Args edits the context");

			const auto add = Utility::invoke_command<Add>(counter, {.amount = 2, .times = 3, .report = true});
			CHECK_TRUE(add.success, "A command with Args runs");
			CHECK_EQUAL(counter.value, 7, "Args reach execute");
			CHECK_EQUAL(add.message, std::string("7"), "The result message is returned");
		}
		// Executors only commit a command's undo Group if it succeeded, and the console colours output by success, so ok and fail
		// must set success correctly and keep their message.
		{SCOPE_SECTION("CommandResult")
			CHECK_TRUE(Utility::CommandResult::ok().success, "ok succeeds");
			CHECK_TRUE(Utility::CommandResult::ok().message.empty(), "ok has no message by default");
			CHECK_EQUAL(Utility::CommandResult::ok("done").message, std::string("done"), "ok keeps its message");
			CHECK_TRUE(!Utility::CommandResult::fail("bad").success, "fail does not succeed");
			CHECK_EQUAL(Utility::CommandResult::fail("bad").message, std::string("bad"), "fail keeps its message");
		}
		// Check the real command's argument metadata so typed UI actions and future text callers agree on which image is captured.
		// A missing or malformed target must be rejected rather than unexpectedly including editor UI in a viewport-only capture.
		{SCOPE_SECTION("Screenshot arguments")
			UI::Commands::TakeScreenshot::Args args{};
			CHECK_TRUE(!Utility::parse_args("include_ui=true", args).has_value(), "Full-window capture arguments parse");
			CHECK_TRUE(args.include_ui, "true includes editor UI");
			CHECK_TRUE(!Utility::parse_args("include_ui=false", args).has_value(), "Viewport-only capture arguments parse");
			CHECK_TRUE(!args.include_ui, "false excludes editor UI");
			CHECK_TRUE(mentions(Utility::parse_args("", args), "include_ui"), "The capture target is required");
			CHECK_TRUE(mentions(Utility::parse_args("include_ui=yes", args), "include_ui"), "Invalid capture flags are rejected");
			CHECK_TRUE(mentions(Utility::parse_args("viewport_only=true", args), "viewport_only"), "Unknown capture arguments are rejected");
		}
		// parse_args turns console and MCP text into a command's Args.
		{SCOPE_SECTION("parse_args")
			// Arguments are named rather than positional, so they must parse in any order.
			{SCOPE_SECTION("Any order")
				const auto [args, error] = parse_add("times=4 report=true amount=-3");
				CHECK_TRUE(!error.has_value(), "Valid arguments parse");
				CHECK_EQUAL(args.amount, -3, "Signed integers parse");
				CHECK_EQUAL(args.times, std::size_t{4}, "Unsigned integers parse");
				CHECK_TRUE(args.report, "true parses");
			}
			// Typed console input often has stray spaces, which should not be errors. Also checks the 1/0 spelling of bool.
			{SCOPE_SECTION("Extra spaces")
				const auto [args, error] = parse_add("  amount=1   times=2 report=0 ");
				CHECK_TRUE(!error.has_value(), "Extra spaces are ignored");
				CHECK_EQUAL(args.amount, 1, "First argument parses");
				CHECK_TRUE(!args.report, "0 parses as false");
			}
			// Every field is required, so a forgotten argument (e.g. an entity ID) cannot silently default to 0 and edit the wrong thing.
			{SCOPE_SECTION("Missing argument")
				const auto error = parse_add("amount=1 report=true").second;
				CHECK_TRUE(mentions(error, "times"), "The missing argument is reported");
			}
			// Misspelt argument names must be reported rather than ignored.
			{SCOPE_SECTION("Unknown argument")
				const auto error = parse_add("amount=1 times=1 report=true colour=red").second;
				CHECK_TRUE(mentions(error, "colour"), "The unknown argument is reported");
			}
			// Values must parse in full so input is rejected instead of being truncated ("12abc" as 12), wrapped ("-1" as a huge
			// unsigned ID) or guessed ("yes" as true).
			{SCOPE_SECTION("Invalid value")
				CHECK_TRUE(mentions(parse_add("amount=12abc times=1 report=true").second, "amount"), "Trailing characters are rejected");
				CHECK_TRUE(mentions(parse_add("amount=1 times=-1 report=true").second, "times"),     "Negative unsigned values are rejected");
				CHECK_TRUE(mentions(parse_add("amount=99999999999 times=1 report=true").second, "amount"), "Out of range values are rejected");
				CHECK_TRUE(mentions(parse_add("amount=1 times=1 report=yes").second, "report"),      "Only true, false, 1 and 0 are bools");
				CHECK_TRUE(mentions(parse_add("amount= times=1 report=true").second, "amount"),      "Empty values are rejected");
			}
			// Arguments must be name=value pairs, positional values are not supported.
			{SCOPE_SECTION("Missing equals")
				const auto error = parse_add("amount 1 times=1 report=true").second;
				CHECK_TRUE(mentions(error, "name=value"), "Pairs without '=' are reported");
			}
			// Commands without Args are run from text with NoArgs. No text must succeed, while any argument is an error so typos
			// in the command line aren't silently ignored.
			{SCOPE_SECTION("No arguments")
				Utility::NoArgs args;
				CHECK_TRUE(!Utility::parse_args("", args).has_value(), "Empty text parses");
				CHECK_TRUE(!Utility::parse_args("   ", args).has_value(), "Only spaces parses");
				CHECK_TRUE(mentions(Utility::parse_args("force=true", args), "force"), "Any argument is unknown");
			}
		}
	}
} // namespace Test
