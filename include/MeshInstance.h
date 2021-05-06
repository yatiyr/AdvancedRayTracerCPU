#ifndef __MESH_INSTANCE_H__
#define __MESH_INSTANCE_H__

#include <Object.h>
#include <Mesh.h>

class MeshInstance : public Object
{
private:

public:
    Mesh* mesh;

    MeshInstance(Mesh* mesh, bool resetTransform);

};

#endif /* __MESH_INSTANCE_H__ */