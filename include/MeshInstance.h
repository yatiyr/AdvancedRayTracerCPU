#ifndef __MESH_INSTANCE_H__
#define __MESH_INSTANCE_H__

#include <Object.h>
#include <Mesh.h>

class MeshInstance : public Object
{
private:
public:
    Mesh* mesh;
    MeshInstance();
    bool Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling);    

};

#endif /* __MESH_INSTANCE_H__ */