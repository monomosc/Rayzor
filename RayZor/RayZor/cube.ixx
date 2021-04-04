module;


#include <memory>
#include <d3d12.h>
#include <directxmath.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

export module cube;

template<class T> using Ptr = Microsoft::WRL::ComPtr<T>;
using namespace DirectX;

import direct_render;
import entity;
import has_assets;
import utilities;
import d3d_components;

using namespace std;

struct matrices_constant_buffer_type {
	XMMATRIX world_matrix;
	XMMATRIX view_matrix;
	XMMATRIX projection_matrix;
	uint8_t padding[64];
};

struct color_constant_buffer_type {
	XMFLOAT4 color;
	uint8_t padding[192];
};

struct vertex_input {
	XMFLOAT4 position;
	XMFLOAT4 normal;
};

namespace entities {
	export class cube : public entity<direct_render, has_assets, default_pipeline, translation_rotation_scale> {
	public:
		using default_pipeline::transform_world_matrix;
		cube(ID3D12Device* device){
			ensure_initialized(device);
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = 2;
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			CHECK_EXCEPTION(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(cbv_descriptor_heap.ReleaseAndGetAddressOf())));

			auto hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto b = CD3DX12_RESOURCE_DESC::Buffer(sizeof(matrices_constant_buffer_type));
			device->CreateCommittedResource(
				&hp,
				D3D12_HEAP_FLAG_NONE,
				&b,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(matrices_constant_buffer.ReleaseAndGetAddressOf()));
			b = CD3DX12_RESOURCE_DESC::Buffer(sizeof(color_constant_buffer_type));
			device->CreateCommittedResource(
				&hp,
				D3D12_HEAP_FLAG_NONE,
				&b,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(color_constant_buffer.ReleaseAndGetAddressOf()));
			D3D12_CONSTANT_BUFFER_VIEW_DESC matrix_cbv_desc = {};
			matrix_cbv_desc.BufferLocation = matrices_constant_buffer->GetGPUVirtualAddress();
			matrix_cbv_desc.SizeInBytes = sizeof(matrices_constant_buffer_type);
			device->CreateConstantBufferView(&matrix_cbv_desc, cbv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());


			/*D3D12_CONSTANT_BUFFER_VIEW_DESC color_cbv_desc = {};
			matrix_cbv_desc.BufferLocation = color_constant_buffer->GetGPUVirtualAddress();
			matrix_cbv_desc.SizeInBytes = sizeof(color_constant_buffer_type);
			device->CreateConstantBufferView(&color_cbv_desc, cbv_descriptor_heap->GetCPUDescriptorHandleForHeapStart() + 1);*/
		}
		static shared_ptr<cube> create_cube(ID3D12Device* device) {
			std::shared_ptr<cube> c = make_shared<cube>(device);
			return c;
		}
		static void initialize_factory(Ptr<ID3D12Device> device);
		virtual ~cube() = default;

		virtual void prepare_render_command_list(ID3D12GraphicsCommandList* cmds);
		virtual void populate_render_command_list(ID3D12GraphicsCommandList* cmds);
		virtual void update_object_assets(XMMATRIX& world_matrix, XMMATRIX& view_matrix, XMMATRIX& projection_matrix);
		XMVECTOR position;
		XMVECTOR rotation_quaternion;
		double scale;


	private:
		static Ptr<ID3D12Resource> vertex_buffer;
		static Ptr<ID3D12DescriptorHeap> cbv_descriptor_heap;
		static D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
		static int vertex_count;

		Ptr<ID3D12Resource> matrices_constant_buffer;
		
		Ptr<ID3D12Resource> color_constant_buffer;


	};
	Ptr<ID3D12DescriptorHeap> cube::cbv_descriptor_heap;
	Ptr<ID3D12Resource> cube::vertex_buffer;
	D3D12_VERTEX_BUFFER_VIEW cube::vertex_buffer_view;
	int cube::vertex_count;

	void cube::prepare_render_command_list(ID3D12GraphicsCommandList* cmds) {
		cmds->SetPipelineState(get_pipeline().Get());
		cmds->SetGraphicsRootSignature(get_root_sig().Get());
		cmds->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmds->IASetVertexBuffers(0, 1, &cube::vertex_buffer_view);
		ID3D12DescriptorHeap* heaps[] = { cube::cbv_descriptor_heap.Get() };
		cmds->SetDescriptorHeaps(1, heaps);
	}
	void cube::populate_render_command_list(ID3D12GraphicsCommandList* cmds) {
		cmds->SetGraphicsRootDescriptorTable(0, cbv_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
		cmds->DrawInstanced(cube::vertex_count, 1, 0, 0);
	}

	void cube::initialize_factory(Ptr<ID3D12Device> device) {
		vertex_input vertices[] = {
			{ { -1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f } },	//front
			{ { 1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f } },
			{ { 1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f } },
			{ { -1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f } },
			{ { 1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f } },
			{ { -1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f } },

			{ { -1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f, 1.0f } },	//bottom
			{ { 1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f, 1.0f } },
			{ { 1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f, 1.0f } },
			{ { 1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f, 1.0f } },
			{ { -1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f, 1.0f } },
			{ { -1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f, 1.0f } },

		};
		vertex_count = sizeof(vertices) / sizeof(vertices[0]);
		auto hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto buf = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
		device->CreateCommittedResource(
			&hp,
			D3D12_HEAP_FLAG_NONE,
			&buf,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(vertex_buffer.ReleaseAndGetAddressOf()));
		void* vertexDataBegin;
		auto r = CD3DX12_RANGE(0, 0);
		vertex_buffer->Map(0, &r, &vertexDataBegin);
		memcpy(vertexDataBegin, vertices, sizeof(vertices));
		vertex_buffer->Unmap(0, nullptr);

		vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
		vertex_buffer_view.StrideInBytes = sizeof(vertex_input);
		vertex_buffer_view.SizeInBytes = sizeof(vertices);
	}

	void cube::update_object_assets(XMMATRIX& world_matrix, XMMATRIX& view_matrix, XMMATRIX& projection_matrix) {
		matrices_constant_buffer_type matrices;
		matrices.world_matrix = XMMatrixTranspose(world_matrix);
		matrices.view_matrix = XMMatrixTranspose(view_matrix);
		matrices.projection_matrix = XMMatrixTranspose(projection_matrix);
		CD3DX12_RANGE read_range(0, 0);
		void* matrix_cbv_data_ptr;
		matrices_constant_buffer->Map(0, &read_range, &matrix_cbv_data_ptr);
		memcpy(matrix_cbv_data_ptr, (void*)&matrices, sizeof(matrices_constant_buffer_type));
		matrices_constant_buffer->Unmap(0, nullptr);
	}
}