﻿#pragma once
#include <fastenvelope/Types.hpp>
#include <vector>
#include <array>
#include <fenv.h>
#include <unordered_map>
#include<iostream>
#include<arbitraryprecision/fprecision.h>
#include<arbitraryprecision/intervalprecision.h>
namespace fastEnvelope {


	class FastEnvelope
	{
	private:
		static const int ORI_POSITIVE = 1;
		static const int ORI_ZERO = 0;
		static const int ORI_NEGATIVE = -1;
		static const int NOT_INTERSECTD = 2;
		static const int OUT_PRISM = 1;
		static const int IN_PRISM = 0;
		static const int CUT_COPLANAR = 4;
		static const int CUT_COPLANAR_OR_ABOVE = 4;
		static const int CUT_EMPTY = -1;
		static const int CUT_FACE = 3;

		static const int NOT_DEGENERATED = 0;
		static const int NERLY_DEGENERATED = 1;
		static const int DEGENERATED_SEGMENT = 2;
		static const int DEGENERATED_POINT = 3;
		//static const Scalar  BOX_SCALE = 1 / 10.0;
	public:
		FastEnvelope(const std::vector<Vector3>& m_ver, const std::vector<Vector3i>& m_faces, const Scalar& eps, const int& spac);
		bool is_outside(const std::array<Vector3, 3> &triangle) const;
		inline int prism_size() const { return envprism.size(); }
		bool sample_triangle_outside(const std::array<Vector3, 3> &triangle, const Scalar sampleerror) const;
		void print_prisms(const std::array<Vector3, 3> &triangle) const;
		static int test_dege(const std::array<Vector3, 3>& triangle)  {
			return is_triangle_degenerated(triangle);
		}
		static Vector3 accurate_normal_vector(const std::array<Vector3, 3> & triangle, const int &digit);

	private:
		std::vector<std::array<Vector3, 12>> envprism;
		std::unordered_map<int, std::vector<int>> prismmap;
		std::vector<std::array<Vector3, 2>> cornerlist;
		Vector3 min, max;
		int subx, suby, subz;
	private:
		//static bool FastEnvelopeTest(const std::array<Vector3, 3> &triangle, const std::vector<std::array<Vector3, 12>>& envprism);
		//static bool FastEnvelopeTestTemp(const std::array<Vector3, 3> &triangle, const std::vector<std::array<Vector3, 12>>& envprism);
		static bool FastEnvelopeTestImplicit(const std::array<Vector3, 3> &triangle, const std::vector<std::array<Vector3, 12>>& envprism);

