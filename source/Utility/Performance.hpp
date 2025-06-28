#pragma once

#include "Utility/Logger.hpp"

#include "tracy/Tracy.hpp"

#include <array>
#include <chrono>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Utility
{
	// An N-tree structure to represent the performance benchmarks.
	// Keeps the hierarchical structure of performance measurements in a given frame and the historical data of each node based on the name.
	// Node names are built as hierarchical paths during tree construction
	// The historical data persists across frames, allowing for analysis of performance trends over time.
	// Tree is built by constructing ScopedPerformanceBench objects, which are used to measure the performance of a block of code.
	class PerformanceTree
	{
	public:
		friend struct ScopedPerformanceBench; // Allow ScopedPerformanceBench to access add_node and end_node methods.
		using Clock    = std::chrono::steady_clock;
		using Duration = std::chrono::duration<float, std::milli>;

		// Node stores the curent frame's tree structure via the parent and children indices. This is cleared at the end of each frame.
		// Node also stores the historical data of the node, which is stored in a circular buffer of samples.
		// Node indices are stable, the same node will always have the same index in the nodes vector.
		struct Node
		{
			std::array<Duration, 120> samples; // Circular buffer of samples.
			size_t current_index; // Index into samples where the next sample will be added.
			size_t valid_sample_count; // Number of valid samples in the samples array (up to the size of the array).
			Duration average_duration;
			Duration max_duration;
			Duration min_duration;

			std::string name; // Hierarchical name of the node, e.g. "Physics:Collision:BroadPhase".
			std::string stem; // The stem name of the node, e.g. "BroadPhase" for "Physics:Collision:BroadPhase".
			std::optional<size_t> parent;
			std::vector<size_t> children;
			bool active; // Whether the node is active in the current frame.
			Duration frame_accumulated_duration; // Accumulated duration for the current frame.

			// add_node constructor
			Node(std::string_view name, const std::optional<size_t>& parent_index) :
				samples{},
				current_index(0),
				valid_sample_count(0),
				average_duration(Duration::zero()),
				max_duration(Duration::zero()),
				min_duration(Duration::max()),
				name(name),
				stem(name.substr(name.rfind(':') + 1)),
				parent(parent_index),
				children(),
				active(false),
				frame_accumulated_duration(Duration::zero())
			{}

			// Adds a new sample to the history, updating the average, max, and min durations.
			void add_sample(const Duration& p_duration)
			{
				Duration old_sample = samples[current_index];
				bool overwriting_valid_sample = (valid_sample_count == samples.size()) && (old_sample != Duration::zero());

				samples[current_index] = p_duration;
				current_index = (current_index + 1) % samples.size();

				if (valid_sample_count < samples.size())
				{
					valid_sample_count++;
					average_duration = (average_duration * (valid_sample_count - 1) + p_duration) / valid_sample_count;
				}
				else if (overwriting_valid_sample)
					average_duration = average_duration + (p_duration - old_sample) / valid_sample_count;

				if (p_duration > max_duration) max_duration = p_duration;
				if (p_duration < min_duration) min_duration = p_duration;

				if (overwriting_valid_sample && (old_sample == max_duration || old_sample == min_duration))
				{
					max_duration = Duration::zero();
					min_duration = Duration::max();
					for (size_t i = 0; i < valid_sample_count; ++i)
					{
						const Duration& sample = samples[i];
						if (sample == Duration::zero())
							continue;
						if (sample > max_duration)
							max_duration = sample;
						if (sample < min_duration)
							min_duration = sample;
					}
				}
			}
		};

	private:
		std::vector<Node> nodes; // Cleared at the end of each frame, storing the nodes for the current frame.
		std::unordered_map<std::string, size_t> node_lookup; // Accelerator map to find the index of a node's history by its hierarchical name.
		std::optional<size_t> current_node_index; // The index of the current node in frame_nodes, used to add children to the current node.
		std::vector<size_t> frame_active_nodes; // List of active nodes in the current frame.

	public:
		// Resets the performance tree for a new frame.
		void end_frame()
		{
			for (size_t node_index : frame_active_nodes)
			{
				Node& node = nodes[node_index];
				if (node.frame_accumulated_duration > Duration::zero())
					node.add_sample(node.frame_accumulated_duration);

				// Reset the node for the next frame.
				node.parent.reset();
				node.children.clear();
				node.active = false;
				node.frame_accumulated_duration = Duration::zero();
			}
			frame_active_nodes.clear();
			current_node_index.reset();
		}

		const std::vector<size_t> get_root_nodes() const
		{
			std::vector<size_t> roots;
			for (size_t i = 0; i < frame_active_nodes.size(); ++i)
			{
				if (!nodes[frame_active_nodes[i]].parent.has_value())
					roots.push_back(frame_active_nodes[i]);
			}
			return roots;
		}
		const Node& operator[](size_t p_index) const
		{
			ASSERT(p_index < nodes.size(), "Index out of bounds in PerformanceTree::operator[]");
			return nodes[p_index];
		}

	private:
		// Adds a new node to the performance tree with the given name.
		void add_node(std::string_view p_name)
		{
			auto name = current_node_index ? std::format("{}:{}", nodes[*current_node_index].name, p_name) : std::string(p_name);
			auto it = node_lookup.find(name);

			if (it == node_lookup.end())
			{
				nodes.push_back(Node{name, current_node_index });

				if (current_node_index)
					nodes[*current_node_index].children.push_back(nodes.size() - 1);

				current_node_index = nodes.size() - 1; // Set the current node index to the newly added node.
				node_lookup.emplace(nodes.back().name, current_node_index.value());
				frame_active_nodes.push_back(*current_node_index);
			}
			else // If the node of the same name already exists, we update just the tree informaton.
			{
				auto& existing_node = nodes[it->second];
				existing_node.parent = current_node_index;
				if (!existing_node.active) // Only add to children once. If existing node is already active, it means it was added in this frame already.
				{
					if (current_node_index)
						nodes[*current_node_index].children.push_back(it->second);

					frame_active_nodes.push_back(it->second);
				}

				current_node_index = it->second; // Set the current node index to the newly added node.
			}

			nodes[*current_node_index].active = true; // Mark the current node as active for this frame.
		}
		// This branch is ended by moving the current node index to the parent node and accumulating the duration of the current node.
		void end_node(const Duration& p_duration)
		{
			ASSERT_THROW(current_node_index.has_value(), "Cannot end a node when there is no current node.");
			ASSERT(node_lookup.find(nodes[*current_node_index].name) != node_lookup.end(), "Node lookup should contain the current node name.");

			nodes[*current_node_index].frame_accumulated_duration += p_duration;
			current_node_index = nodes[*current_node_index].parent; // Move back to the parent node, so the next add_node will add a child to this node.
		}
	};

	// ScopedPerformanceBench is a RAII class that automatically adds and ends frame_nodes in the performance tree on construction and destruction.
	struct ScopedPerformanceBench
	{
		PerformanceTree::Clock::time_point m_start_time;
		static inline PerformanceTree s_performance_benchmarks = {};

		ScopedPerformanceBench(const std::string_view p_name) : m_start_time(PerformanceTree::Clock::now()) { s_performance_benchmarks.add_node(p_name); }
		~ScopedPerformanceBench() { s_performance_benchmarks.end_node(PerformanceTree::Clock::now() - m_start_time); }
	};
}// namespace Utility

#ifdef Z_DEBUG
#define PERF(p_name) ZoneScopedN(#p_name); Utility::ScopedPerformanceBench perf_##p_name{#p_name};
#define PERF_FRAME_END FrameMark;
#else
#define PERF(p_name) ZoneScopedN(#p_name);
#define PERF_FRAME_END FrameMark;

#endif