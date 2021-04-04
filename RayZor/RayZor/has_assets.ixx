module;

#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>
#include <sstream>

export module has_assets;

import basic_entity;
import utilities;

using namespace DirectX;


export class has_assets : public virtual basic_entity, public register_cap<capability::has_assets> {
public:
	virtual void update_object_assets(XMMATRIX& world_matrix, XMMATRIX& view_matrix, XMMATRIX& projection_matrix) = 0;
	has_assets* get() {
		return this;
	}
	static void update_object_assets(basic_entity* node, XMMATRIX& world_matrix, XMMATRIX& view_matrix, XMMATRIX& projection_matrix) {
		if (!node->has_capability(capability::has_assets)) {
			node->log(L"CRITICAL: update_object_assets on non-asset Entity");
		}
		node->get_component<has_assets>()->update_object_assets(world_matrix, view_matrix, projection_matrix);
	}
};