		static int seg_cut_tri(const Vector3 & seg0, const Vector3 &seg1, const Vector3&t0, const Vector3&t1, const Vector3 &t2);
	public:
		static void triangle_sample(const std::array<Vector3, 3> &triangle, std::vector<Vector3>& ps, const Scalar &error);
	private:
		//static void seg_tri_cut_on_tir()()
		static void get_bb_corners(const std::vector<Vector3> &vertices, Vector3 &min, Vector3 &max) {
			min = vertices.front();
			max = vertices.front();

			for (size_t j = 0; j < vertices.size(); j++) {
				for (int i = 0; i < 3; i++) {
					min(i) = std::min(min(i), vertices[j](i));
					max(i) = std::max(max(i), vertices[j](i));
				}
			}

			const Scalar dis = (max - min).minCoeff() * 0.1;//TODO it used to be Parameters::box_scale

			for (int j = 0; j < 3; j++) {
				min[j] -= dis;
				max[j] += dis;
			}

			//cout << "min = " << min[0] << " " << min[1] << " " << min[2] << endl;
			//cout << "max = " << max[0] << " " << max[1] << " " << max[2] << endl;
			//            pausee();
		}
		static void  CornerList(const std::vector<std::array<Vector3, 12>>& prism,
			std::vector<std::array<Vector3, 2>>& list) {
			std::vector<Vector3> ver12(12);
			Vector3 min, max;
			list.resize(prism.size());//to be safer
			for (int i = 0; i < prism.size(); i++) {
				for (int j = 0; j < 12; j++) {
					ver12[j] = prism[i][j];
				}
				get_bb_corners(ver12, min, max);
				list[i] = { {min,max } };
			}
		}
		static void BoxFindCells(const Vector3& min, const Vector3& max,
			const Vector3& cellmin, const Vector3& cellmax, const int& subx, const int&suby, const int subz, std::vector<int>& intercell) {

			Vector3 delta;
			delta[0] = (cellmax - cellmin)[0] / subx;
			delta[1] = (cellmax - cellmin)[1] / suby;
			delta[2] = (cellmax - cellmin)[2] / subz;
			//intercell.reserve(int((max - min)[0] / delta[0])*int((max - min)[1] / delta[1])*int((max - min)[2] / delta[2]));
			intercell.clear();
			int location[2][3];
			for (int i = 0; i < 3; i++) {
				location[0][i] = (min[i] - cellmin[i]) / delta[i];
			}
			for (int i = 0; i < 3; i++) {
				location[1][i] = (max[i] - cellmin[i]) / delta[i];
			}
			for (int i = location[0][0]; i <= location[1][0]; i++) {
				for (int j = location[0][1]; j <= location[1][1]; j++) {
					for (int k = location[0][2]; k <= location[1][2]; k++) {
						intercell.emplace_back(k*subx*suby + j * subx + i);
					}
				}
			}


		}
		static void get_triangle_corners(const std::array<Vector3, 3> &triangle, Vector3 &mint, Vector3 &maxt) {
			mint[0] = std::min(std::min(triangle[0][0], triangle[1][0]), triangle[2][0]);
			mint[1] = std::min(std::min(triangle[0][1], triangle[1][1]), triangle[2][1]);
			mint[2] = std::min(std::min(triangle[0][2], triangle[1][2]), triangle[2][2]);
			maxt[0] = std::max(std::max(triangle[0][0], triangle[1][0]), triangle[2][0]);
			maxt[1] = std::max(std::max(triangle[0][1], triangle[1][1]), triangle[2][1]);
			maxt[2] = std::max(std::max(triangle[0][2], triangle[1][2]), triangle[2][2]);

		}

		// to check if a point is in the prisms. the jump index shows the prisms not counted in calculation, and jump is sorted from small to big
		static bool point_out_prism(const Vector3& point, const std::vector<std::array<Vector3, 12>>& envprism, const std::vector<int>& jump);

		static void BoxGeneration(const std::vector<Vector3>& m_ver, const std::vector<Vector3i>& m_faces, std::vector<std::array<Vector3, 12>>& envprism, const Scalar& epsilon);


		static int Implicit_Seg_Facet_interpoint_Out_Prism_redundant(const Vector3& segpoint0, const Vector3& segpoint1, const std::array<Vector3, 3>& triangle,
			const std::vector<std::array<Vector3, 12>>& envprism, const std::vector<int>& jump);



		static int Implicit_Tri_Facet_Facet_interpoint_Out_Prism_redundant(const std::array<Vector3, 3>& triangle, const std::array<Vector3, 3>& facet1, const std::array<Vector3, 3>& facet2,
			const std::vector<std::array<Vector3, 12>>& envprism, const std::vector<int>& jump);



		static int tri_cut_tri_simple(const Vector3& p1, const Vector3& p2, const Vector3& p3,const Vector3& q1, const Vector3& q2, const Vector3& q3);





		static bool is_3_triangle_cut(const std::array<Vector3, 3>& triangle, const std::array<Vector3, 3>& f1, const std::array<Vector3, 3>& f2);

		static int is_triangle_degenerated(const std::array<Vector3, 3>& triangle);
		
