// Unit and performance tests for Utility::UndoRedo, the Editor's undo/redo history.
//
// The tests run the history against a small Document instead of the Editor so they need no window, renderer or scene, and so every
// kind of Change the history supports (trivial, custom serialised, holding containers) can be checked in isolation.
// Each undoable type below edits one value of the Document the way an Editor command does: the set_ helpers record a Change and then
// make the edit themselves. Document::replayed counts every undo and redo call, which is how the tests tell whether the history wrote
// to the live state.

#include "UndoRedoTester.hpp"

#include "Utility/Serialise.hpp"
#include "Utility/Stopwatch.hpp"
#include "Utility/UndoRedo.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace Test
{
	namespace
	{
		struct Colour // Trivially serialisable, written as raw bytes inside Profile.
		{
			float r, g, b, a;
			bool operator==(const Colour&) const = default;
		};
		struct Profile // Custom serialisation mixing containers with a trivially serialisable member.
		{
			std::string name;
			std::vector<int> scores;
			Colour colour;
			bool operator==(const Profile&) const = default;

			static void serialise(Utility::BinaryWriter& p_out, const Profile& p_profile)
			{
				p_out.write(p_profile.name);
				p_out.write(p_profile.scores);
				p_out.write(p_profile.colour);
			}
			static Profile deserialise(Utility::BinaryReader& p_in)
			{
				Profile profile{};
				p_in.read(profile.name);
				p_in.read(profile.scores);
				p_in.read(profile.colour);
				return profile;
			}
		};

		// The Context being edited, standing in for the Editor.
		struct Document
		{
			bool flag = false;
			std::array<int, 3> cells = {};
			Profile profile = {};
			std::vector<std::string> tags;
			int replayed = 0; // Number of undo and redo calls.
		};
		using History = Utility::UndoRedo<Document>;

		bool same_values(const Document& p_a, const Document& p_b)
		{
			return p_a.flag == p_b.flag && p_a.cells == p_b.cells && p_a.profile == p_b.profile && p_a.tags == p_b.tags;
		}

		// Trivially serialisable Change, recorded as raw bytes.
		struct SetFlag
		{
			struct Change
			{
				bool before;
				bool after;
			};
			static void undo(Document& p_document, const Change& p_change) { p_document.flag = p_change.before; ++p_document.replayed; }
			static void redo(Document& p_document, const Change& p_change) { p_document.flag = p_change.after;  ++p_document.replayed; }
		};
		// The Change identifies which value it edits, as an EntityID does in ResetScale::Change.
		struct SetCell
		{
			struct Change
			{
				std::size_t index;
				int before;
				int after;
			};
			static void undo(Document& p_document, const Change& p_change) { p_document.cells[p_change.index] = p_change.before; ++p_document.replayed; }
			static void redo(Document& p_document, const Change& p_change) { p_document.cells[p_change.index] = p_change.after;  ++p_document.replayed; }
		};
		// A Change holding non-trivial values needs custom serialisation, as one holding whole Transforms would.
		struct SetProfile
		{
			struct Change
			{
				Profile before;
				Profile after;

				static void serialise(Utility::BinaryWriter& p_out, const Change& p_change)
				{
					p_out.write(p_change.before);
					p_out.write(p_change.after);
				}
				static Change deserialise(Utility::BinaryReader& p_in)
				{
					Change change{};
					p_in.read(change.before);
					p_in.read(change.after);
					return change;
				}
			};
			static void undo(Document& p_document, const Change& p_change) { p_document.profile = p_change.before; ++p_document.replayed; }
			static void redo(Document& p_document, const Change& p_change) { p_document.profile = p_change.after;  ++p_document.replayed; }
		};
		// A Change holding containers, whose size varies between records.
		struct SetTags
		{
			struct Change
			{
				std::vector<std::string> before;
				std::vector<std::string> after;

				static void serialise(Utility::BinaryWriter& p_out, const Change& p_change)
				{
					p_out.write(p_change.before);
					p_out.write(p_change.after);
				}
				static Change deserialise(Utility::BinaryReader& p_in)
				{
					Change change{};
					p_in.read(change.before);
					p_in.read(change.after);
					return change;
				}
			};
			static void undo(Document& p_document, const Change& p_change) { p_document.tags = p_change.before; ++p_document.replayed; }
			static void redo(Document& p_document, const Change& p_change) { p_document.tags = p_change.after;  ++p_document.replayed; }
		};

		// Make an edit the way a command does: record a Change describing it, then apply it to the live state.
		void set_flag(History& p_history, Document& p_document, bool p_flag)
		{
			p_history.record<SetFlag>({.before = p_document.flag, .after = p_flag});
			p_document.flag = p_flag;
		}
		void set_cell(History& p_history, Document& p_document, std::size_t p_index, int p_value)
		{
			p_history.record<SetCell>({.index = p_index, .before = p_document.cells[p_index], .after = p_value});
			p_document.cells[p_index] = p_value;
		}
		// Changes holding heap-owning values are built by moving the old and new values in, then the new value is moved back out after
		// recording. record only reads the Change, so this records without copying either value.
		void set_profile(History& p_history, Document& p_document, Profile p_profile)
		{
			SetProfile::Change change{.before = std::move(p_document.profile), .after = std::move(p_profile)};
			p_history.record<SetProfile>(change);
			p_document.profile = std::move(change.after);
		}
		void set_tags(History& p_history, Document& p_document, std::vector<std::string> p_tags)
		{
			SetTags::Change change{.before = std::move(p_document.tags), .after = std::move(p_tags)};
			p_history.record<SetTags>(change);
			p_document.tags = std::move(change.after);
		}

		// Compile time checks that Is_Undoable accepts well formed types and rejects the mistakes it is meant to catch, so they fail at
		// the record call with a constraint error rather than deep inside the history.
		struct MissingChange
		{
			static void undo(Document&, const bool&) {}
			static void redo(Document&, const bool&) {}
		};
		struct NotSerialisableChange
		{
			struct Change
			{
				std::string text; // Not trivial, and the Change provides no custom serialisation.
			};
			static void undo(Document&, const Change&) {}
			static void redo(Document&, const Change&) {}
		};
		struct MissingRedo
		{
			struct Change
			{
				bool value;
			};
			static void undo(Document&, const Change&) {}
		};
		struct FallibleUndo
		{
			struct Change
			{
				bool value;
			};
			static bool undo(Document&, const Change&) { return true; }
			static void redo(Document&, const Change&) {}
		};
		struct DefaultedChange
		{
			struct Change
			{
				bool value = false;
			};
			static void undo(Document&, const Change&) {}
			static void redo(Document&, const Change&) {}
		};

		static_assert(Utility::Is_Undoable<SetFlag, Document>);
		static_assert(Utility::Is_Undoable<SetCell, Document>);
		static_assert(Utility::Is_Undoable<SetProfile, Document>);
		static_assert(Utility::Is_Undoable<SetTags, Document>);
		static_assert(!Utility::Is_Undoable<SetFlag, Profile>,                "undo and redo must accept the Context");
		static_assert(!Utility::Is_Undoable<MissingChange, Document>,         "Undoable types must declare a Change");
		static_assert(!Utility::Is_Undoable<NotSerialisableChange, Document>, "The Change must be serialisable");
		static_assert(!Utility::Is_Undoable<MissingRedo, Document>,           "Undoable types must provide redo");
		static_assert(!Utility::Is_Undoable<FallibleUndo, Document>,          "undo must return void, as undoing cannot be refused");
		// Documents a Serialise.hpp limitation: default member initialisers make a type non-trivial, so a Change cannot use them.
		// Remove once Is_Trivially_Serializable accepts trivially copyable types.
		static_assert(!Utility::Is_Undoable<DefaultedChange, Document>,       "Default member initialisers are not serialisable");
	} // namespace

	void UndoRedoTester::run_unit_tests()
	{SCOPE_SECTION("UndoRedo")
		// Undo and redo are reachable from menus, the console and MCP at any time, so calling them with nothing recorded must be
		// a harmless no-op rather than an error or a write to the live state.
		{SCOPE_SECTION("Empty history")
			Document document;
			History history;
			CHECK_TRUE(!history.can_undo(), "Nothing to undo");
			CHECK_TRUE(!history.can_redo(), "Nothing to redo");

			history.undo(document);
			history.redo(document);
			CHECK_EQUAL(document.replayed, 0, "Undo and redo do not touch the document");
		}
		// Commands make their edit themselves and record a Change describing it. record must only store the Change, as applying it
		// again would repeat the edit's side effects, and undo and redo must restore the before and after values through the type.
		{SCOPE_SECTION("record")
			Document document;
			History history;
			set_flag(history, document, true);
			CHECK_EQUAL(document.replayed, 0, "record does not call undo or redo");
			CHECK_TRUE(history.can_undo(), "Can undo after recording");
			CHECK_TRUE(!history.can_redo(), "Nothing to redo after recording");

			history.undo(document);
			CHECK_TRUE(!document.flag, "Undo restores the before value");
			CHECK_TRUE(!history.can_undo(), "Nothing left to undo");
			CHECK_TRUE(history.can_redo(), "Can redo after undo");

			history.redo(document);
			CHECK_TRUE(document.flag, "Redo restores the after value");
			CHECK_TRUE(history.can_undo(), "Can undo after redo");
			CHECK_TRUE(!history.can_redo(), "Nothing left to redo");
			CHECK_EQUAL(document.replayed, 2, "Undo and redo each call the type once");
		}
		// Changes such as ResetScale::Change carry which instance they edit. The history must store that with each step so undo
		// restores the instance that was edited and leaves the others alone.
		{SCOPE_SECTION("The Change identifies the target")
			Document document;
			History history;
			set_cell(history, document, 1, 5);
			set_cell(history, document, 2, 7);

			history.undo(document);
			CHECK_EQUAL(document.cells[2], 0, "Undo restores the cell the Change identifies");
			CHECK_EQUAL(document.cells[1], 5, "Undo leaves other cells untouched");
			history.undo(document);
			CHECK_EQUAL(document.cells[1], 0, "Undo restores the earlier cell");

			history.redo(document);
			CHECK_EQUAL(document.cells[1], 5, "Redo restores the earlier cell");
			CHECK_EQUAL(document.cells[2], 0, "Redo applies one step at a time");
			history.redo(document);
			CHECK_EQUAL(document.cells[2], 7, "Redo restores the later cell");
		}
		// Changes are stored as bytes, so every kind of Change Serialise.hpp supports must round trip through undo and redo exactly.
		{SCOPE_SECTION("Serialisation")
			{SCOPE_SECTION("Custom serialisation")
				Document document;
				document.profile = {"Ann", {}, {1.f, 0.f, 0.f, 1.f}};
				const Profile before = document.profile;
				const Profile after  = {"Bob Smith", {3, 1, 4, 1, 5}, {1.f, 0.f, 1.f, 1.f}};
				History history;
				set_profile(history, document, after);

				history.undo(document);
				CHECK_TRUE(document.profile == before, "Undo restores the before value");
				history.redo(document);
				CHECK_TRUE(document.profile == after, "Redo restores the after value");
			}
			{SCOPE_SECTION("Containers")
				Document document;
				document.tags = {"red"};
				const std::vector<std::string> before = document.tags;
				const std::vector<std::string> after  = {"red", "green", "blue"};
				History history;
				set_tags(history, document, after);

				history.undo(document);
				CHECK_TRUE(document.tags == before, "Undo restores the before value");
				history.redo(document);
				CHECK_TRUE(document.tags == after, "Redo restores the after value");
			}
		}
		// The history is linear: a new record after undoing discards the undone steps. Changes of different sizes share one buffer,
		// so this also checks the buffer is truncated correctly and the remaining steps still decode their own bytes.
		{SCOPE_SECTION("New record discards the redo branch")
			Document document;
			const std::vector<std::string> tags = {"a tag long enough to vary the record size"};
			History history;
			set_tags(history, document, tags);
			set_cell(history, document, 1, 2);
			history.undo(document);
			set_flag(history, document, true);
			CHECK_TRUE(!history.can_redo(), "The new record discards the redo branch");

			history.undo(document);
			history.undo(document);
			CHECK_TRUE(!history.can_undo(), "The discarded step cannot be undone");
			CHECK_TRUE(same_values(document, Document{}), "Undo restores the initial state");

			history.redo(document);
			history.redo(document);
			CHECK_TRUE(document.tags == tags, "The kept step is redone");
			CHECK_TRUE(document.flag, "The new step is redone");
			CHECK_EQUAL(document.cells[1], 0, "The discarded step is not redone");
		}
		// The CommandExecutor brackets every command with begin_command and end_command, so these behaviours are what make commands
		// atomic undo steps.
		{SCOPE_SECTION("Commands")
			// A command making several edits (e.g. resetting the scale of every selected entity) must undo and redo in one go.
			{SCOPE_SECTION("A command's records are one undo step")
				Document document;
				History history;
				set_cell(history, document, 2, 5);
				const std::size_t mark = history.begin_command();
				set_cell(history, document, 0, 1);
				set_cell(history, document, 1, 2);
				set_flag(history, document, true);
				history.end_command(document, mark, true);

				history.undo(document);
				CHECK_TRUE((document.cells == std::array{0, 0, 5}), "One undo restores every cell the command edited");
				CHECK_TRUE(!document.flag, "One undo restores every type the command edited");
				CHECK_TRUE(history.can_undo(), "The step before the command is separate");

				history.redo(document);
				CHECK_TRUE((document.cells == std::array{1, 2, 5}) && document.flag, "One redo reapplies the whole command");
				CHECK_TRUE(!history.can_redo(), "The command was one step");
			}
			// A command that fails part way through must not leave its earlier edits applied, nor an undo step for itself.
			{SCOPE_SECTION("A failed command is rolled back")
				Document document;
				History history;
				set_cell(history, document, 2, 9);
				const std::size_t mark = history.begin_command();
				set_cell(history, document, 0, 1);
				set_tags(history, document, {"rolled back"});
				history.end_command(document, mark, false);
				CHECK_EQUAL(document.cells[0], 0, "Edits the command recorded are undone");
				CHECK_TRUE(document.tags.empty(), "Every edit the command recorded is undone");
				CHECK_EQUAL(document.cells[2], 9, "Edits before the command are kept");
				CHECK_TRUE(!history.can_redo(), "Rolled back edits cannot be redone");

				history.undo(document);
				CHECK_EQUAL(document.cells[2], 0, "The step before the command is still undoable");
				CHECK_TRUE(!history.can_undo(), "The rolled back edits were not kept");
			}
			// Commands may run other commands. An inner command's records must join the outer command's undo step, and an inner
			// failure must only roll back the inner command's edits.
			{SCOPE_SECTION("Nested commands")
				Document document;
				History history;
				const std::size_t outer = history.begin_command();
				set_cell(history, document, 0, 1);
				{
					const std::size_t inner = history.begin_command();
					set_cell(history, document, 1, 2);
					history.end_command(document, inner, true);
				}
				{
					const std::size_t failed = history.begin_command();
					set_cell(history, document, 2, 3);
					history.end_command(document, failed, false);
				}
				history.end_command(document, outer, true);
				CHECK_TRUE((document.cells == std::array{1, 2, 0}), "Only the failed inner command is rolled back");

				history.undo(document);
				CHECK_TRUE((document.cells == std::array{0, 0, 0}), "One undo restores the outer and inner command");
				CHECK_TRUE(!history.can_undo(), "Nested commands are one step");
				history.redo(document);
				CHECK_TRUE((document.cells == std::array{1, 2, 0}), "One redo reapplies the outer and inner command");
			}
			// The Undo and Redo commands also run between begin_command and end_command. Undoing there must step back exactly as it
			// would outside a command, without recording anything or merging the undone steps together.
			{SCOPE_SECTION("Undo and redo inside a command")
				Document document;
				History history;
				set_cell(history, document, 0, 1);
				set_cell(history, document, 1, 2);
				{
					const std::size_t mark = history.begin_command();
					history.undo(document);
					history.end_command(document, mark, true);
				}
				CHECK_TRUE((document.cells == std::array{1, 0, 0}), "Undo inside a command undoes one step");
				CHECK_TRUE(history.can_redo(), "Undo inside a command keeps the redo branch");
				{
					const std::size_t mark = history.begin_command();
					history.redo(document);
					history.end_command(document, mark, true);
				}
				CHECK_TRUE((document.cells == std::array{1, 2, 0}), "Redo inside a command redoes one step");

				history.undo(document);
				history.undo(document);
				CHECK_TRUE((document.cells == std::array{0, 0, 0}), "The steps stayed separate");
			}
			// A command that fails before editing anything (e.g. an invalid entity ID) must not discard what the user could redo.
			{SCOPE_SECTION("A command that recorded nothing keeps the redo branch")
				Document document;
				History history;
				set_flag(history, document, true);
				history.undo(document);
				{
					const std::size_t mark = history.begin_command();
					history.end_command(document, mark, false);
				}
				{
					const std::size_t mark = history.begin_command();
					history.end_command(document, mark, true);
				}
				CHECK_TRUE(history.can_redo(), "The redo branch is kept");
				CHECK_TRUE(!history.can_undo(), "Empty commands record nothing");
				history.redo(document);
				CHECK_TRUE(document.flag, "The kept redo branch still redoes");
			}
			// Interactive tools such as the gizmo record once when a drag ends, outside any command. Each of those records must be a
			// step of its own rather than joining the command that ran before it.
			{SCOPE_SECTION("Records outside a command are separate steps")
				Document document;
				History history;
				{
					const std::size_t mark = history.begin_command();
					set_cell(history, document, 0, 1);
					history.end_command(document, mark, true);
				}
				set_cell(history, document, 1, 2);
				set_cell(history, document, 2, 3);

				history.undo(document);
				CHECK_TRUE((document.cells == std::array{1, 2, 0}), "Undo restores only the last record");
				history.undo(document);
				CHECK_TRUE((document.cells == std::array{1, 0, 0}), "Each record outside a command is its own step");
				history.undo(document);
				CHECK_TRUE((document.cells == std::array{0, 0, 0}), "The command before them is a separate step");
			}
		}
		// Loading a scene replaces the entities recorded steps point at. clear must forget every step without applying any, and the
		// history must keep working afterwards.
		{SCOPE_SECTION("clear")
			Document document;
			History history;
			set_flag(history, document, true);
			set_cell(history, document, 0, 1);
			history.undo(document);
			const int replayed = document.replayed;
			history.clear();
			CHECK_TRUE(!history.can_undo(), "Nothing to undo");
			CHECK_TRUE(!history.can_redo(), "Nothing to redo");

			history.undo(document);
			history.redo(document);
			CHECK_EQUAL(document.replayed, replayed, "Cleared steps are not applied");

			set_flag(history, document, false);
			history.undo(document);
			CHECK_TRUE(document.flag, "Records again after clear");
		}
		// Steps of different types and sizes share one buffer. Undoing and redoing everything checks every step decodes its own Change,
		// and that intermediate values of a repeatedly edited value are restored in order.
		{SCOPE_SECTION("Mixed history")
			Document document;
			const Document initial = document;
			History history;
			set_flag(history, document, true);
			set_cell(history, document, 0, 42);
			set_profile(history, document, {"Mixed", {7, 8}, {0.f, 1.f, 0.f, 1.f}});
			set_tags(history, document, {"one", "two"});
			set_cell(history, document, 0, -1);
			const Document edited = document;

			history.undo(document);
			history.undo(document);
			CHECK_EQUAL(document.cells[0], 42, "Undo restores the cell's intermediate value");
			CHECK_TRUE(document.tags.empty(), "Undo restores the tags");
			CHECK_TRUE(document.profile == edited.profile, "Older steps are untouched");

			while (history.can_undo())
				history.undo(document);
			CHECK_TRUE(same_values(document, initial), "Undoing everything restores the initial state");

			while (history.can_redo())
				history.redo(document);
			CHECK_TRUE(same_values(document, edited), "Redoing everything restores the edited state");
			CHECK_EQUAL(document.replayed, 10, "Every undo and redo calls the type once");
		}
	}

	void UndoRedoTester::run_performance_tests()
	{
		constexpr std::size_t count = 100'000;
		constexpr std::size_t runs  = 3;

		// Calls p_operation count times and returns the time taken in milliseconds.
		auto time_ms = [&](auto&& p_operation)
		{
			Utility::Stopwatch stopwatch;
			for (std::size_t i = 0; i < count; ++i)
				p_operation();
			return stopwatch.duration_since_start<double, std::milli>().count();
		};
		auto print = [&](const char* p_name, const char* p_state, double p_ms)
		{
			printf("PERF [UndoRedo] %s %s x%zu - best of %zu: %.3fms (%.1fns per call)\n", p_name, p_state, count, runs, p_ms, p_ms * 1'000'000.0 / static_cast<double>(count));
		};
		// Times count calls of p_edit on a fresh history, then undoing and redoing them all.
		// Keeps the best time of each over several runs so a cold first run does not skew the results.
		auto measure = [&](const char* p_state, const Document& p_initial, auto&& p_edit)
		{
			double edit_ms = std::numeric_limits<double>::max();
			double undo_ms = std::numeric_limits<double>::max();
			double redo_ms = std::numeric_limits<double>::max();
			for (std::size_t run = 0; run < runs; ++run)
			{
				Document document = p_initial;
				History history;
				edit_ms = std::min(edit_ms, time_ms([&] { p_edit(history, document); }));
				undo_ms = std::min(undo_ms, time_ms([&] { history.undo(document); }));
				redo_ms = std::min(redo_ms, time_ms([&] { history.redo(document); }));
			}
			print("Edit", p_state, edit_ms);
			print("Undo", p_state, undo_ms);
			print("Redo", p_state, redo_ms);
		};

		// Baseline cost of the history itself, the Change is two bytes.
		measure("bool", Document{}, [](History& p_history, Document& p_document)
		{
			set_flag(p_history, p_document, !p_document.flag);
		});

		// Custom serialised Change, every edit serialises two Profiles and every undo and redo deserialises them.
		Document profile_document;
		profile_document.profile = {"Profile", {1, 2, 3, 4, 5, 6, 7, 8}, {}};
		measure("Profile", profile_document, [](History& p_history, Document& p_document)
		{
			Profile profile = p_document.profile;
			++profile.scores[0];
			set_profile(p_history, p_document, std::move(profile));
		});
	}
} // namespace Test
