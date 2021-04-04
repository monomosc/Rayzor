module;

#include <DirectXMath.h>

export module core_math;

using namespace DirectX;

namespace core {
	namespace math {
		class vector;
		class matrix;
		class scalar;
		class quaternion;

		export enum class element_tag {
			zero,
			identity
		};

		export class quaternion {
		public:
			quaternion() { m_quat = XMQuaternionIdentity(); }
			quaternion(const quaternion& other) {
				m_quat = other.m_quat;
			}
			quaternion(const element_tag tag) {
				switch (tag) {
				case element_tag::zero:
					m_quat = XMQuaternionIdentity();
					break;
				case element_tag::identity:
					m_quat = XMQuaternionIdentity();
					break;
				}
				assert(false);
			}
		private:
			XMVECTOR m_quat;
		};
		export class vector {
		public:
			friend class scalar;
			vector() {}
			vector(const XMFLOAT4& v) { m_vec = XMLoadFloat4(&v); }
			vector(const float x, const float y, const float z, const float w) { m_vec = XMVectorSet(x, y, z, w); }
			vector(const vector& other) { m_vec = other.m_vec; }
			vector(const element_tag tag) {
				if (tag == element_tag::zero) {
					m_vec = XMVectorZero();
					return;
				}
				assert(false, "should not create identity-vector");
			}
			operator XMVECTOR() const { return m_vec; }
			inline explicit vector(FXMVECTOR vec) { m_vec = vec; }
			void set_x(scalar x);
			void set_y(scalar x);
			void set_z(scalar x);
			void set_w(scalar x);

		private:
			XMVECTOR m_vec;
		};
		export class scalar {
		public:
			friend class vector;
			scalar() {}
			scalar(float f) { m_vec = XMVectorReplicate(f); }
			scalar(const scalar& other) { m_vec = other.m_vec; }
			scalar(const element_tag tag) {
				switch (tag) {
				case element_tag::zero:
					m_vec = XMVectorZero();
					break;
				case element_tag::identity:
					m_vec = XMVectorReplicate(1.0f);
					break;
				}
			}
			inline explicit scalar(FXMVECTOR vec) { m_vec = vec; }
			inline operator float() const { return XMVectorGetX(m_vec); }
		private:
			XMVECTOR m_vec;
		};
		export class bounding_sphere {
			bounding_sphere(vector v) { m_vec = v; }
			bounding_sphere(vector center, scalar radius) {
				m_vec = center;
				m_vec.set_w(radius);
			}
		private:
			vector m_vec;
		};
		export class matrix {
		public:
			matrix() { m_mat = XMMatrixIdentity(); }
		private:
			XMMATRIX m_mat;
		};

		void vector::set_x(scalar x) { XMVectorPermute<XM_PERMUTE_1X, XM_PERMUTE_0Y, XM_PERMUTE_0Z, XM_PERMUTE_0W>(m_vec, x.m_vec); }
		void vector::set_y(scalar x) { XMVectorPermute<XM_PERMUTE_0X, XM_PERMUTE_1Y, XM_PERMUTE_0Z, XM_PERMUTE_0W>(m_vec, x.m_vec); }
		void vector::set_z(scalar x) { XMVectorPermute<XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_1Z, XM_PERMUTE_0W>(m_vec, x.m_vec); }
		void vector::set_w(scalar x) { XMVectorPermute<XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_0Z, XM_PERMUTE_1W>(m_vec, x.m_vec); }
	}
}