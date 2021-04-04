module;

#include <memory>
#include <list>

export module scene_graph;

import basic_entity;
import entity;
import has_children;

export class root_node : public entity<has_children> {
public:
	std::list<std::shared_ptr<basic_entity>> children;
	virtual std::list<std::shared_ptr<basic_entity>>& get_children() {
		return children;
	}
};


export class scene_graph {
public:
	scene_graph() {
		root.reset(new root_node());
	}
	scene_graph(const scene_graph& other) = delete;
	scene_graph(const scene_graph&& other) = delete;
	std::shared_ptr<root_node> get_root() {
		return root;
	}
private:
	std::shared_ptr<root_node> root;
};