		static Vector2 to_2d(const Vector3 &p, int t) {
			return Vector2(p[(t + 1) % 3], p[(t + 2) % 3]);
		}

		
		template<typename T>
		static int orient3D_LPI_filtered_multiprecision(
			T px, T py, T pz, T qx, T qy, T qz,
			T rx, T ry, T rz, T sx, T sy, T sz, T tx, T ty, T tz,
			T ax, T ay, T az, T bx, T by, T bz, T cx, T cy, T cz, const std::function<int(T)> &checker) {
			T a11, a12, a13, a21, a22, a23, a31, a32, a33, d21, d31, d22, d32, d23, d33;
			T px_rx, py_ry, pz_rz, px_cx, py_cy, pz_cz;
			T a2233, a2133, a2132;
			T d, n;
			T d11, d12, d13;
			T d2233, d2332, d2133, d2331, d2132, d2231;
			T det;

			a11 = (px - qx);
			a12 = (py - qy);
			a13 = (pz - qz);
			a21 = (sx - rx);
			a22 = (sy - ry);
			a23 = (sz - rz);
			a31 = (tx - rx);
			a32 = (ty - ry);
			a33 = (tz - rz);
			a2233 = ((a22 * a33) - (a23 * a32));
			a2133 = ((a21 * a33) - (a23 * a31));
			a2132 = ((a21 * a32) - (a22 * a31));
			d = (((a11 * a2233) - (a12 * a2133)) + (a13 * a2132));
			int flag1 = checker(d);
			if (flag1 == -2) {
				return 100;// not enough precision
			}
			if (flag1 == 0) {
				return -2;// not exist
			}
			// The almost static filter for 'd' might be moved here

			px_rx = px - rx;
			py_ry = py - ry;
			pz_rz = pz - rz;
			n = ((((py_ry)* a2133) - ((px_rx)* a2233)) - ((pz_rz)* a2132));

			px_cx = px - cx;
			py_cy = py - cy;
			pz_cz = pz - cz;

			d11 = (d * px_cx) + (a11 * n);
			d21 = (ax - cx);
			d31 = (bx - cx);
			d12 = (d * py_cy) + (a12 * n);
			d22 = (ay - cy);
			d32 = (by - cy);
			d13 = (d * pz_cz) + (a13 * n);
			d23 = (az - cz);
			d33 = (bz - cz);

			d2233 = d22 * d33;
			d2332 = d23 * d32;
			d2133 = d21 * d33;
			d2331 = d23 * d31;
			d2132 = d21 * d32;
			d2231 = d22 * d31;

			det = d11 * (d2233 - d2332) - d12 * (d2133 - d2331) + d13 * (d2132 - d2231);

			int flag2 = checker(det);
			if (flag2 == -2) {
				return 100;// not enough precision
			}
			if (flag2 == 1) {
				if (flag1 == 1) {
					return 1;
				}
				if (flag1 == -1) {
					return -1;
				}
			}
			if (flag2 == -1) {
				if (flag1 == 1) {
					return -1;
				}
				if (flag1 == -1) {
					return 1;
				}
			}
			return 0;

			//if (d.is_class() == ZERO) {
			//	return -2;//not exist
			//}

			//if (d.is_class() == MIXED) {
			//	return 100;//failed in precision
			//}
			//if (det.is_class() == MIXED) {
			//	return 100;//failed in precision
			//}

			//if ((det.is_class() == POSITIVE)) {
			//	if (d.is_class() == POSITIVE) {
			//		return 1;
			//	}
			//	if (d.is_class() == NEGATIVE) {
			//		return -1;
			//	}
			//}
			//if ((det.is_class() == NEGATIVE)) {
			//	if (d.is_class() == POSITIVE) {
			//		return -1;
			//	}
			//	if (d.is_class() == NEGATIVE) {
			//		return 1;
			//	}
			//}

			//return 0;



		}

		template<typename T>

		static int orient3D_TPI_filtered_multiprecision(
			T v1x, T v1y, T v1z, T v2x, T v2y, T v2z, T v3x, T v3y, T v3z,
			T w1x, T w1y, T w1z, T w2x, T w2y, T w2z, T w3x, T w3y, T w3z,
			T u1x, T u1y, T u1z, T u2x, T u2y, T u2z, T u3x, T u3y, T u3z,
			T q1x, T q1y, T q1z, T q2x, T q2y, T q2z, T q3x, T q3y, T q3z, T nbr, const std::function<int(T)> &checker) {

			::feclearexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);

			v3x -= v2x;
			v3y -= v2y;
			v3z -= v2z;
			v2x -= v1x;
			v2y -= v1y;
			v2z -= v1z;
			T nvx = v2y * v3z - v2z * v3y;
			T  nvy = v3x * v2z - v3z * v2x;
			T  nvz = v2x * v3y - v2y * v3x;

