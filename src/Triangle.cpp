#include <Triangle.h>


Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    this->a = a;
    this->b = b;
    this->c = c;

    this->normal = glm::normalize(glm::cross((b-a), (c-a)));
}

Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 aNormal, glm::vec3 bNormal, glm::vec3 cNormal)
{
	this->a = a;
	this->b = b;
	this->c = c;

	this->aNormal = aNormal;
	this->bNormal = bNormal;
	this->cNormal = cNormal;

    this->normal = glm::normalize(glm::cross((b-a), (c-a)));	
}

bool Triangle::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon, bool softShadingFlag)
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

		if(softShadingFlag)
		{
			float alpha = 1 - (beta + gamma);
			glm::vec3 normal = glm::normalize(alpha*aNormal + beta*bNormal + gamma*cNormal);
			report.normal = normal;
		}
		else
		{
        	report.normal       = this->normal;
		}
		report.materialId   = materialId;
        return true;
	}

	return false;     
}

// TODO: PRODUCES WEIRD IMAGES
bool Triangle::FasterIntersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon)
{
	glm::vec3 v0v1 = b - a;
	glm::vec3 v0v2 = c - a;
	glm::vec3 pVec = glm::cross(ray.direction, v0v2);
	float det = glm::dot(v0v1, pVec);

#ifdef CULLING

	if(det < intersectionTestEpsilon)
		return false;

#else
	if (std::fabs(det) < intersectionTestEpsilon)
		return false;
#endif

	float invDet = 1 / det;

	glm::vec3 tvec = ray.origin - a;
	float u = glm::dot(tvec, pVec) * invDet;

	if(u<0 || u>1)
		return false;

	glm::vec3 qvec = glm::cross(tvec, v0v1);
	float v = glm::dot(ray.direction, pVec) * invDet;

	if(v<0 || u + v > 1)
		return false;

	float t = glm::dot(v0v2, qvec) * invDet;

	if(t < tmin || t > tmax)
		return false;
		
    report.d            = t;
	report.intersection = ray.origin + t*ray.direction;
    report.normal       = this->normal;
	report.materialId   = materialId;

	return true;

}

glm::vec3 Triangle::GiveCenter() const
{
    glm::vec3 result = (a + b + c);
    result.x /= 3;
    result.y /= 3;
    result.z /= 3;

    return result;
}