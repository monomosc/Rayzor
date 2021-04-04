module;

#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <mutex>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "d3dx12.h"
export module d3d_components;

import basic_entity;
import utilities;

using namespace DirectX;
using namespace std;
using Microsoft::WRL::ComPtr;

export class has_pipeline : public virtual basic_entity, public register_cap<capability::has_pipeline> {
public:
	virtual ComPtr<ID3D12PipelineState> get_pipeline() = 0;
	virtual ComPtr<ID3D12RootSignature> get_root_sig() = 0;
	virtual ~has_pipeline() = default;
};

export class default_pipeline : public has_pipeline {
public:
	virtual ComPtr<ID3D12PipelineState> get_pipeline() {
		return get_instance()->pipeline_state;
	}
	virtual ComPtr<ID3D12RootSignature> get_root_sig() {
		return get_instance()->root_signature;
	}
	virtual ~default_pipeline() {}
	static void ensure_initialized(ID3D12Device* device) {
		std::lock_guard<std::mutex> l(lock);
		if (!instance_ptr) {
			instance_ptr = make_shared<default_pipeline_singleton>(device);
		}
	}

private:
	class default_pipeline_singleton {
	public:
		
		ComPtr<ID3D12PipelineState> pipeline_state;
		ComPtr<ID3D12RootSignature> root_signature;

		default_pipeline_singleton(ID3D12Device* device) {
			CD3DX12_DESCRIPTOR_RANGE1 desc_range[2];
			desc_range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			desc_range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			CD3DX12_ROOT_PARAMETER1 rp[1];
			rp[0].InitAsDescriptorTable(2, &desc_range[0], D3D12_SHADER_VISIBILITY_ALL);
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_sig_desc(1, rp, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
			ComPtr<ID3DBlob> sig_blob;
			ComPtr<ID3DBlob> error_blob;
			CHECK_EXCEPTION(D3D12SerializeVersionedRootSignature(&root_sig_desc, sig_blob.ReleaseAndGetAddressOf(), error_blob.ReleaseAndGetAddressOf()));
			device->CreateRootSignature(0, sig_blob->GetBufferPointer(), sig_blob->GetBufferSize(), IID_PPV_ARGS(root_signature.ReleaseAndGetAddressOf()));

			ComPtr<ID3DBlob> pixel_shader_blob;
			ComPtr<ID3DBlob> vertex_shader_blob;
			CHECK_EXCEPTION(D3DReadFileToBlob(L"PixelShader.cso", pixel_shader_blob.ReleaseAndGetAddressOf()));
			CHECK_EXCEPTION(D3DReadFileToBlob(L"VertexShader.cso", vertex_shader_blob.ReleaseAndGetAddressOf()));
			D3D12_INPUT_ELEMENT_DESC vs_input_descs[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
			};
			D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
			pso_desc.InputLayout = { vs_input_descs, _countof(vs_input_descs) };
			pso_desc.pRootSignature = root_signature.Get();
			pso_desc.VS = { vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize() };
			pso_desc.PS = { pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize() };
			pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			pso_desc.DepthStencilState.DepthEnable = false;
			pso_desc.DepthStencilState.StencilEnable = false;
			pso_desc.SampleMask = UINT_MAX;
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pso_desc.NumRenderTargets = 1;
			pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			pso_desc.SampleDesc.Count = 1;

			CHECK_EXCEPTION(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(pipeline_state.ReleaseAndGetAddressOf())));
		}
	};
public:
	using singleton_ptr = std::shared_ptr<default_pipeline_singleton>;
private:
	static std::mutex lock;
	static std::shared_ptr<default_pipeline_singleton> get_instance() {
		return instance_ptr;
	}
	static singleton_ptr instance_ptr;
};

std::mutex default_pipeline::lock;
default_pipeline::singleton_ptr default_pipeline::instance_ptr;

export class translation_rotation_scale : public virtual basic_entity, public register_cap<capability::translation_rotation_scale> {
public:
	translation_rotation_scale() {
		XMStoreFloat4(&rotation_quaternion, XMQuaternionIdentity());
		XMStoreFloat4(&location, XMVectorZero());
	}
	virtual void transform_world_matrix(XMMATRIX& world_matrix) override {
		auto rot_mat = XMMatrixRotationQuaternion(XMLoadFloat4(&rotation_quaternion));
		auto translate_mat = XMMatrixTranslationFromVector(XMLoadFloat4(&location));
		auto scale_mat = XMMatrixScaling(scale, scale, scale);
		auto transform_mat = XMMatrixMultiply(rot_mat, scale_mat);
		transform_mat = XMMatrixMultiply(transform_mat, translate_mat);
		XMMatrixMultiply(transform_mat, world_matrix);
	}
	void rotate(XMVECTOR rotate_by) {
		XMStoreFloat4(&rotation_quaternion, XMQuaternionMultiply(rotate_by, XMLoadFloat4(&rotation_quaternion)));
	}
	void move(XMVECTOR move_by) {
		XMStoreFloat4(&location, XMVectorAdd(XMLoadFloat4(&location), move_by));
	}
	void scale_by(float s) {
		scale *= s;
	}
	float scale = 1.0f;
	XMFLOAT4 rotation_quaternion;
	XMFLOAT4 location;
	virtual ~translation_rotation_scale() {}
};