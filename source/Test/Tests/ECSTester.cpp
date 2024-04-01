#include "ECSTester.hpp"
#include "MemoryCorrectnessItem.hpp"

#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "ECS/Storage.hpp"
#include "Utility/Config.hpp"
#include "Utility/Serialise.hpp"
#include "Utility/Logger.hpp"

#include <set>
#include <algorithm>
#include <vector>
#include <random>
#include <chrono>

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNUSED_VARIABLE // Required to stop variables being destroyed before they are used in tests.

namespace Test
{
	struct MyDouble : public PrimitiveTypeWrapper<double>
	{
		static constexpr ECS::ComponentID Persistent_ID = 1;

		static void Serialise(const MyDouble& p_value, std::ofstream& p_out, uint16_t p_version)
		{
			Utility::write_binary(p_out, p_value.value);
		}
		static MyDouble Deserialise(std::ifstream& p_in, uint16_t p_version)
		{
			double value;
			Utility::read_binary(p_in, value);
			return MyDouble{value};
		}
	};
	struct MyFloat : public PrimitiveTypeWrapper<float>
	{
		static constexpr ECS::ComponentID Persistent_ID = 2;

		static void Serialise(const MyFloat& p_value, std::ofstream& p_out, uint16_t p_version)
		{
			Utility::write_binary(p_out, p_value.value);
		}
		static MyFloat Deserialise(std::ifstream& p_in, uint16_t p_version)
		{
			float value;
			Utility::read_binary(p_in, value);
			return MyFloat{value};
		}
	};
	struct MyBool : public PrimitiveTypeWrapper<bool>
	{
		static constexpr ECS::ComponentID Persistent_ID = 3;

		static void Serialise(const MyBool& p_value, std::ofstream& p_out, uint16_t p_version)
		{
			Utility::write_binary(p_out, p_value.value);
		}
		static MyBool Deserialise(std::ifstream& p_in, uint16_t p_version)
		{
			bool value;
			Utility::read_binary(p_in, value);
			return MyBool{value};
		}
	};
	struct MyInt : public PrimitiveTypeWrapper<int>
	{
		static constexpr ECS::ComponentID Persistent_ID = 4;

		static void Serialise(const MyInt& p_value, std::ofstream& p_out, uint16_t p_version)
		{
			Utility::write_binary(p_out, p_value.value);
		}
		static MyInt Deserialise(std::ifstream& p_in, uint16_t p_version)
		{
			int value;
			Utility::read_binary(p_in, value);
			return MyInt{value};
		}
	};

	struct MyChar   : public PrimitiveTypeWrapper<char>        { static constexpr ECS::ComponentID Persistent_ID = 5; };
	struct MyString : public PrimitiveTypeWrapper<std::string> { static constexpr ECS::ComponentID Persistent_ID = 6; };
	struct MySizet  : public PrimitiveTypeWrapper<size_t>      { static constexpr ECS::ComponentID Persistent_ID = 7; };
} // namespace Test


// Formatter for PrimitiveTypeWrapper
namespace std
{
	template<>
	struct formatter<Test::MyDouble>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Test::MyDouble& wrapper, format_context& ctx) const { return format_to(ctx.out(), "{}", wrapper.value); }
	};
	template<>
	struct formatter<Test::MyFloat>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Test::MyFloat& wrapper, format_context& ctx) const { return format_to(ctx.out(), "{}", wrapper.value); }
	};
	template<>
	struct formatter<Test::MyBool>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Test::MyBool& wrapper, format_context& ctx) const { return format_to(ctx.out(), "{}", wrapper.value); }
	};
	template<>
	struct formatter<Test::MyInt>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Test::MyInt& wrapper, format_context& ctx) const { return format_to(ctx.out(), "{}", wrapper.value); }
	};
	template<>
	struct formatter<Test::MyChar>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Test::MyChar& wrapper, format_context& ctx) const { return format_to(ctx.out(), "{}", wrapper.value); }
	};
	template<>
	struct formatter<Test::MyString>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Test::MyString& wrapper, format_context& ctx) const { return format_to(ctx.out(), "{}", wrapper.value); }
	};
	template<>
	struct formatter<Test::MySizet>
	{
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
		auto format(const Test::MySizet& wrapper, format_context& ctx) const { return format_to(ctx.out(), "{}", wrapper.value); }
	};
} // namespace std

namespace Test
{
	#define RUN_MEMORY_TEST(p_alive_count_expected) CHECK_EQUAL(MemoryCorrectnessItem::count_errors(), 0, "Check memory errors"); CHECK_EQUAL(MemoryCorrectnessItem::count_alive(), p_alive_count_expected, "Check alive count");

	void ECSTester::run_performance_tests()
	{}

