module;

#include <d3d12.h>
#include <vector>


export module direct_render;

import basic_entity;


export class direct_render : public virtual basic_entity, public register_cap<capability::direct_render> {
public:
	virtual void prepare_render_command_list(ID3D12GraphicsCommandList* cmds) = 0;
	virtual void populate_render_command_list(ID3D12GraphicsCommandList* cmds) = 0;
	virtual ~direct_render() = default;

	static void prepare_render_command_list(basic_entity* node, ID3D12GraphicsCommandList* cmds) {
		if (!node->has_capability(capability::direct_render)) {
			node->log(L"CRITICAL: prepare_render_command_list on non-render entity");
		}
		node->get_component<direct_render>()->prepare_render_command_list(cmds);
	}
	static void populate_render_command_list(basic_entity* node, ID3D12GraphicsCommandList* cmds) {
		if (!node->has_capability(capability::direct_render)) {
			node->log(L"CRITICAL: populate_render_command_list on non-render entity");
		}
		node->get_component<direct_render>()->populate_render_command_list(cmds);
	}
};
