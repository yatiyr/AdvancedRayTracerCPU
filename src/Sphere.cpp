#include <Sphere.h>

Sphere::Sphere(glm::vec3 center, float radius, size_t materialId)
{
    this->center = center;
    this->radius = radius;
    this->materialId = materialId;
}

bool Sphere::Intersect(const Ray& r, IntersectionReport& report, float tmin, float tmax)
{
    report.intersection = glm::vec3(-FLT_MAX);
    report.d            = FLT_MAX;

    float discriminant = pow(glm::dot(r.direction, (r.origin - center)), 2) -
                         dot(r.direction, r.direction) * (glm::dot(r.origin - center, r.origin - center) -
                         radius * radius);

    float t;

    if(discriminant >= 0)
    {
        float t1 = -(glm::dot(r.direction, (r.origin - center)) + sqrt(discriminant)) / glm::dot(r.direction, r.direction);
        float t2 = -(glm::dot(r.direction, (r.origin - center)) - sqrt(discriminant)) / glm::dot(r.direction, r.direction);

        t = std::min(t1, t2);

        if(t > tmin && t < tmax)
        {
            report.d            = t;
            report.intersection = r.origin + t*r.direction;
            report.materialId   = materialId;
            report.normal       = glm::normalize(report.intersection - center);
        }

        return true;        
    }

    return false;

}