module;

#include <tuple>
#include <DirectXMath.h>

export module is_camera;

import basic_entity;
using namespace DirectX;


export class is_camera : public virtual basic_entity, public register_cap<capability::is_camera> {
public:
	virtual bool is_main_camera() = 0;
	virtual std::tuple<XMMATRIX, XMMATRIX> get_view_proj_matrix() = 0;
};