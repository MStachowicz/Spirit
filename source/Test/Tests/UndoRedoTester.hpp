#pragma once

#include "TestManager.hpp"

namespace Test
{
	// Tests Utility::UndoRedo against a plain test Context, covering recording, undo/redo ordering, command grouping and failure rollback.
	// The performance tests guard the per-edit cost, as the Editor records every user-authored change through it.
	class UndoRedoTester : public TestManager
	{
	public:
		UndoRedoTester() : TestManager(std::string("UNDO REDO")) {}

		void run_unit_tests()        override;
		void run_performance_tests() override;
	};
} // namespace Test
