#include <Sphere.h>

Sphere::Sphere(glm::vec3 center, float radius, size_t materialId)
{
    this->center = center;
    this->radius = radius;
    this->materialId = materialId;
}

bool Sphere::solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discriminant = b*b -4*a*c;
    if(discriminant < 0)
        return false;
    else if(discriminant == 0)
        x0 = x1 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discriminant)) :
            -0.5 * (b - sqrt(discriminant));
        x0 = q / a;
        x1 = c / q;
    }

    if (x0 > x1)
        std::swap(x0, x1);

    return true;
        
}

bool Sphere::Intersect(const Ray& r, IntersectionReport& report, float tmin, float tmax, float intersectionEpsilon, bool backfaceCulling)
{

    glm::mat4 motionBlurTranslationMatrix = MotionBlurTranslate(r.time);

    glm::vec3 newOrigin = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(r.origin, 1.0f));
    glm::vec3 newDirection = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(r.direction, 0.0f));
    Ray newRay(newOrigin, newDirection);

    report.intersection = glm::vec3(-FLT_MAX);
    report.d            = FLT_MAX;

    float discriminant = pow(glm::dot(newRay.direction, (newRay.origin - center)), 2) -
                         dot(newRay.direction, newRay.direction) * (glm::dot(newRay.origin - center, newRay.origin - center) -
                         radius * radius);

    float t;

    if(discriminant >= 0)
    {
        float t1 = -(glm::dot(newRay.direction, (newRay.origin - center)) + sqrt(discriminant)) / glm::dot(newRay.direction, newRay.direction);
        float t2 = -(glm::dot(newRay.direction, (newRay.origin - center)) - sqrt(discriminant)) / glm::dot(newRay.direction, newRay.direction);

        t = std::min(t1, t2);

        if(t1 * t2 < 0)
            t = std::max(t1, t2);


        if(t > tmin && t < tmax)
        {
            report.d            = t;
            report.intersection = r.origin + t*r.direction;
            report.materialId   = materialId;
            glm::vec3 normal = (newRay.origin + t*newRay.direction) - center;
            report.normal       = glm::transpose(motionBlurTranslationMatrix)*transformationMatrixInverseTransposed * glm::vec4(normal, 0.0f);
            report.normal = glm::normalize(report.normal);
            return true;
        }
      
    }

    return false;

}