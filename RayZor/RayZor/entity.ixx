module;
#include <vector>
#include <type_traits>

export module entity;

import basic_entity;


template<typename T> concept RegisterCapabilities = requires(T component, std::vector<CapabilityDescriptor>&c) {
	component.Register(c);
};



class demo_class : public virtual basic_entity {
public:
	void Register(std::vector<CapabilityDescriptor>&c) {}
};

class other_demo_class : public virtual basic_entity {
public:
	void Register(std::vector<CapabilityDescriptor>& c) {}
};

export template<RegisterCapabilities... components>
class entity : public virtual basic_entity, public components... {
public:
	virtual ~entity() {}
protected:
	entity() {
		auto* caps = basic_entity::get_capabilities();
		(static_cast<components*>(this)->Register(*((std::vector<CapabilityDescriptor>*) caps)), ...);
	}
private:
};