	void ECSTester::run_unit_tests()
	{
		ECS::Component::set_info<MemoryCorrectnessItem>();
		ECS::Component::set_info<MyDouble>();
		ECS::Component::set_info<MyFloat>();
		ECS::Component::set_info<MyBool>();
		ECS::Component::set_info<MyInt>();
		ECS::Component::set_info<MyChar>();
		ECS::Component::set_info<MyString>();
		ECS::Component::set_info<MySizet>();

		SCOPE_SECTION("ECS");
		{SCOPE_SECTION("count_entities")

			ECS::Storage storage;
			CHECK_EQUAL(storage.count_entities(), 0, "Start Empty");
			std::optional<ECS::Entity> double_ent;
			std::optional<ECS::Entity> float_ent;
			std::optional<ECS::Entity> float_and_double_ent;

			{SCOPE_SECTION("Add entity")

				double_ent = storage.add_entity(MyDouble{42.0});
				CHECK_EQUAL(storage.count_entities(), 1, "Add single component entity");

				float_ent = storage.add_entity(MyFloat{13.f});
				CHECK_EQUAL(storage.count_entities(), 2, "Add new archetype entity");

				float_and_double_ent = storage.add_entity(MyFloat{13.f}, MyDouble{42.0});
				CHECK_EQUAL(storage.count_entities(), 3, "Add another entity with both component types");
			}
			{SCOPE_SECTION("Delete entity");

				storage.delete_entity(double_ent.value());
				CHECK_EQUAL(storage.count_entities(), 2, "Delete entity");

				storage.delete_entity(float_ent.value());
				CHECK_EQUAL(storage.count_entities(), 1, "Delete another entity");

				storage.delete_entity(float_and_double_ent.value());
				CHECK_EQUAL(storage.count_entities(), 0, "Delete last entity");
			}
		}

		{SCOPE_SECTION("count_components")

			ECS::Storage storage;
			CHECK_EQUAL(storage.count_components<MyDouble>(), 0, "Start Empty");
			std::optional<ECS::Entity> double_ent;
			std::optional<ECS::Entity> float_ent;
			std::optional<ECS::Entity> float_and_double_ent;

			{SCOPE_SECTION("Add component");

				double_ent = storage.add_entity(MyDouble{42.0});
				CHECK_EQUAL(storage.count_components<MyDouble>(), 1, "Add MyDouble ent");
				CHECK_EQUAL(storage.count_components<MyFloat>(),  0, "Add MyDouble ent check MyFloat");
				CHECK_EQUAL(storage.count_components<MyInt>(),    0, "Add MyDouble ent check MyInt");

				float_ent = storage.add_entity(MyFloat{13.f});
				CHECK_EQUAL(storage.count_components<MyDouble>(), 1, "Add MyFloat ent check MyDouble");
				CHECK_EQUAL(storage.count_components<MyFloat>(),  1, "Add MyFloat ent check MyFloat");
				CHECK_EQUAL(storage.count_components<MyInt>(),    0, "Add MyFloat ent check MyInt");

				float_and_double_ent = storage.add_entity(MyFloat{13.f}, MyDouble{42.0});
				CHECK_EQUAL(storage.count_components<MyDouble>(), 2, "Add MyFloat and MyDouble ent check MyDouble");
				CHECK_EQUAL(storage.count_components<MyFloat>(),  2, "Add MyFloat and MyDouble ent check MyFloat");
				CHECK_EQUAL(storage.count_components<MyInt>(),    0, "Count type not in storage");
				auto count_combo = storage.count_components<MyDouble, MyFloat>(); // comma in template args is not supported by CHECK_EQUAL
				CHECK_EQUAL(count_combo, 1, "Add MyFloat and MyDouble ent check combo");
			}

			{SCOPE_SECTION("Delete entity")
				storage.delete_entity(double_ent.value());
				CHECK_EQUAL(storage.count_components<MyDouble>(), 1, "Remove MyDouble ent check MyDouble");
				CHECK_EQUAL(storage.count_components<MyFloat>(),  2, "Remove MyDouble ent check MyFloat");

				storage.delete_component<MyFloat>(float_and_double_ent.value());
				CHECK_EQUAL(storage.count_components<MyDouble>(), 1, "Remove MyFloat from float_and_double ent check MyDouble");
				CHECK_EQUAL(storage.count_components<MyFloat>(),  1, "Remove MyFloat from float_and_double ent check MyFloat");
				auto count_combo = storage.count_components<MyDouble, MyFloat>(); // comma in template args is not supported by CHECK_EQUAL
				CHECK_EQUAL(count_combo, 0, "Remove MyFloat from float_and_double ent check combo");

				storage.delete_entity(float_and_double_ent.value());
				CHECK_EQUAL(storage.count_components<MyDouble>(), 0, "Remove MyFloat and MyDouble ent check MyDouble");
				CHECK_EQUAL(storage.count_components<MyFloat>(),  1, "Remove MyFloat and MyDouble ent check MyFloat");

				storage.delete_entity(float_ent.value());
				CHECK_EQUAL(storage.count_components<MyDouble>(), 0, "Remove MyFloat ent check MyDouble");
				CHECK_EQUAL(storage.count_components<MyFloat>(),  0, "Remove MyFloat ent check MyFloat");
			}
		}

		{SCOPE_SECTION("add_entity");
			{
				ECS::Storage storage;
				const MyFloat float_comp{42.f};
				const MyDouble double_comp{13.0};

				RUN_MEMORY_TEST(0);

				storage.add_entity(float_comp);
				CHECK_EQUAL(storage.count_entities(), 1, "Add single component entity");

				storage.add_entity(double_comp);
				CHECK_EQUAL(storage.count_entities(), 2, "Add another single component entity");

				storage.add_entity(double_comp, float_comp);
				CHECK_EQUAL(storage.count_entities(), 3, "Add another entity with both component types");
			}
			{SCOPE_SECTION("Memory correctness");
				MemoryCorrectnessItem::reset(); // Reset before starting new tests
				ECS::Storage storage;
				MemoryCorrectnessItem comp;

				{SCOPE_SECTION("Add by copy");
					storage.add_entity(comp);
					RUN_MEMORY_TEST(2);
				}
				{SCOPE_SECTION("Add second copy");
					storage.add_entity(comp);
					RUN_MEMORY_TEST(3);
				}
				{SCOPE_SECTION("New archetype");
					storage.add_entity(MyFloat{1.f});
					RUN_MEMORY_TEST(3); // Should still be 3 alive because we didnt add another mem correctness item
				}
				{SCOPE_SECTION("Add by move");
					storage.add_entity(MemoryCorrectnessItem());
					RUN_MEMORY_TEST(4); // Should now be 4 alive because we move constructed a new one into storage
				}
				{SCOPE_SECTION("Add 100");
					for (int i = 0; i < 100; i++)
						storage.add_entity(MemoryCorrectnessItem());

					RUN_MEMORY_TEST(104);
				}
			}
		}

		{SCOPE_SECTION("delete_entity");
			{
				ECS::Storage storage;

				auto ent = storage.add_entity(MyFloat{1.f});
				CHECK_EQUAL(storage.count_entities(), 1, "Add 1 entity");

				storage.delete_entity(ent);
				CHECK_EQUAL(storage.count_entities(), 0, "Add 1 entity then delete");
			}
			{SCOPE_SECTION("Memory correctness");
				{
					MemoryCorrectnessItem::reset(); // Reset before starting new tests
					ECS::Storage storage;

					auto ent = storage.add_entity(MemoryCorrectnessItem());
					storage.delete_entity(ent);
					RUN_MEMORY_TEST(0);
				}

				{SCOPE_SECTION("Destroy storage with entity still alive");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;
						storage.add_entity(MemoryCorrectnessItem());
					}
					RUN_MEMORY_TEST(0); // Dangling memory check
				}

				{SCOPE_SECTION("Add 3 delete back to front"); // Back to front is easiest to deal with for removing, no moving is required.
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;
						auto front_ent  = storage.add_entity(MemoryCorrectnessItem());
						auto middle_ent = storage.add_entity(MemoryCorrectnessItem());
						auto back_ent   = storage.add_entity(MemoryCorrectnessItem());

						storage.delete_entity(back_ent);
						CHECK_EQUAL(storage.count_entities(), 2, "First delete");
						RUN_MEMORY_TEST(2);

						storage.delete_entity(middle_ent);
						CHECK_EQUAL(storage.count_entities(), 1, "Second delete");
						RUN_MEMORY_TEST(1);

						storage.delete_entity(front_ent);
						CHECK_EQUAL(storage.count_entities(), 0, "Third delete");
						RUN_MEMORY_TEST(0);
					}
					RUN_MEMORY_TEST(0); // Dangling memory check
				}

				{SCOPE_SECTION("Add 3 delete front to back"); // Front to back is the worst case removal requiring moving of all items.
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;
						auto front_ent  = storage.add_entity(MemoryCorrectnessItem());
						auto middle_ent = storage.add_entity(MemoryCorrectnessItem());
						auto back_ent   = storage.add_entity(MemoryCorrectnessItem());

						storage.delete_entity(front_ent);
						CHECK_EQUAL(storage.count_entities(), 2, "First delete");
						RUN_MEMORY_TEST(2);

						storage.delete_entity(middle_ent);
						CHECK_EQUAL(storage.count_entities(), 1, "Second delete");
						RUN_MEMORY_TEST(1);

						storage.delete_entity(back_ent);
						CHECK_EQUAL(storage.count_entities(), 0, "Third delete");
						RUN_MEMORY_TEST(0);
					}
					RUN_MEMORY_TEST(0); // Dangling memory check
				}
				{SCOPE_SECTION("Add 3 delete middle -> front -> back");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;
						auto front_ent  = storage.add_entity(MemoryCorrectnessItem());
						auto middle_ent = storage.add_entity(MemoryCorrectnessItem());
						auto back_ent   = storage.add_entity(MemoryCorrectnessItem());

						storage.delete_entity(middle_ent);
						CHECK_EQUAL(storage.count_entities(), 2, "First delete");
						RUN_MEMORY_TEST(2);

						storage.delete_entity(front_ent);
						CHECK_EQUAL(storage.count_entities(), 1, "Second delete");
						RUN_MEMORY_TEST(1);

						storage.delete_entity(back_ent);
						CHECK_EQUAL(storage.count_entities(), 0, "Third delete");
						RUN_MEMORY_TEST(0);
					}
					RUN_MEMORY_TEST(0); // Dangling memory check
				}
				{SCOPE_SECTION("Add 100 delete 100 in random order");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;

						std::vector<ECS::Entity> entities;
						entities.reserve(100);

						for (size_t i = 0; i < 100; i++)
							entities.push_back(storage.add_entity(MemoryCorrectnessItem()));

						// shuffle the order of entities
						auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
						auto e = std::default_random_engine(seed);
						std::shuffle(entities.begin(), entities.end(), e);

						for (auto& ent : entities)
							storage.delete_entity(ent);

						RUN_MEMORY_TEST(0);
					}
					RUN_MEMORY_TEST(0); // Dangling memory check
				}
				{SCOPE_SECTION("Overwrite memory");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;

						auto ent = storage.add_entity(MemoryCorrectnessItem());
						storage.delete_entity(ent);
						storage.add_entity(MemoryCorrectnessItem());
						CHECK_EQUAL(storage.count_entities(), 1, "Overwrite Add -> Delete -> Add");
						RUN_MEMORY_TEST(1);
					}
					RUN_MEMORY_TEST(0);
				}
				{SCOPE_SECTION("Overwrite memory test 100");
					{
						MemoryCorrectnessItem::reset();
						ECS::Storage storage;

						std::vector<ECS::Entity> entities;
						entities.reserve(100);

						for (int i = 0; i < 100; i++)
							entities.push_back(storage.add_entity(MemoryCorrectnessItem()));

						for (auto& ent : entities)
							storage.delete_entity(ent);

						for (int i = 0; i < 100; i++)
							entities.push_back(storage.add_entity(MemoryCorrectnessItem()));

						CHECK_EQUAL(storage.count_entities(), 100, "Add 100, Delete 100, Add 100");
						RUN_MEMORY_TEST(100);
					}
					RUN_MEMORY_TEST(0);
				}
			}
		}

		{SCOPE_SECTION("add_component")
			{
				ECS::Storage storage;
				auto entity = storage.add_entity(MyDouble{42.0});

				storage.add_component(entity, MyFloat{13.f});
				CHECK_EQUAL(storage.count_components<MyFloat>(), 1, "Add another component to existing archetype");

				storage.add_component(entity, MyBool{true});
				CHECK_EQUAL(storage.count_components<MyBool>(), 1, "Add another component to existing archetype");

				storage.add_component(entity, MyInt{69});
				CHECK_EQUAL(storage.count_components<MyInt>(), 1, "Add another component to existing archetype");
			}
			{SCOPE_SECTION("Memory correctness");
				{
					MemoryCorrectnessItem::reset();
					ECS::Storage storage;
					auto comp = MemoryCorrectnessItem();

					{SCOPE_SECTION("Add by copy");
						auto entity = storage.add_entity(MyDouble{42.0});
						storage.add_component(entity, comp);
						RUN_MEMORY_TEST(2);
					}
					{SCOPE_SECTION("Add second copy");
						auto entity = storage.add_entity(MyDouble{42.0});
						storage.add_component(entity, comp);
						RUN_MEMORY_TEST(3);
					}
					{SCOPE_SECTION("New archetype");
						auto entity = storage.add_entity(MyDouble{42.0});
						storage.add_component(entity, MyFloat{1.f});
						RUN_MEMORY_TEST(3); // Should still be 3 alive because we didnt add another mem correctness item
					}
					{SCOPE_SECTION("Add by move");
						auto entity = storage.add_entity(MyDouble{42.0});
						storage.add_component(entity, MemoryCorrectnessItem());
						RUN_MEMORY_TEST(4); // Should now be 4 alive because we move constructed a new one into storage
					}
					{SCOPE_SECTION("Add 100");
						for (int i = 0; i < 100; i++)
						{
							auto entity = storage.add_entity(MyDouble{42.0});
							storage.add_component(entity, MemoryCorrectnessItem());
						}

						RUN_MEMORY_TEST(104);
					}
				}
			}
		}

		{SCOPE_SECTION("get_component");

			{SCOPE_SECTION("const")
				ECS::Storage storage;
				auto entity = storage.add_entity(MyDouble{42.0});
				CHECK_EQUAL(storage.get_component<MyDouble>(entity), 42.0, "single component entity");

				// Signature variations
				CHECK_EQUAL(storage.get_component<MyDouble&>(entity), 42.0, "non-const & get");
				CHECK_EQUAL(storage.get_component<const MyDouble&>(entity), 42.0, "const & get");

				// std::decay doesnt work on * so the below dont compile for now.
				// CHECK_EQUAL(storage.get_component<MyDouble*>(entity), 42.0, "single component* entity");
				// CHECK_EQUAL(storage.get_component<const MyDouble*>(entity), 42.0, "single 'const component*' entity");

				{SCOPE_SECTION("MyDouble MyFloat MyBool entity");

					auto entity = storage.add_entity(MyDouble{1.0}, MyFloat{2.f}, MyBool{true});
					CHECK_EQUAL(storage.get_component<MyDouble>(entity), 1.0, "get MyDouble");
					CHECK_EQUAL(storage.get_component<MyFloat>(entity), 2.0f, "get MyFloat");
					CHECK_EQUAL(storage.get_component<MyBool>(entity),  true, "get MyBool");
				}
				{SCOPE_SECTION("MyBool MyFloat MyDouble entity"); // Reverse order but same components/archetype as previous

					auto entity_reverse = storage.add_entity(MyBool{false}, MyFloat{1.f}, MyDouble{2.0});
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_reverse), 2.0, "get MyDouble");
					CHECK_EQUAL(storage.get_component<MyFloat>(entity_reverse), 1.0f, "get MyFloat");
					CHECK_EQUAL(storage.get_component<MyBool>(entity_reverse), false, "get MyBool");
				}
				{SCOPE_SECTION("MyFloat MyBool MyDouble entity"); // Different order but same components/archetype as previous 2
					auto entity_new = storage.add_entity(MyFloat{13.f}, MyBool{true}, MyDouble{42.0});
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_new), 42.0, "get MyDouble");
					CHECK_EQUAL(storage.get_component<MyFloat>(entity_new), 13.0f, "get MyFloat");
					CHECK_EQUAL(storage.get_component<MyBool>(entity_new),   true, "get MyBool");
				}
				{SCOPE_SECTION("char entity");
					auto entity_new = storage.add_entity(MyChar{'G'});
					CHECK_EQUAL(storage.get_component<MyChar>(entity_new), 'G', "get char");
				}

				{SCOPE_SECTION("Data limits") // Setting as many bits as possible
					constexpr MyDouble max_double{std::numeric_limits<double>::max()};
					constexpr MyDouble min_double{std::numeric_limits<double>::min()};

					auto entity_max_double_1 = storage.add_entity(max_double);
					auto entity_min_double_1 = storage.add_entity(min_double);
					auto entity_max_double_2 = storage.add_entity(max_double);
					auto entity_max_double_3 = storage.add_entity(max_double);
					auto entity_min_double_2 = storage.add_entity(min_double);
					auto entity_min_double_3 = storage.add_entity(min_double);
					auto entity_min_double_4 = storage.add_entity(min_double);
					auto entity_max_double_4 = storage.add_entity(max_double);

					CHECK_EQUAL(storage.get_component<MyDouble>(entity_max_double_1), max_double, "1");
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_max_double_2), max_double, "2");
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_max_double_3), max_double, "3");
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_max_double_4), max_double, "4");
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_min_double_1), min_double, "1");
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_min_double_2), min_double, "2");
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_min_double_3), min_double, "3");
					CHECK_EQUAL(storage.get_component<MyDouble>(entity_min_double_4), min_double, "4");
				}
			}
			{SCOPE_SECTION("non-const")
				ECS::Storage storage;

				{SCOPE_SECTION("Get and assign");
					auto entity  = storage.add_entity(MyDouble{42.0});
					auto& comp   = storage.get_component<MyDouble>(entity);
					comp.value   = 69.0;

					CHECK_EQUAL(storage.get_component<MyDouble>(entity), 69.0, "Value change after assign");

					storage.get_component<MyDouble>(entity) += 10.0;
					CHECK_EQUAL(storage.get_component<MyDouble>(entity), 79.0, "get and set one liner");

				}
				{SCOPE_SECTION("Get and assign second"); // Add to the same archetype
					auto entity = storage.add_entity(MyDouble{27.0});
					storage.get_component<MyDouble>(entity) += 3.0;
					CHECK_EQUAL(storage.get_component<MyDouble>(entity), 30.0, "get and set to same archetype");
				}

				{SCOPE_SECTION("Add new archetype ent");
					auto entity = storage.add_entity(MyDouble{27.0}, MyFloat{49.f});
					storage.get_component<MyDouble>(entity) += 3.0;
					CHECK_EQUAL(storage.get_component<MyDouble>(entity), 30.0, "check");

					storage.get_component<MyFloat>(entity) += 1.0f;
					CHECK_EQUAL(storage.get_component<MyFloat>(entity), 50.0f, "check 2");
				}
				{SCOPE_SECTION("MyDouble MyFloat MyInt entity");
					auto entity = storage.add_entity(MyDouble{1.0}, MyFloat{2.f}, MyInt{3});
					storage.get_component<MyInt>(entity) += 1;
					CHECK_EQUAL(storage.get_component<MyInt>(entity), 4, "Edit MyInt");

					storage.get_component<MyFloat>(entity) += 19.0f;
					CHECK_EQUAL(storage.get_component<MyFloat>(entity), MyFloat{21.f}, "Edit MyFloat");

					storage.get_component<MyDouble>(entity) += 13.0;
					CHECK_EQUAL(storage.get_component<MyDouble>(entity), 14.0, "Edit MyDouble");
				}
			}
			{SCOPE_SECTION("Memory correctness");
				{
					MemoryCorrectnessItem::reset();
					ECS::Storage storage;
					auto mem_correct_entity = storage.add_entity(MemoryCorrectnessItem());

					{SCOPE_SECTION("const");
						{SCOPE_SECTION("Return a reference");
							const auto& compRef = storage.get_component<MemoryCorrectnessItem>(mem_correct_entity);
							RUN_MEMORY_TEST(1);
						}
						{SCOPE_SECTION("Return by copy");
							const auto compCopy = storage.get_component<MemoryCorrectnessItem>(mem_correct_entity);
							RUN_MEMORY_TEST(2);
						}
					}
					{SCOPE_SECTION("non-const");
						{SCOPE_SECTION("Return a reference");
							auto& compRef = storage.get_component<MemoryCorrectnessItem>(mem_correct_entity);
							RUN_MEMORY_TEST(1);
						}
						{SCOPE_SECTION("Return by copy");
							const auto compCopy = storage.get_component<MemoryCorrectnessItem>(mem_correct_entity);
							RUN_MEMORY_TEST(2);
						}
					}
				}

				RUN_MEMORY_TEST(0);
			}
		}

		{SCOPE_SECTION("has_components")

			ECS::Storage storage;
			const auto double_float_bool_ent = storage.add_entity(MyDouble{1.0}, MyFloat{2.f}, MyBool{true});
			const auto double_ent            = storage.add_entity(MyDouble{1.0});

			{
				auto has_components = storage.has_components<MyDouble>(double_ent);
				CHECK_EQUAL(has_components, true, "exact match single type single component");
			}
			{
				auto has_components = storage.has_components<MyDouble, MyFloat, MyBool>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "exact match multiple types");
			}
			{
				auto has_components = storage.has_components<MyBool, MyFloat, MyDouble>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "exact match different order multiple types");
			}
			{
				auto has_components = storage.has_components<MyFloat>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "single type match from multiple component middle");
			}
			{
				auto has_components = storage.has_components<MyDouble, MyBool>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "subset match");
			}
			{
				auto has_components = storage.has_components<MyBool, MyDouble>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "subset match different order");
			}
			{
				auto has_components = storage.has_components<MyDouble>(double_float_bool_ent);
				CHECK_EQUAL(has_components, true, "subset match single type");
			}
			{
				auto has_components = storage.has_components<MyString>(double_float_bool_ent);
				CHECK_EQUAL(has_components, false, "no match single type");
			}
			{
				auto has_components = storage.has_components<MyString, MySizet>(double_float_bool_ent);
				CHECK_EQUAL(has_components, false, "no match multiple types");
			}
		}

		{SCOPE_SECTION("foreach");
			{
				ECS::Storage storage;

				{SCOPE_SECTION("Iterate empty");

					size_t count = 0;
					MyDouble sum_double{0.0};
					MyFloat sum_float{0.0f};
					MyInt sum_int{0};

					storage.foreach([&](MyDouble& p_double, MyFloat& p_float, MyInt& p_int)
					{
						sum_double += p_double;
						sum_float  += p_float;
						sum_int    += p_int;
						count++;
					});

					CHECK_EQUAL(sum_double,  0.0, "Sum of doubles");
					CHECK_EQUAL(sum_float,  0.0f, "Sum of floats");
					CHECK_EQUAL(sum_int,       0, "Sum of ints");
					CHECK_EQUAL(count, 0, "Iterate count");
				}

				storage.add_entity(MyDouble{13.69}, MyFloat{1.33f}, MyInt{2});
				storage.add_entity(MyDouble{13.69}, MyFloat{1.33f}, MyInt{2});
				storage.add_entity(MyDouble{13.69}, MyFloat{1.33f}, MyInt{2});

				{SCOPE_SECTION("Exact match and order to archetype");
					size_t count = 0;
					storage.foreach([&](MyDouble& p_double, MyFloat& p_float, MyInt& p_int)
					{
						CHECK_EQUAL(p_double, 13.69, "Check MyDouble");
						CHECK_EQUAL(p_int,        2, "Check MyInt");
						CHECK_EQUAL(p_float,  1.33f, "Check MyFloat");
						count++;
					});
					CHECK_EQUAL(count, 3, "Iterate count");
				}
				{SCOPE_SECTION("Exact match different order to archetype");
					size_t count = 0;
					storage.foreach([&](MyFloat& p_float, MyInt& p_int, MyDouble& p_double)
					{
						CHECK_EQUAL(p_double, 13.69, "Check MyDouble");
						CHECK_EQUAL(p_int,        2, "Check MyInt");
						CHECK_EQUAL(p_float,  1.33f, "Check MyFloat");
						count++;
					});
					CHECK_EQUAL(count, 3, "Ieration count");
				}
				{SCOPE_SECTION("Subset match same order to archetype");
					size_t count = 0;
					storage.foreach([&](MyDouble& p_double, MyFloat& p_float)
					{
						CHECK_EQUAL(p_double, 13.69, "Check MyDouble");
						CHECK_EQUAL(p_float,  1.33f, "Check MyFloat");
						count++;
					});
					CHECK_EQUAL(count, 3, "Ieration count");
				}
				{SCOPE_SECTION("Subset match different order to archetype");
					size_t count = 0;
					storage.foreach([&](MyInt& p_int, MyFloat& p_float)
					{
						CHECK_EQUAL(p_int,       2, "Check MyInt");
						CHECK_EQUAL(p_float, 1.33f, "Check MyFloat");
						count++;
					});
					CHECK_EQUAL(count, 3, "Ieration count");
				}
				{SCOPE_SECTION("Single argument match to archetype");
					{SCOPE_SECTION("Front");
						size_t count = 0;
						storage.foreach([&](MyDouble& p_double)
						{
							CHECK_EQUAL(p_double, 13.69, "Check MyDouble");
							count++;
						});
						CHECK_EQUAL(count, 3, "Ieration count");
					}
					{SCOPE_SECTION("Middle");
						size_t count = 0;
						storage.foreach([&](MyFloat& p_float)
						{
							CHECK_EQUAL(p_float, 1.33f, "Check MyFloat");
							count++;
						});
						CHECK_EQUAL(count, 3, "Ieration count");
					}
					{SCOPE_SECTION("Back");
						size_t count = 0;
						storage.foreach([&](MyInt& p_int)
						{
							CHECK_EQUAL(p_int, 2, "Check MyInt");
							count++;
						});
						CHECK_EQUAL(count, 3, "Ieration count");
					}
				}
				{SCOPE_SECTION("Exact match change data");
					size_t count = 0;
					storage.foreach([&](MyDouble& p_double, MyFloat& p_float, MyInt& p_int)
					{
						p_double += 1.0;
						p_float  += 1.0f;
						p_int    += 1;
						count++;
					});
					CHECK_EQUAL(count, 3, "Ieration count");
				}
				{SCOPE_SECTION("Exact match check changed data");
					storage.foreach([&](MyDouble& p_double, MyFloat& p_float, MyInt& p_int)
					{
						CHECK_EQUAL(p_double, 14.69, "Check MyDouble");
						CHECK_EQUAL(p_int,        3, "Check MyInt");
						CHECK_EQUAL(p_float,  2.33f, "Check MyFloat");
					});
				}
				{SCOPE_SECTION("Add a new entity to a new archetype");
					storage.add_entity(MyDouble{13.0});
					size_t count = 0;
					MyDouble sum{0.0};
					storage.foreach([&](MyDouble& p_double)
					{
						sum += p_double;
						count++;
					});

					CHECK_EQUAL(sum, 57.07, "Sum of doubles"); // 14.69 * 3 + 13.0 = 57.07
					CHECK_EQUAL(count, 4, "Iteration count");
				}
			}

			{SCOPE_SECTION("Entity argument") // ECS::Entity inside the foreach func arguments, expecting the Entity passed with its owned components

				ECS::Storage storage;

				std::vector<ECS::Entity> entities;
				for (int i = 0; i < 12; i++)
					entities.push_back(storage.add_entity(MyDouble{1.0}, MyFloat{2.f}, MyInt{1}));

				{SCOPE_SECTION("Iterate Entity only")

					std::set<ECS::Entity> entity_set;
					storage.foreach([&](ECS::Entity& p_entity){ entity_set.insert(p_entity); });

					for (const auto& entity : entities)
						CHECK_TRUE(entity_set.contains(entity), "Entity in set");
				}

				{SCOPE_SECTION("Iterate exact match");
					std::set<ECS::Entity> entity_set;
					MyDouble sum_double{0.0};
					MyFloat sum_float{0.0f};
					MyInt sum_int{0};

					storage.foreach([&](ECS::Entity& p_entity, MyDouble& p_double, MyFloat& p_float, MyInt& p_int)
					{
						sum_double += p_double;
						sum_float  += p_float;
						sum_int    += p_int;
						entity_set.insert(p_entity);
					});

					for (const auto& entity : entities)
						CHECK_TRUE(entity_set.contains(entity), "Entity in set");
				}

				{SCOPE_SECTION("Iterate partial match");
					std::set<ECS::Entity> entity_set;
					MyDouble sum_double{0.0};
					MyFloat sum_float{0.0f};

					storage.foreach([&](ECS::Entity& p_entity, MyFloat& p_float, MyDouble& p_double)
					{
						sum_double += p_double;
						sum_float  += p_float;
						entity_set.insert(p_entity);
					});

					for (const auto& entity : entities)
						CHECK_TRUE(entity_set.contains(entity), "Entity in set");
				}

				{SCOPE_SECTION("Clear storage")
					for (const auto& entity : entities)
						storage.delete_entity(entity);
					entities.clear();

					{// Iterate empty after delete
						MyDouble sum_double{0.0};
						MyFloat sum_float{0.0f};
						MyInt sum_int{0};
						size_t count = 0;

						storage.foreach([&](ECS::Entity& p_entity, MyDouble& p_double, MyFloat& p_float, MyBool& p_int)
						{
							sum_double += p_double;
							sum_float  += p_float;
							sum_int    += p_int;
							count++;
						});
						CHECK_EQUAL(count, 0, "No iteration after clear");
					}
				}
			}
		}

		{SCOPE_SECTION("Serialisation")
			ECS::Storage storage_deserialised;
			ECS::Storage storage_serialised;
			auto entity             = storage_serialised.add_entity(MyDouble{42.0}, MyFloat{13.f}, MyBool{true}, MyInt{69});
			auto test_ecs_save_file = Config::Scene_Save_Directory / "serialisation_test.ecs"; // TODO: Make sure this is unique.
			std::filesystem::create_directories(test_ecs_save_file.parent_path());
			bool serialised_successfully = true;


			{SCOPE_SECTION("Save")
				ASSERT_THROW(storage_serialised.count_entities() == 1, "To retrieve components after load, we reuse the same entity ID. So we need to make sure we only have 1 entity in the storage.");
				std::ofstream ostrm;
				ostrm.exceptions(std::ofstream::failbit | std::ofstream::badbit);
				try
				{
					ostrm.open(test_ecs_save_file, std::ios::binary);
					ECS::Storage::Serialise(storage_serialised, ostrm, Config::Save_Version);
					ostrm.close();
				}
				catch (std::ofstream::failure& e)
				{
					CHECK_TRUE(false, e.what());
					serialised_successfully = false;
				}
			}

			if (serialised_successfully)
			{SCOPE_SECTION("Load")
				std::ifstream istrm;
				istrm.exceptions(std::ifstream::failbit | std::ifstream::badbit);
				try
				{
					istrm.open(test_ecs_save_file, std::ios::binary);
					storage_deserialised = ECS::Storage::Deserialise(istrm, Config::Save_Version);
					istrm.close();
				}
				catch (std::ifstream::failure& e)
				{
					CHECK_TRUE(false, e.what());
					serialised_successfully = false;
				}
			}

			CHECK_TRUE(serialised_successfully, "Serialisation success");

			if (serialised_successfully) // If serialisation failed, dont bother checking the deserialised storage
			{
				CHECK_EQUAL(storage_serialised.count_entities(), storage_deserialised.count_entities(), "Entity count");
				CHECK_EQUAL(storage_serialised.count_components<MyDouble>(), storage_deserialised.count_components<MyDouble>(), "MyDouble count");
				CHECK_EQUAL(storage_serialised.count_components<MyFloat>(), storage_deserialised.count_components<MyFloat>(), "MyFloat count");
				CHECK_EQUAL(storage_serialised.count_components<MyBool>(), storage_deserialised.count_components<MyBool>(), "MyBool count");
				CHECK_EQUAL(storage_serialised.count_components<MyInt>(), storage_deserialised.count_components<MyInt>(), "MyInt count");

				// While ECS serialisation doesnt guarantee Entity stability, we can ignore this since we only save 1 entity.
				CHECK_EQUAL(storage_serialised.get_component<MyDouble>(entity), storage_deserialised.get_component<MyDouble>(entity), "MyDouble value");
				CHECK_EQUAL(storage_serialised.get_component<MyFloat>(entity), storage_deserialised.get_component<MyFloat>(entity), "MyFloat value");
				CHECK_EQUAL(storage_serialised.get_component<MyBool>(entity), storage_deserialised.get_component<MyBool>(entity), "MyBool value");
				CHECK_EQUAL(storage_serialised.get_component<MyInt>(entity), storage_deserialised.get_component<MyInt>(entity), "MyInt value");
			}

			// Cleanup the test file
			std::filesystem::remove(test_ecs_save_file);
		}
	}
} // namespace Test
DISABLE_WARNING_POP