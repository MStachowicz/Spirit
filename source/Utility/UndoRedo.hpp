#pragma once

// Linear undo/redo history of edits made to a Context, e.g. the Editor.
//
// Design decisions:
// - Edits are recorded by the code that makes them. An undoable type, usually a command, declares a Change holding what one step needs
//   (typically the target's ID and its before and after values) and static undo and redo functions that put those values back. The
//   edit and its undo sit side by side in one struct, so each can be checked against the other.
// - record only stores a Change, it never applies one: the caller makes the edit itself. Undo and redo functions must not record, so
//   undo and redo can never create history entries.
// - Every Change is serialised into one shared byte buffer and replayed through plain function pointers instantiated per type by
//   record. This avoids a heap allocation and virtual call per step and keeps the whole history in one contiguous block.
// - The CommandExecutor brackets every command with begin_command and end_command. Every step recorded in between shares a tag and is
//   undone and redone with the others, so a command is one undo step however many Changes it records, and a command that fails has
//   its steps undone and discarded. Commands never group their own edits.
// - Nothing here catches exceptions. Failures are reported by commands returning fail, so an exception is a bug or out of memory and
//   is left to end the program.
// - The history holds no reference to its Context, undo and redo are handed it. Its owner can declare the two in any order.

#include "Utility/Logger.hpp"
#include "Utility/Serialise.hpp"

#include <concepts>
#include <cstddef>
#include <span>
#include <vector>

namespace Utility
{
	// A type whose edits the history can undo and redo, usually a command.
	// T::Change is what one step stores: the target's ID and the values needed to restore it, never pointers or references, as the
	// target can move or be deleted before the step is replayed. It must be serialisable. Note Serialise.hpp only accepts trivial types
	// unless they provide custom serialisation, so a Change has no constructors or default member initialisers.
	// T::undo and T::redo restore the before and after values of a Change. They return void as undo cannot be refused: they must not
	// throw or record history, must skip a target that no longer exists and must repeat every side effect of the original edit.
	// e.g. struct ResetScale { struct Change { EntityID entity; glm::vec3 before; glm::vec3 after; };
	//                          static void undo(EditorContext&, const Change&); static void redo(EditorContext&, const Change&); };
	template <typename T, typename Context>
	concept Is_Undoable = requires { typename T::Change; } && Is_Serializable_v<typename T::Change>
		&& requires(Context& p_context, const typename T::Change& p_change)
	{
		{ T::undo(p_context, p_change) } -> std::same_as<void>;
		{ T::redo(p_context, p_change) } -> std::same_as<void>;
	};

	// The undo/redo history for one Context.
	// Exists so user-authored changes can be stepped backwards and forwards regardless of whether they came from the UI, console or MCP.
	// Owned by the Editor. Commands record through it, and the CommandExecutor brackets each command with begin_command and end_command.
	template <typename Context>
	class UndoRedo
	{
	public:
		// Stores p_change as a step, discarding anything that could be redone. Does not apply it, the caller makes the edit.
		// Inside a command the step joins that command's undo step. Outside one, e.g. when a gizmo drag ends, it is a step of its own.
		// Don't record edits that change nothing, they would leave the user an undo step that does nothing.
		//@param p_change What T::undo and T::redo need, e.g. record<ResetScale>({.entity = id, .before = scale, .after = glm::vec3(1.f)}).
		template <typename T>
			requires Is_Undoable<T, Context>
		void record(const typename T::Change& p_change)
		{
			ASSERT(!m_replaying, "Undo and redo functions must not record history.");
			forget_redo();

			const std::size_t begin = m_buffer.size();
			BinaryWriter{m_buffer}.write(p_change);
			const std::size_t command = m_depth > 0 ? m_command : ++m_command;
			m_steps.push_back({&undo_change<T>, &redo_change<T>, begin, m_buffer.size(), command});
			++m_cursor;
		}

