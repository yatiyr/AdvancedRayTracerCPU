#include <Triangle.h>


Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    this->a = a;
    this->b = b;
    this->c = c;

    this->normal = glm::normalize(glm::cross((b-a), (c-a)));
}

bool Triangle::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon)
{

    report.d = FLT_MAX;

	float detA = glm::determinant(glm::mat3(glm::vec3(a.x - b.x, a.y - b.y, a.z - b.z),
	                              glm::vec3(a.x - c.x, a.y - c.y, a.z - c.z),
								  glm::vec3(ray.direction.x, ray.direction.y, ray.direction.z)));


	float beta = glm::determinant(glm::mat3(glm::vec3(a.x - ray.origin.x, a.y - ray.origin.y, a.z - ray.origin.z),
	                              glm::vec3(a.x - c.x, a.y - c.y, a.z - c.z),
								  glm::vec3(ray.direction.x, ray.direction.y, ray.direction.z)))/detA;

	float gamma = glm::determinant(glm::mat3(glm::vec3(a.x - b.x, a.y - b.y, a.z - b.z),
	                               glm::vec3(a.x - ray.origin.x, a.y - ray.origin.y, a.z - ray.origin.z),
								   glm::vec3(ray.direction.x, ray.direction.y, ray.direction.z)))/detA;


	float t = glm::determinant(glm::mat3(glm::vec3(a.x - b.x, a.y - b.y, a.z - b.z),
	                           glm::vec3(a.x - c.x, a.y - c.y, a.z - c.z),
							   glm::vec3(a.x - ray.origin.x, a.y - ray.origin.y, a.z - ray.origin.z)))/detA;

    
	if((beta + gamma <= 1 + intersectionTestEpsilon) && (beta >= -intersectionTestEpsilon) && (gamma >= -intersectionTestEpsilon) && t>tmin && t<tmax)
	{
        report.d            = t;
		report.intersection = ray.origin + t*ray.direction;
        report.normal       = this->normal;
		report.materialId   = materialId;
        return true;
	}

	return false;     
}

glm::vec3 Triangle::GiveCenter() const
{
    glm::vec3 result = (a + b + c);
    result.x /= 3;
    result.y /= 3;
    result.z /= 3;

    return result;
}