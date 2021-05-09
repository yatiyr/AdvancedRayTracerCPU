#include <MeshInstance.h>


MeshInstance::MeshInstance()
{

}


bool MeshInstance::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling)
{
    glm::vec3 newOrigin = (transformationMatrixInversed * glm::vec4(ray.origin, 1.0f));
    glm::vec3 newDirection = (transformationMatrixInversed * glm::vec4(ray.direction, 0.0f));
    Ray newRay(newOrigin, newDirection);

    bool test = mesh->bvhRoot->Intersect(newRay, report, tmin, tmax, intersectionEpsilon, mesh->softShadingFlag, transformationMatrixInverseTransposed, backfaceCulling);

    report.materialId = materialId;
    if(test)
    {
        report.intersection = ray.origin + ray.direction * report.d;
    }

    return test;
}