			w3x -= w2x;
			w3y -= w2y;
			w3z -= w2z;
			w2x -= w1x;
			w2y -= w1y;
			w2z -= w1z;
			T  nwx = w2y * w3z - w2z * w3y;
			T  nwy = w3x * w2z - w3z * w2x;
			T  nwz = w2x * w3y - w2y * w3x;

			u3x -= u2x;
			u3y -= u2y;
			u3z -= u2z;
			u2x -= u1x;
			u2y -= u1y;
			u2z -= u1z;
			T  nux = u2y * u3z - u2z * u3y;
			T  nuy = u3x * u2z - u3z * u2x;
			T  nuz = u2x * u3y - u2y * u3x;

			T  nwyuz = nwy * nuz - nwz * nuy;
			T  nwxuz = nwx * nuz - nwz * nux;
			T  nwxuy = nwx * nuy - nwy * nux;

			T  nvyuz = nvy * nuz - nvz * nuy;
			T  nvxuz = nvx * nuz - nvz * nux;
			T  nvxuy = nvx * nuy - nvy * nux;

			T  nvywz = nvy * nwz - nvz * nwy;
			T  nvxwz = nvx * nwz - nvz * nwx;
			T  nvxwy = nvx * nwy - nvy * nwx;

			T  d = nvx * nwyuz - nvy * nwxuz + nvz * nwxuy;
			
			int flag1 = checker(d);
			if (flag1 == -2) {
				return 100;// not enough precision
			}
			if (flag1 == 0) {
				return -2;// not exist
			}

			T  p1 = nvx * v1x + nvy * v1y + nvz * v1z;
			T  p2 = nwx * w1x + nwy * w1y + nwz * w1z;
			T  p3 = nux * u1x + nuy * u1y + nuz * u1z;

			T  n1 = p1 * nwyuz - p2 * nvyuz + p3 * nvywz;
			T  n2 = p2 * nvxuz - p3 * nvxwz - p1 * nwxuz;
			T  n3 = p3 * nvxwy - p2 * nvxuy + p1 * nwxuy;

			T  dq3x = d * q3x;
			T  dq3y = d * q3y;
			T  dq3z = d * q3z;

			T  a11 = n1 - dq3x;
			T  a12 = n2 - dq3y;
			T  a13 = n3 - dq3z;
			T  a21 = q1x - q3x;
			T  a22 = q1y - q3y;
			T  a23 = q1z - q3z;
			T  a31 = q2x - q3x;
			T  a32 = q2y - q3y;
			T  a33 = q2z - q3z;

			T  det = a11 * (a22*a33 - a23 * a32) - a12 * (a21*a33 - a23 * a31) + a13 * (a21*a32 - a22 * a31);

			if (::fetestexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)) return 0; // Fast reject in case of under/overflow

			int flag2 = checker(det);
			if (flag2 == -2) {
				return 100;// not enough precision
			}
			if (flag2 == 1) {
				if (flag1 == 1) {
					return 1;
				}
				if (flag1 == -1) {
					return -1;
				}
			}
			if (flag2 == -1) {
				if (flag1 == 1) {
					return -1;
				}
				if (flag1 == -1) {
					return 1;
				}
			}
			return 0;

		}

		const std::function<int(double)> check_double = [](double v) {
			if (fabs(v) < 1e-10)
				return -2;

			if (v > 0)
				return 1;

			if (v < 0)
				return -1;

			return 0;
		};


		
		const std::function<int(arbitrary_precision::interval<arbitrary_precision::float_precision>)> check_interval =
			[](arbitrary_precision::interval<arbitrary_precision::float_precision> v) {
			const auto clazz = v.is_class();
			if (clazz == arbitrary_precision::MIXED || clazz == arbitrary_precision::NO_CLASS)
				return -2;

			if (clazz == arbitrary_precision::POSITIVE)
				return 1;

			if (clazz == arbitrary_precision::NEGATIVE)
				return -1;

			assert(clazz == arbitrary_precision::ZERO);
			return 0;
		};



	};
}
