#include "Frustrum.hpp"

#include "glm/mat4x4.hpp"

namespace Geometry
{
    Frustrum::Frustrum(const glm::mat4& p_projection) noexcept
        : m_left{  glm::vec4{p_projection[0][3] + p_projection[0][0], p_projection[1][3] + p_projection[1][0], p_projection[2][3] + p_projection[2][0], p_projection[3][3] + p_projection[3][0]}}
        , m_right{ glm::vec4{p_projection[0][3] - p_projection[0][0], p_projection[1][3] - p_projection[1][0], p_projection[2][3] - p_projection[2][0], p_projection[3][3] - p_projection[3][0]}}
        , m_bottom{glm::vec4{p_projection[0][3] + p_projection[0][1], p_projection[1][3] + p_projection[1][1], p_projection[2][3] + p_projection[2][1], p_projection[3][3] + p_projection[3][1]}}
        , m_top{   glm::vec4{p_projection[0][3] - p_projection[0][1], p_projection[1][3] - p_projection[1][1], p_projection[2][3] - p_projection[2][1], p_projection[3][3] - p_projection[3][1]}}
        // Inverting near and far normal to maintain the conventon of Frustrum that all plane normals point inside the space.
        , m_near{  glm::vec4{-(p_projection[0][3] + p_projection[0][2]), -(p_projection[1][3] + p_projection[1][2]), -(p_projection[2][3] + p_projection[2][2]), -(p_projection[3][3] + p_projection[3][2])}}
        , m_far{   glm::vec4{-(p_projection[0][3] - p_projection[0][2]), -(p_projection[1][3] - p_projection[1][2]), -(p_projection[2][3] - p_projection[2][2]), -(p_projection[3][3] - p_projection[3][2])}}
    {
        m_left.normalise();
        m_right.normalise();
        m_bottom.normalise();
        m_top.normalise();
        m_near.normalise();
        m_far.normalise();
    }
}