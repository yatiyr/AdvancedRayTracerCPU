#ifndef __BVH_H__
#define __BVH_H__

#include <AABB.h>
#include <Structures.h>
#include <vector>
#include <Triangle.h>

struct VertexIndex
{
    alignas(16) glm::vec3 vertex;
    int index;
};

struct SplittedTriangles
{
    std::vector<Triangle> p1;
    std::vector<Triangle> p2;
};

// Comparator functions
struct CompareX
{
    inline bool operator() (const VertexIndex& lhs, const VertexIndex& rhs) const
    {
        return lhs.vertex.x < rhs.vertex.x;
    }
};

struct CompareY
{
    inline bool operator() (const VertexIndex& lhs, const VertexIndex& rhs) const
    {
        return lhs.vertex.y < rhs.vertex.y;
    }
};

struct CompareZ
{
    inline bool operator() (const VertexIndex& lhs, const VertexIndex& rhs) const
    {
        return lhs.vertex.z < rhs.vertex.z;
    }
};

class BVH
{
private:
    AABB box;
    BVH* leftChild;
    BVH* rightChild;
    std::vector<Triangle> primitives;

    int splitAxis;

    SplittedTriangles splitMidpoint(const std::vector<Triangle>& triangleList);
    SplittedTriangles splitMidpoint(const std::vector<Triangle>& triangleList, int axis);
public:
    BVH(const std::vector<Triangle>& triangleList, int depth, int maxdepth);
    BVH(const std::vector<Triangle>& triangleList, int depth, int maxdepth, int axis);
    
    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon, bool softShadingFlag, const glm::mat4& transformationMatrixTransposed);
};


#endif