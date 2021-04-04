module;

#include <sstream>
#include <mutex>
#include <vector>
#include <DirectXMath.h>
#include <algorithm>
#include <span>

export module basic_entity;


export enum class capability {
	direct_render,
	has_children,
	has_assets,
	is_camera,
	has_pipeline,
	translation_rotation_scale
};


export struct CapabilityDescriptor {
	CapabilityDescriptor(capability c) : cap(c) {};
	capability cap;
};

export template<capability cap>
class register_cap {
public:
	void Register(std::vector<CapabilityDescriptor>& c) { 
		c.push_back(CapabilityDescriptor(cap)); 
	}
};



export class basic_entity {
public:
	basic_entity();
	basic_entity(std::wstring n);
	basic_entity(uint64_t id);
	std::wstring get_name() const;
	uint64_t id;
	virtual void transform_world_matrix(DirectX::XMMATRIX& world_matrix) { };
	bool has_capability(capability cap) {
		auto x = std::find_if(capabilities.begin(), capabilities.end(), [=](CapabilityDescriptor desc) { return desc.cap == cap; });
		return x != capabilities.end();
	}
	void log(std::wstring t) const;
	virtual ~basic_entity() {}

	template<typename T>
	T* get_component() {
		return dynamic_cast<T*>(this);
	}
protected:
	std::mutex lock;
	std::vector<CapabilityDescriptor>* get_capabilities() { return &capabilities; }
private:
	static uint64_t generate_id();
	static std::mutex global_lock;
	static uint64_t global_id;
	std::wstring name;
	std::vector<CapabilityDescriptor> capabilities;
};



uint64_t basic_entity::global_id = 0;
std::mutex basic_entity::global_lock;


basic_entity::basic_entity() {
	id = generate_id();
	std::wstringstream ss;
	ss << "Entity #" << id;
	name = ss.str();
}


basic_entity::basic_entity(std::wstring n) : name(n) {
	id = generate_id();
}

basic_entity::basic_entity(uint64_t i) : id(i) {
	std::wstringstream ss;
	ss << "Entity #" << id;
	name = ss.str();
}
std::wstring basic_entity::get_name() const {
	return name;
}

uint64_t basic_entity::generate_id() {
	std::lock_guard<std::mutex> l(basic_entity::global_lock);
	basic_entity::global_id++;
	return basic_entity::global_id;
}

void basic_entity::log(std::wstring t) const {
	std::wstringstream ss;
	ss << get_name() << L":: " << t << std::endl;
}
