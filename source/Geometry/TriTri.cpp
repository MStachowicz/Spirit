#include "TriTri.hpp"

#include "glm/glm.hpp"

namespace Geometry
{
	static bool check_min_max(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                          const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2,
	                          glm::vec3& v1, glm::vec3& v2, glm::vec3& N1)
	{
		v1 = p2 - q1;
		v2 = p1 - q1;
		N1 = glm::cross(v1, v2);
		v1 = q2 - q1;

		if (glm::dot(v1, N1) > 0.0f)
			return false;

		v1 = p2 - p1;
		v2 = r1 - p1;
		N1 = glm::cross(v1, v2);
		v1 = r2 - p1;

		if (glm::dot(v1, N1) > 0.0f)
			return false;
		else
			return true;
	}

	static float orient_2D(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
	{
		return ((a[0] - c[0]) * (b[1] - c[1]) - (a[1] - c[1]) * (b[0] - c[0]));
	}

	static bool intersection_test_vertex(const glm::vec2& P1, const glm::vec2& Q1, const glm::vec2& R1,
	                                     const glm::vec2& P2, const glm::vec2& Q2, const glm::vec2& R2)
	{
		if (orient_2D(R2, P2, Q1) >= 0.0f)
			if (orient_2D(R2, Q2, Q1) <= 0.0f)
				if (orient_2D(P1, P2, Q1) > 0.0f)
					if (orient_2D(P1, Q2, Q1) <= 0.0f)
						return true;
					else
						return false;
				else if (orient_2D(P1, P2, R1) >= 0.0f)
					if (orient_2D(Q1, R1, P2) >= 0.0f)
						return true;
					else
						return false;
				else
					return false;
			else if (orient_2D(P1, Q2, Q1) <= 0.0f)
				if (orient_2D(R2, Q2, R1) <= 0.0f)
					if (orient_2D(Q1, R1, Q2) >= 0.0f)
						return true;
					else
						return false;
				else
					return false;
			else
				return false;
		else if (orient_2D(R2, P2, R1) >= 0.0f)
			if (orient_2D(Q1, R1, R2) >= 0.0f)
				if (orient_2D(P1, P2, R1) >= 0.0f)
					return true;
				else
					return false;
			else if (orient_2D(Q1, R1, Q2) >= 0.0f)
				if (orient_2D(R2, R1, Q2) >= 0.0f)
					return true;
				else
					return false;
			else
				return false;
		else
			return false;
	}

	static bool intersection_test_edge(const glm::vec2& P1, const glm::vec2& Q1, const glm::vec2& R1, const glm::vec2& P2, const glm::vec2& R2)
	{
		if (orient_2D(R2, P2, Q1) >= 0.0f)
			if (orient_2D(P1, P2, Q1) >= 0.0f)
				if (orient_2D(P1, Q1, R2) >= 0.0f)
					return true;
				else
					return false;
			else if (orient_2D(Q1, R1, P2) >= 0.0f)
				if (orient_2D(R1, P1, P2) >= 0.0f)
					return true;
				else
					return false;
			else
				return false;
		else if (orient_2D(R2, P2, R1) >= 0.0f)
			if (orient_2D(P1, P2, R1) >= 0.0f)
				if (orient_2D(P1, R1, R2) >= 0.0f)
					return true;
				else
					return false;
			else
				return false;
		else
			return false;
	}

	static bool ccw_tri_tri_intersection_2D(const glm::vec2& p1, const glm::vec2& q1, const glm::vec2& r1,
	                                        const glm::vec2& p2, const glm::vec2& q2, const glm::vec2& r2)
	{
		if (orient_2D(p2, q2, p1) >= 0.0f)
			if (orient_2D(q2, r2, p1) >= 0.0f)
				if (orient_2D(r2, p2, p1) >= 0.0f)
					return true;
				else
					return intersection_test_edge(p1, q1, r1, p2, r2);
			else if (orient_2D(r2, p2, p1) >= 0.0f)
				return intersection_test_edge(p1, q1, r1, r2, q2);
			else
				return intersection_test_vertex(p1, q1, r1, p2, q2, r2);
		else
		{
			if (orient_2D(q2, r2, p1) >= 0.0f)
				if (orient_2D(r2, p2, p1) >= 0.0f)
					return intersection_test_edge(p1, q1, r1, q2, p2);
				else
					return intersection_test_vertex(p1, q1, r1, q2, r2, p2);
			else
				return intersection_test_vertex(p1, q1, r1, r2, p2, q2);
		}
	};

	static bool tri_tri_overlap_test_2d(const glm::vec2& p1, const glm::vec2& q1, const glm::vec2& r1,
	                                    const glm::vec2& p2, const glm::vec2& q2, const glm::vec2& r2)
	{
		if (orient_2D(p1, q1, r1) < 0.0f)
			if (orient_2D(p2, q2, r2) < 0.0f)
				return ccw_tri_tri_intersection_2D(p1, r1, q1, p2, r2, q2);
			else
				return ccw_tri_tri_intersection_2D(p1, r1, q1, p2, q2, r2);
		else if (orient_2D(p2, q2, r2) < 0.0f)
			return ccw_tri_tri_intersection_2D(p1, q1, r1, p2, r2, q2);
		else
			return ccw_tri_tri_intersection_2D(p1, q1, r1, p2, q2, r2);
	};

	static bool coplanar_tri_tri_3D(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                                const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2,
	                                const glm::vec3& normal_1)
	{
		float n_x = ((normal_1[0] < 0) ? -normal_1[0] : normal_1[0]);
		float n_y = ((normal_1[1] < 0) ? -normal_1[1] : normal_1[1]);
		float n_z = ((normal_1[2] < 0) ? -normal_1[2] : normal_1[2]);

		// Projection of the triangles in 3D onto 2D such that the area of the projection is maximized.
		glm::vec2 P1, Q1, R1, P2, Q2, R2;

		if ((n_x > n_z) && (n_x >= n_y))
		{ // Project onto plane YZ
			P1[0] = q1[2];
			P1[1] = q1[1];
			Q1[0] = p1[2];
			Q1[1] = p1[1];
			R1[0] = r1[2];
			R1[1] = r1[1];

			P2[0] = q2[2];
			P2[1] = q2[1];
			Q2[0] = p2[2];
			Q2[1] = p2[1];
			R2[0] = r2[2];
			R2[1] = r2[1];
		}
		else if ((n_y > n_z) && (n_y >= n_x))
		{ // Project onto plane XZ
			P1[0] = q1[0];
			P1[1] = q1[2];
			Q1[0] = p1[0];
			Q1[1] = p1[2];
			R1[0] = r1[0];
			R1[1] = r1[2];

			P2[0] = q2[0];
			P2[1] = q2[2];
			Q2[0] = p2[0];
			Q2[1] = p2[2];
			R2[0] = r2[0];
			R2[1] = r2[2];
		}
		else
		{ // Project onto plane XY
			P1[0] = p1[0];
			P1[1] = p1[1];
			Q1[0] = q1[0];
			Q1[1] = q1[1];
			R1[0] = r1[0];
			R1[1] = r1[1];

			P2[0] = p2[0];
			P2[1] = p2[1];
			Q2[0] = q2[0];
			Q2[1] = q2[1];
			R2[0] = r2[0];
			R2[1] = r2[1];
		}

		return tri_tri_overlap_test_2d(P1, Q1, R1, P2, Q2, R2);
	};

	// Permutation in a canonical form of T2's vertices
	static bool tri_tri_3D(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                       const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2,
	                       glm::vec3& v1, glm::vec3& v2, glm::vec3& N1,
	                       float dp2, float dq2, float dr2)
	{
		if (dp2 > 0.0f)
			if (dq2 > 0.0f)
				return check_min_max(p1, r1, q1, r2, p2, q2, v1, v2, N1);
			else if (dr2 > 0.0f)
				return check_min_max(p1, r1, q1, q2, r2, p2, v1, v2, N1);
			else
				return check_min_max(p1, q1, r1, p2, q2, r2, v1, v2, N1);
		else if (dp2 < 0.0f)
			if (dq2 < 0.0f)
				return check_min_max(p1, q1, r1, r2, p2, q2, v1, v2, N1);
			else if (dr2 < 0.0f)
				return check_min_max(p1, q1, r1, q2, r2, p2, v1, v2, N1);
			else
				return check_min_max(p1, r1, q1, p2, q2, r2, v1, v2, N1);
		else if (dq2 < 0.0f)
			if (dr2 >= 0.0f)
				return check_min_max(p1, r1, q1, q2, r2, p2, v1, v2, N1);
			else
				return check_min_max(p1, q1, r1, p2, q2, r2, v1, v2, N1);
		else if (dq2 > 0.0f)
			if (dr2 > 0.0f)
				return check_min_max(p1, r1, q1, p2, q2, r2, v1, v2, N1);
			else
				return check_min_max(p1, q1, r1, q2, r2, p2, v1, v2, N1);
		else if (dr2 > 0.0f)
			return check_min_max(p1, q1, r1, r2, p2, q2, v1, v2, N1);
		else if (dr2 < 0.0f)
			return check_min_max(p1, r1, q1, r2, p2, q2, v1, v2, N1);
		else
			return coplanar_tri_tri_3D(p1, q1, r1, p2, q2, r2, N1);
	}

	bool tri_tri_is_intersecting(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                             const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2)
	{
		// Compute distance signs of p1, q1 and r1 to the plane of triangle(p2,q2,r2)
		glm::vec3 v1 = p2 - r2;
		glm::vec3 v2 = q2 - r2;
		glm::vec3 N2 = glm::cross(v1, v2);

		v1        = p1 - r2;
		float dp1 = glm::dot(v1, N2);
		v1        = q1 - r2;
		float dq1 = glm::dot(v1, N2);
		v1        = r1 - r2;
		float dr1 = glm::dot(v1, N2);

		// Coplanarity robustness check
		if constexpr (USE_EPSILON_TEST_TRI_TRI)
		{
			constexpr auto EPSILON_TRI_TRI = std::numeric_limits<decltype(dp1)>::epsilon();
			if (std::abs(dp1) < EPSILON_TRI_TRI) dp1 = 0.0f;
			if (std::abs(dq1) < EPSILON_TRI_TRI) dq1 = 0.0f;
			if (std::abs(dr1) < EPSILON_TRI_TRI) dr1 = 0.0f;
		}

		if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))
			return false;

