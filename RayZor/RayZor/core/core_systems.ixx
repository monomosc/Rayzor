module;
#include <wrl/client.h>
#include <d3d12.h>
#include <cassert>
#include <DirectXMath.h>
#include "../d3dx12.h"


export module core_systems;

import core_utilities;
import core_math;

using Microsoft::WRL::ComPtr;
using DirectX::XMMATRIX;

namespace core {
	class graphics_context;
	class gpu_resource;
	class render_constants;

	export class globals {
	public:
		static ComPtr<ID3D12Device> g_device;
		static void initialize(ID3D12Device* device); 
	};

	export class gpu_resource : non_copyable {
	public:
		friend class graphics_context;
		ID3D12Resource* operator->() { return m_resource; }
		ID3D12Resource** GetAddressOf() { return &m_resource; }
		ID3D12Resource* Get() { return m_resource; }
		void take_ownership(ID3D12Resource* resource) {
			assert(m_resource == nullptr);
			m_resource = resource;
			m_gpu_virtual_address = m_resource->GetGPUVirtualAddress();
		}
		virtual void destroy() {
			m_resource->Release();
			m_resource = nullptr;
		}

	protected:
		gpu_resource() :
			m_gpu_virtual_address(core::gpu_address_null()),
			m_current_state(D3D12_RESOURCE_STATE_COMMON),
			m_transition_state((D3D12_RESOURCE_STATES)-1),
			m_resource(nullptr)
		{}
		gpu_resource(ID3D12Resource* resource, D3D12_RESOURCE_STATES current_state = D3D12_RESOURCE_STATE_COMMON) :
			m_gpu_virtual_address(resource->GetGPUVirtualAddress()),
			m_resource(resource),
			m_current_state(current_state),
			m_transition_state((D3D12_RESOURCE_STATES)-1)
		{}
		ID3D12Resource* m_resource;
		D3D12_RESOURCE_STATES m_current_state;
		D3D12_RESOURCE_STATES m_transition_state;
		D3D12_GPU_VIRTUAL_ADDRESS m_gpu_virtual_address;
	};
	export class upload_buffer : public gpu_resource {
	public:
		template<typename T>
		void create_buffer(size_t num_elements = 1) {
			destroy();
			D3D12_HEAP_PROPERTIES heap_props = {
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				D3D12_MEMORY_POOL_UNKNOWN,
				1,
				1
			};
			D3D12_RESOURCE_DESC resource_desc = {};
			resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resource_desc.Width = sizeof(T) * num_elements;
			resource_desc.Height = 1;
			resource_desc.DepthOrArraySize = 1;
			resource_desc.MipLevels = 1;
			resource_desc.Format = DXGI_FORMAT_UNKNOWN;
			resource_desc.SampleDesc.Count = 1;
			resource_desc.SampleDesc.Quality = 0;
			resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
			
			CHECK_EXCEPTION(globals::g_device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_resource)));
			m_gpu_virtual_address = m_resource->GetGPUVirtualAddress();
			m_buffer_size = sizeof(T) * num_elements;
		}
		template<typename T>
		T* map() {
			void* memory;
			auto range = CD3DX12_RANGE(0, m_buffer_size);
			m_resource->Map(0, &range, &memory);
			return (T*)memory;
		}
		void unmap() {
			auto range = CD3DX12_RANGE(0, m_buffer_size);
			m_resource->Unmap(0, &range);
		}
	private:
		size_t m_buffer_size;
	};
	export class gpu_buffer : public gpu_resource {
	public:
		/// <summary>
		/// leaves source in D3D12_RESOURCE_STATE_COPY_SOURCE and *this in its previous state
		/// </summary>
		void copy_from(upload_buffer& source, graphics_context& cmds);
		void create_buffer(uint32_t num_elems, uint32_t elem_size) {
			destroy();
			m_num_elements = num_elems;
			m_element_size = elem_size;
			m_buffer_size = m_num_elements * m_element_size;
			auto desc = describe_buffer();

			m_current_state = D3D12_RESOURCE_STATE_COMMON;

			D3D12_HEAP_PROPERTIES heap_props;
			heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
			heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_props.CreationNodeMask = 1;
			heap_props.VisibleNodeMask = 1;
			CHECK_EXCEPTION(globals::g_device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE,
				&desc, m_current_state, nullptr, IID_PPV_ARGS(&m_resource)));
			m_gpu_virtual_address = m_resource->GetGPUVirtualAddress();
		}
	private:
	protected:
		uint32_t m_num_elements;
		uint32_t m_element_size;
		uint32_t m_buffer_size;
		D3D12_RESOURCE_FLAGS m_resource_flags = D3D12_RESOURCE_FLAG_NONE;
		D3D12_RESOURCE_DESC describe_buffer(void) {
			assert(m_buffer_size != 0);
			D3D12_RESOURCE_DESC Desc = {};
			Desc.Alignment = 0;
			Desc.DepthOrArraySize = 1;
			Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			Desc.Flags = m_resource_flags;
			Desc.Format = DXGI_FORMAT_UNKNOWN;
			Desc.Height = 1;
			Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			Desc.MipLevels = 1;
			Desc.SampleDesc.Count = 1;
			Desc.SampleDesc.Quality = 0;
			Desc.Width = (UINT64)m_buffer_size;
			return Desc;
		}
	};

	export class graphics_context : public non_copyable {
	public:
		graphics_context(ID3D12GraphicsCommandList* cmds) {
			m_cmds = cmds;
		}
		virtual ~graphics_context() = default;
		void transition_resource(gpu_resource& resource, const D3D12_RESOURCE_STATES new_state, bool immediate = false) {
			auto old_state = resource.m_current_state;
			if (old_state != new_state) {
				assert(m_num_open_barriers < 8);
				D3D12_RESOURCE_BARRIER& barrier = m_barriers[m_num_open_barriers++];
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.pResource = resource.Get();
				barrier.Transition.StateBefore = old_state;
				barrier.Transition.StateAfter = new_state;
				if (new_state == resource.m_transition_state) {
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
				} else {
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				}
				resource.m_current_state = new_state;
			}
			if (immediate || m_num_open_barriers == 8) {
				flush_all_barriers();
			}
		}
		void set_input_layout() {
			assert(false);
		}
		inline void set_constant_buffer(UINT root_index, D3D12_GPU_VIRTUAL_ADDRESS cbv) {
			m_cmds->SetGraphicsRootConstantBufferView(root_index, cbv);
		}
		inline void set_vertex_index_buffers() {
			assert(false);
		}
		inline ID3D12GraphicsCommandList* get_cmds() { return m_cmds.Get(); }

	private:
		ComPtr<ID3D12GraphicsCommandList> m_cmds;
		D3D12_COMMAND_LIST_TYPE m_command_list_type;
		D3D12_RESOURCE_BARRIER m_barriers[8];
		short m_num_open_barriers = 0;
	protected:
		void flush_all_barriers() {
			if (m_num_open_barriers == 0)
				return;
			m_cmds->ResourceBarrier(m_num_open_barriers, m_barriers);
			m_num_open_barriers = 0;
		}
	};
	void gpu_buffer::copy_from(upload_buffer& source, graphics_context& cmds) {
		auto old_state = m_current_state;
		cmds.transition_resource(source, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cmds.transition_resource(*this, D3D12_RESOURCE_STATE_COPY_DEST);
		cmds.get_cmds()->CopyResource(m_resource, source.Get());
		cmds.transition_resource(*this, old_state);
	}
	class render_constants {
	public:
		__declspec(align(256)) struct global_constants {
			math::matrix view_matrix;
			math::matrix proj_matrix;
		};
		static gpu_buffer render_matrices;
		static upload_buffer render_matrices_upload;
		static_assert(sizeof(global_constants) == 256);
	};
	void globals::initialize(ID3D12Device* device) {
		g_device = device;

		render_constants::render_matrices.create_buffer(1, sizeof(render_constants::global_constants));
		render_constants::render_matrices_upload.create_buffer<render_constants::global_constants>(1);
	}
}

