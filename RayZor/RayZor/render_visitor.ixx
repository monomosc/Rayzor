module;

#include <d3d12.h>
#include <DirectXMath.h>
#include <stack>

export module render_visitor;



import basic_entity;
import has_assets;
import direct_render;
import has_children;

using namespace DirectX;

export class render_visitor {
public:
	render_visitor(ID3D12GraphicsCommandList* c) : cmds(c) {
		world_matrix_stack.push(XMMatrixIdentity());
		view_matrix = XMMatrixLookAtLH({ 1.0f,1.0f,3.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		projection_matrix = XMMatrixPerspectiveFovLH(5.0f, 4.0f / 3.0f, 0.01f, 100.0f);
	}
	void render(basic_entity* root_node) {
		visit(root_node);
	}


private:
	ID3D12GraphicsCommandList* cmds;
	std::stack<XMMATRIX> world_matrix_stack;
	XMMATRIX view_matrix;
	XMMATRIX projection_matrix;

private:
	void visit(basic_entity* node);
};



void render_visitor::visit(basic_entity* node) {
	XMMATRIX mat(world_matrix_stack.top());
	node->transform_world_matrix(mat);
	world_matrix_stack.push(mat);
	if (node->has_capability(capability::has_assets)) {
		has_assets::update_object_assets(node, mat, view_matrix, projection_matrix);
	}
	if (node->has_capability(capability::direct_render)) {
		direct_render::prepare_render_command_list(node, cmds);
		direct_render::populate_render_command_list(node, cmds);
	}
	if (node->has_capability(capability::has_children)) {
		auto& children = has_children::get_children(node);
		for (auto& child : children) {
			visit(child.get());
		}
	}
	world_matrix_stack.pop();
}
