module;

#include <memory>
#include <list>

export module has_children;

import basic_entity;



export class has_children : public virtual basic_entity, public register_cap<capability::has_children> {
public:
	virtual std::list<std::shared_ptr<basic_entity>>& get_children() = 0;
	static std::list<std::shared_ptr<basic_entity>>& get_children(basic_entity* node) {
		if (!node->has_capability(capability::has_children)) {
			node->log(L"CRITICAL: get_children on non-group Entity");
		}
		return node->get_component<has_children>()->get_children();
	}
};