		// Restores the state before the last step: every Change the command that recorded it made, newest first.
		//@param p_context Handed to each T::undo.
		void undo(Context& p_context)
		{
			if (!can_undo())
				return;

			m_replaying = true;
			const std::size_t command = m_steps[m_cursor - 1].command;
			while (m_cursor > 0 && m_steps[m_cursor - 1].command == command)
			{
				const Step& step = m_steps[--m_cursor];
				step.undo(p_context, bytes(step));
			}
			m_replaying = false;
		}
		// Reapplies the step that was last undone: every Change the command that recorded it made, oldest first.
		//@param p_context Handed to each T::redo.
		void redo(Context& p_context)
		{
			if (!can_redo())
				return;

			m_replaying = true;
			const std::size_t command = m_steps[m_cursor].command;
			while (m_cursor < m_steps.size() && m_steps[m_cursor].command == command)
			{
				const Step& step = m_steps[m_cursor++];
				step.redo(p_context, bytes(step));
			}
			m_replaying = false;
		}

		bool can_undo() const noexcept { return m_cursor > 0; }
		bool can_redo() const noexcept { return m_cursor < m_steps.size(); }
		// Forgets every step without applying any. Call when the Context is replaced, e.g. a scene is loaded, as recorded Changes may
		// identify state (e.g. entity IDs) that no longer exists or now refers to something else.
		void clear() noexcept
		{
			m_buffer.clear();
			m_steps.clear();
			m_cursor = 0;
		}

		// Called by the CommandExecutor before running a command. Every step recorded until the matching end_command, including those of
		// commands it runs, becomes one undo step.
		//@return The mark to pass to end_command.
		std::size_t begin_command()
		{
			if (m_depth++ == 0)
				++m_command;
			return m_cursor;
		}
		// Called by the CommandExecutor after a command has run. If it failed, undoes and discards the steps it recorded.
		// A command that failed before recording anything, e.g. on an invalid entity ID, keeps the redo branch.
		//@param p_context Handed to T::undo when rolling back.
		//@param p_mark What begin_command returned for this command.
		//@param p_success Whether the command succeeded.
		void end_command(Context& p_context, std::size_t p_mark, bool p_success)
		{
			ASSERT(m_depth > 0, "end_command called without begin_command.");
			--m_depth;
			if (p_success || m_cursor <= p_mark)
				return;

			m_replaying = true;
			while (m_cursor > p_mark)
			{
				const Step& step = m_steps[--m_cursor];
				step.undo(p_context, bytes(step));
			}
			m_replaying = false;
			forget_redo();
		}

	private:
		// One recorded Change and how to replay it.
		struct Step
		{
			void (*undo)(Context&, std::span<const std::byte> p_change); // undo_change<T>
			void (*redo)(Context&, std::span<const std::byte> p_change); // redo_change<T>
			std::size_t begin;   // Offset of the step's serialised Change in m_buffer.
			std::size_t end;     // Offset just past it.
			std::size_t command; // Shared by every step one command recorded, which are undone and redone together.
		};

		// Instantiated per T by record, so a plain function pointer can decode and replay any Change without virtual dispatch.
		template <typename T>
		static void undo_change(Context& p_context, std::span<const std::byte> p_change)
		{
			T::undo(p_context, BinaryReader{p_change}.read<typename T::Change>());
		}
		template <typename T>
		static void redo_change(Context& p_context, std::span<const std::byte> p_change)
		{
			T::redo(p_context, BinaryReader{p_change}.read<typename T::Change>());
		}

		// Discards the steps after the cursor and their bytes, which cannot be redone once something new is recorded.
		void forget_redo()
		{
			m_steps.resize(m_cursor);
			m_buffer.resize(m_cursor == 0 ? 0 : m_steps.back().end);
		}
		std::span<const std::byte> bytes(const Step& p_step) const
		{
			return std::span<const std::byte>{m_buffer}.subspan(p_step.begin, p_step.end - p_step.begin);
		}

		std::vector<std::byte> m_buffer; // Every step's serialised Change, oldest first.
		std::vector<Step> m_steps;       // Offsets into m_buffer, oldest first.
		std::size_t m_cursor  = 0;       // Steps before the cursor can be undone, the rest can be redone.
		std::size_t m_command = 0;       // Tag for the steps recorded now. Advances per command, and per record outside a command.
		std::size_t m_depth   = 0;       // Commands running. More than one while a command runs another.
		bool m_replaying      = false;   // Set while undo and redo functions run, as they must not record.
	};
} // namespace Utility
