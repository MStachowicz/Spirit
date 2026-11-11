#pragma once

#include "TestManager.hpp"

namespace Test
{
	// Tests the generic command building blocks in Utility/Command.hpp: the Is_Command concept, invoke_command and text parsing.
	// The Editor's CommandExecutor is not tested here as the Test target does not link the UI library; its undo grouping is covered
	// by UndoRedoTester's Commands tests, which call begin_command and end_command as the executor does.
	class CommandTester : public TestManager
	{
	public:
		CommandTester() : TestManager(std::string("COMMAND")) {}

		void run_unit_tests()        override;
		void run_performance_tests() override {}
	};
} // namespace Test