		// Compute distance signs of p2, q2 and r2 to the plane of triangle(p1,q1,r1)
		v1           = q1 - p1;
		v2           = r1 - p1;
		glm::vec3 N1 = glm::cross(v1, v2);

		v1        = p2 - r1;
		float dp2 = glm::dot(v1, N1);
		v1        = q2 - r1;
		float dq2 = glm::dot(v1, N1);
		v1        = r2 - r1;
		float dr2 = glm::dot(v1, N1);

		if constexpr (USE_EPSILON_TEST_TRI_TRI)
		{
			constexpr auto EPSILON_TRI_TRI = std::numeric_limits<decltype(dp2)>::epsilon();
			if (std::abs(dp2) < EPSILON_TRI_TRI) dp2 = 0.0f;
			if (std::abs(dq2) < EPSILON_TRI_TRI) dq2 = 0.0f;
			if (std::abs(dr2) < EPSILON_TRI_TRI) dr2 = 0.0f;
		}

		if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f))
			return false;

		// Permutation in a canonical form of T1's vertices
		if (dp1 > 0.0f)
			if (dq1 > 0.0f)
				return tri_tri_3D(r1, p1, q1, p2, r2, q2, v1, v2, N1, dp2, dr2, dq2);
			else if (dr1 > 0.0f)
				return tri_tri_3D(q1, r1, p1, p2, r2, q2, v1, v2, N1, dp2, dr2, dq2);
			else
				return tri_tri_3D(p1, q1, r1, p2, q2, r2, v1, v2, N1, dp2, dq2, dr2);
		else if (dp1 < 0.0f)
			if (dq1 < 0.0f)
				return tri_tri_3D(r1, p1, q1, p2, q2, r2, v1, v2, N1, dp2, dq2, dr2);
			else if (dr1 < 0.0f)
				return tri_tri_3D(q1, r1, p1, p2, q2, r2, v1, v2, N1, dp2, dq2, dr2);
			else
				return tri_tri_3D(p1, q1, r1, p2, r2, q2, v1, v2, N1, dp2, dr2, dq2);
		else if (dq1 < 0.0f)
			if (dr1 >= 0.0f)
				return tri_tri_3D(q1, r1, p1, p2, r2, q2, v1, v2, N1, dp2, dr2, dq2);
			else
				return tri_tri_3D(p1, q1, r1, p2, q2, r2, v1, v2, N1, dp2, dq2, dr2);
		else if (dq1 > 0.0f)
			if (dr1 > 0.0f)
				return tri_tri_3D(p1, q1, r1, p2, r2, q2, v1, v2, N1, dp2, dr2, dq2);
			else
				return tri_tri_3D(q1, r1, p1, p2, q2, r2, v1, v2, N1, dp2, dq2, dr2);
		else if (dr1 > 0.0f)
			return tri_tri_3D(r1, p1, q1, p2, q2, r2, v1, v2, N1, dp2, dq2, dr2);
		else if (dr1 < 0.0f)
			return tri_tri_3D(r1, p1, q1, p2, r2, q2, v1, v2, N1, dp2, dr2, dq2);
		else
			return coplanar_tri_tri_3D(p1, q1, r1, p2, q2, r2, N1);
	};

	static bool construct_intersection(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                                   const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2,
	                                   const glm::vec3& N1, const glm::vec3& N2,
	                                   glm::vec3& v1, glm::vec3& v2, glm::vec3& N, glm::vec3& v, float& alpha,
	                                   glm::vec3& l_source, glm::vec3& l_target)
	{
		// NOTE: a faster, but possibly less precise, method of computing point B is described here: https://github.com/erich666/jgt-code/issues/5
		v1 = q1 - p1;
		v2 = r2 - p1;
		N  = glm::cross(v1, v2);
		v  = p2 - p1;

		if (glm::dot(v, N) > 0.0f)
		{
			v1 = r1 - p1;
			N  = glm::cross(v1, v2);
			if (glm::dot(v, N) <= 0.0f)
			{
				v2 = q2 - p1;
				N  = glm::cross(v1, v2);
				if (glm::dot(v, N) > 0.0f)
				{
					v1               = p1 - p2;
					v2               = p1 - r1;
					alpha            = glm::dot(v1, N2) / glm::dot(v2, N2);
					v1               = v2 * alpha;
					l_source = p1 - v1;
					v1               = p2 - p1;
					v2               = p2 - r2;
					alpha            = glm::dot(v1, N1) / glm::dot(v2, N1);
					v1               = v2 * alpha;
					l_target = p2 - v1;
					return true;
				}
				else
				{
					v1               = p2 - p1;
					v2               = p2 - q2;
					alpha            = glm::dot(v1, N1) / glm::dot(v2, N1);
					v1               = v2 * alpha;
					l_source = p2 - v1;
					v1               = p2 - p1;
					v2               = p2 - r2;
					alpha            = glm::dot(v1, N1) / glm::dot(v2, N1);
					v1               = v2 * alpha;
					l_target = p2 - v1;
					return true;
				}
			}
			else
				return false;
		}
		else
		{
			v2 = q2 - p1;
			N  = glm::cross(v1, v2);

			if (glm::dot(v, N) < 0.0f)
				return false;
			else
			{
				v1 = r1 - p1;
				N  = glm::cross(v1, v2);
				if (glm::dot(v, N) >= 0.0f)
				{
					v1               = p1 - p2;
					v2               = p1 - r1;
					alpha            = glm::dot(v1, N2) / glm::dot(v2, N2);
					v1               = v2 * alpha;
					l_source = p1 - v1;
					v1               = p1 - p2;
					v2               = p1 - q1;
					alpha            = glm::dot(v1, N2) / glm::dot(v2, N2);
					v1               = v2 * alpha;
					l_target = p1 - v1;
					return true;
				}
				else
				{
					v1               = p2 - p1;
					v2               = p2 - q2;
					alpha            = glm::dot(v1, N1) / glm::dot(v2, N1);
					v1               = v2 * alpha;
					l_source = p2 - v1;
					v1               = p1 - p2;
					v2               = p1 - q1;
					alpha            = glm::dot(v1, N2) / glm::dot(v2, N2);
					v1               = v2 * alpha;
					l_target = p1 - v1;
					return true;
				}
			}
		}
	}

	static bool tri_tri_inter_3D(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                             const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2,
	                             const float& dp2, const float& dq2, const float& dr2,
	                             glm::vec3& N1, glm::vec3& N2, glm::vec3& v1, glm::vec3& v2, glm::vec3& N, glm::vec3& v, float& alpha,
	                             bool& coplanar, glm::vec3& l_source, glm::vec3& l_target)
	{
		if (dp2 > 0.0f)
			if (dq2 > 0.0f)
				return construct_intersection(p1, r1, q1, r2, p2, q2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
			else if (dr2 > 0.0f)
				return construct_intersection(p1, r1, q1, q2, r2, p2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
			else
				return construct_intersection(p1, q1, r1, p2, q2, r2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
		else if (dp2 < 0.0f)
			if (dq2 < 0.0f)
				return construct_intersection(p1, q1, r1, r2, p2, q2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
			else if (dr2 < 0.0f)
				return construct_intersection(p1, q1, r1, q2, r2, p2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
			else
				return construct_intersection(p1, r1, q1, p2, q2, r2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
		else if (dq2 < 0.0f)
			if (dr2 >= 0.0f)
				return construct_intersection(p1, r1, q1, q2, r2, p2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
			else
				return construct_intersection(p1, q1, r1, p2, q2, r2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
		else if (dq2 > 0.0f)
			if (dr2 > 0.0f)
				return construct_intersection(p1, r1, q1, p2, q2, r2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
			else
				return construct_intersection(p1, q1, r1, q2, r2, p2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
		else if (dr2 > 0.0f)
			return construct_intersection(p1, q1, r1, r2, p2, q2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
		else if (dr2 < 0.0f)
			return construct_intersection(p1, r1, q1, r2, p2, q2, N1, N2, v1, v2, N, v, alpha, l_source, l_target);
		else
		{ // Triangles are co-planar
			coplanar = true;
			return coplanar_tri_tri_3D(p1, q1, r1, p2, q2, r2, N1);
		}
	}

	bool tri_tri_get_intersection(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& r1,
	                              const glm::vec3& p2, const glm::vec3& q2, const glm::vec3& r2,
	                              bool& coplanar, glm::vec3& l_source, glm::vec3& l_target)
	{
		float dp1, dq1, dr1, dp2, dq2, dr2;
		glm::vec3 v1, v2, v;
		glm::vec3 N1, N2, N;
		float alpha;

		// Compute distance signs of p1, q1 and r1 to the plane of triangle(p2,q2,r2)
		v1 = p2 - r2;
		v2 = q2 - r2;
		N2 = glm::cross(v1, v2);

		v1  = p1 - r2;
		dp1 = glm::dot(v1, N2);
		v1  = q1 - r2;
		dq1 = glm::dot(v1, N2);
		v1  = r1 - r2;
		dr1 = glm::dot(v1, N2);

		// Coplanarity robustness check
		if constexpr (USE_EPSILON_TEST_TRI_TRI)
		{
			constexpr auto EPSILON_TRI_TRI = std::numeric_limits<decltype(dp1)>::epsilon();
			if (std::abs(dp1) < EPSILON_TRI_TRI) dp1 = 0.0f;
			if (std::abs(dq1) < EPSILON_TRI_TRI) dq1 = 0.0f;
			if (std::abs(dr1) < EPSILON_TRI_TRI) dr1 = 0.0f;
		}

		if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))
			return false;

		// Compute distance signs of p2, q2 and r2 to the plane of triangle(p1,q1,r1)
		v1 = q1 - p1;
		v2 = r1 - p1;
		N1 = glm::cross(v1, v2);

		v1  = p2 - r1;
		dp2 = glm::dot(v1, N1);
		v1  = q2 - r1;
		dq2 = glm::dot(v1, N1);
		v1  = r2 - r1;
		dr2 = glm::dot(v1, N1);

		if constexpr (USE_EPSILON_TEST_TRI_TRI)
		{
			constexpr auto EPSILON_TRI_TRI = std::numeric_limits<decltype(dp2)>::epsilon();
			if (std::abs(dp2) < EPSILON_TRI_TRI) dp2 = 0.0f;
			if (std::abs(dq2) < EPSILON_TRI_TRI) dq2 = 0.0f;
			if (std::abs(dr2) < EPSILON_TRI_TRI) dr2 = 0.0f;
		}

		if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f))
			return false;

		// Permutation in a canonical form of T1's vertices
		if (dp1 > 0.0f)
			if (dq1 > 0.0f)
				return tri_tri_inter_3D(r1, p1, q1, p2, r2, q2, dp2, dr2, dq2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
			else if (dr1 > 0.0f)
				return tri_tri_inter_3D(q1, r1, p1, p2, r2, q2, dp2, dr2, dq2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
			else
				return tri_tri_inter_3D(p1, q1, r1, p2, q2, r2, dp2, dq2, dr2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
		else if (dp1 < 0.0f)
			if (dq1 < 0.0f)
				return tri_tri_inter_3D(r1, p1, q1, p2, q2, r2, dp2, dq2, dr2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
			else if (dr1 < 0.0f)
				return tri_tri_inter_3D(q1, r1, p1, p2, q2, r2, dp2, dq2, dr2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
			else
				return tri_tri_inter_3D(p1, q1, r1, p2, r2, q2, dp2, dr2, dq2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
		else if (dq1 < 0.0f)
			if (dr1 >= 0.0f)
				return tri_tri_inter_3D(q1, r1, p1, p2, r2, q2, dp2, dr2, dq2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
			else
				return tri_tri_inter_3D(p1, q1, r1, p2, q2, r2, dp2, dq2, dr2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
		else if (dq1 > 0.0f)
			if (dr1 > 0.0f)
				return tri_tri_inter_3D(p1, q1, r1, p2, r2, q2, dp2, dr2, dq2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
			else
				return tri_tri_inter_3D(q1, r1, p1, p2, q2, r2, dp2, dq2, dr2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
		else if (dr1 > 0.0f)
			return tri_tri_inter_3D(r1, p1, q1, p2, q2, r2, dp2, dq2, dr2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
		else if (dr1 < 0.0f)
			return tri_tri_inter_3D(r1, p1, q1, p2, r2, q2, dp2, dr2, dq2, N1, N2, v1, v2, N, v, alpha, coplanar, l_source, l_target);
		else
		{ // Triangles are co-planar
			coplanar = true;
			return coplanar_tri_tri_3D(p1, q1, r1, p2, q2, r2, N1);
		}
	};
} // namespace Geometry