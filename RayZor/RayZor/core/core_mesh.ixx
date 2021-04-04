module;

#include <d3d12.h>
#include "../d3dx12.h"
export module core_mesh;

import basic_entity;
import core_systems;

namespace core {
	export class mesh : public virtual basic_entity {
	public:
	private:
	protected:
		upload_buffer m_mesh_constants_upload;
		gpu_buffer m_mesh_constants;
	};
}