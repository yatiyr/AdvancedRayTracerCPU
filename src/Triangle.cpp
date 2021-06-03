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

bool Triangle::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon, bool backfaceCulling)
{

	glm::mat4 motionBlurTranslationMatrix = MotionBlurTranslate(ray.time);

    glm::vec3 newOrigin = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(ray.origin, 1.0f));
    glm::vec3 newDirection = (transformationMatrixInversed*motionBlurTranslationMatrix*glm::vec4(ray.direction, 0.0f));
    Ray newRay(newOrigin, newDirection);

    report.d = FLT_MAX;

	float detA = glm::determinant(glm::mat3(glm::vec3(a.x - b.x, a.y - b.y, a.z - b.z),
	                              glm::vec3(a.x - c.x, a.y - c.y, a.z - c.z),
								  glm::vec3(newRay.direction.x, newRay.direction.y, newRay.direction.z)));


	float beta = glm::determinant(glm::mat3(glm::vec3(a.x - newRay.origin.x, a.y - newRay.origin.y, a.z - newRay.origin.z),
	                              glm::vec3(a.x - c.x, a.y - c.y, a.z - c.z),
								  glm::vec3(newRay.direction.x, newRay.direction.y, newRay.direction.z)))/detA;

	float gamma = glm::determinant(glm::mat3(glm::vec3(a.x - b.x, a.y - b.y, a.z - b.z),
	                               glm::vec3(a.x - newRay.origin.x, a.y - newRay.origin.y, a.z - newRay.origin.z),
								   glm::vec3(newRay.direction.x, newRay.direction.y, newRay.direction.z)))/detA;


	float t = glm::determinant(glm::mat3(glm::vec3(a.x - b.x, a.y - b.y, a.z - b.z),
	                           glm::vec3(a.x - c.x, a.y - c.y, a.z - c.z),
							   glm::vec3(a.x - newRay.origin.x, a.y - newRay.origin.y, a.z - newRay.origin.z)))/detA;

    
	if((beta + gamma <= 1 + intersectionTestEpsilon) && (beta >= -intersectionTestEpsilon) && (gamma >= -intersectionTestEpsilon) && t>tmin && t<tmax)
	{

		float alpha = 1 - (beta + gamma);
				
        report.d            = t;
		report.intersection = ray.origin + t*ray.direction;
        report.normal       = glm::transpose(motionBlurTranslationMatrix)*this->transformationMatrixInverseTransposed * glm::vec4(this->normal, 0.0f);
		report.normal       = glm::normalize(report.normal);

		report.texCoordA = texCoordA;
		report.texCoordB = texCoordB;
		report.texCoordC = texCoordC;

		report.coordA = a;
		report.coordB = b;
		report.coordC = c;
		report.texCoord = alpha*texCoordA + beta*texCoordB + gamma*texCoordC;

		if(backfaceCulling)
		{
			if(glm::dot(ray.direction, normal) > 0)
				return false;
		}

        if(this->diffuseMap)
        {
            report.replaceAll = false;
            if(this->diffuseMap->decalMode == DecalMode::REPLACE_ALL)
                report.replaceAll = true;
            else if(this->diffuseMap->decalMode == DecalMode::REPLACE_KD)
                report.texDiffuseKdMode = 1;
            else if(this->diffuseMap->decalMode == DecalMode::BLEND_KD)
                report.texDiffuseKdMode = 2;

            if(this->diffuseMap->type == TextureType::IMAGE)
            {
                report.texDiffuseReflectance = this->diffuseMap->Fetch(report.texCoord.x, report.texCoord.y);
            }
            else if(this->diffuseMap->type == TextureType::PERLIN)
            {

                float perlin = this->diffuseMap->FetchPerlin(report.intersection);
                // Veiny appearence
                if(this->diffuseMap->noiseConversion == NoiseConversionType::ABSVAL)
                {
                    report.texDiffuseReflectance = glm::vec3(std::fabs(perlin));
                }
                // Patch appearence
                else if(this->diffuseMap->noiseConversion == NoiseConversionType::LINEAR)
                {
                    report.texDiffuseReflectance = glm::vec3((perlin + 1)/2);
                }
            }
            else if(this->diffuseMap->type == TextureType::CHECKERBOARD)
            {
                // Will be implemented
            } 
        }
        
        if(this->specularMap)
        {

            if(this->specularMap->decalMode == DecalMode::REPLACE_KD)
                report.texSpecularKdMode = 1;
            else if(this->specularMap->decalMode == DecalMode::BLEND_KD)
                report.texSpecularKdMode = 2;

            if(this->specularMap->type == TextureType::IMAGE)
            {
                report.texSpecularReflectance = this->specularMap->Fetch(report.texCoord.x, report.texCoord.y);
            }
            else if(this->diffuseMap->type == TextureType::PERLIN)
            {

                float perlin = this->diffuseMap->FetchPerlin(report.intersection);
                // Veiny appearence
                if(this->diffuseMap->noiseConversion == NoiseConversionType::ABSVAL)
                {
                    report.texDiffuseReflectance = glm::vec3(std::fabs(perlin));
                }
                // Patch appearence
                else if(this->diffuseMap->noiseConversion == NoiseConversionType::LINEAR)
                {
                    report.texDiffuseReflectance = glm::vec3((perlin + 1)/2);
                }
            }
            else if(this->diffuseMap->type == TextureType::CHECKERBOARD)
            {
                // Will be implemented
            } 
        }

        if(this->normalMap)
        {
            glm::vec3 fetchedNormal = this->normalMap->Fetch(report.texCoord.x, report.texCoord.y);
            fetchedNormal.x /= 255;
            fetchedNormal.y /= 255;
            fetchedNormal.z /= 255;

            fetchedNormal -= glm::vec3(0.5, 0.5, 0.5);
            fetchedNormal = glm::normalize(fetchedNormal);

            glm::vec3 e1 = report.coordB - report.coordA;
            glm::vec3 e2 = report.coordC - report.coordA;

            glm::mat2 A_Inverse = glm::inverse(glm::mat2(glm::vec2(report.texCoordB.x - report.texCoordA.x, report.texCoordC.x - report.texCoordA.x),
                                                         glm::vec2(report.texCoordB.y - report.texCoordA.y, report.texCoordC.y - report.texCoordA.y)));

            glm::mat3x2 E(glm::vec2(e1.x, e2.x),
                          glm::vec2(e1.y, e2.y),
                          glm::vec2(e1.z, e2.z));

            glm::mat2x3 TB = glm::transpose(A_Inverse * E);
            glm::mat3 TBN(TB[0], TB[1], report.normal);
            report.normal = TBN * fetchedNormal;
        }
        else if(this->bumpMap)
        {
            if(this->bumpMap->type != TextureType::PERLIN)
            {
                glm::vec3 e1 = report.coordB - report.coordA;
                glm::vec3 e2 = report.coordC - report.coordA;

                glm::mat2 A_Inverse = glm::inverse(glm::mat2(glm::vec2(report.texCoordB.x - report.texCoordA.x, report.texCoordC.x - report.texCoordA.x),
                                                            glm::vec2(report.texCoordB.y - report.texCoordA.y, report.texCoordC.y - report.texCoordA.y)));

                glm::mat3x2 E(glm::vec2(e1.x, e2.x),
                            glm::vec2(e1.y, e2.y),
                            glm::vec2(e1.z, e2.z));

                glm::mat2x3 TB = glm::transpose(A_Inverse * E);

                int i = report.texCoord.x * this->bumpMap->image->width;
                int j = report.texCoord.y * this->bumpMap->image->height;
                float du = glm::length(this->bumpMap->image->get(i+1,j) - this->bumpMap->image->get(i,j));
                float dv = glm::length(this->bumpMap->image->get(i,j+1) - this->bumpMap->image->get(i,j));

                glm::vec3 tangentPlaneVec = du * TB[0] + dv * TB[1];
                tangentPlaneVec = glm::normalize(tangentPlaneVec);

                report.normal = report.normal - this->bumpMap->bumpFactor*(tangentPlaneVec);
            }
            else
            {
                // perlin gradient
                float epsilon = 0.001;
                float xDiff = this->bumpMap->FetchPerlin(glm::vec3(report.intersection.x + epsilon, report.intersection.y, report.intersection.z));
                float yDiff = this->bumpMap->FetchPerlin(glm::vec3(report.intersection.x, report.intersection.y + epsilon, report.intersection.z));
                float zDiff = this->bumpMap->FetchPerlin(glm::vec3(report.intersection.x, report.intersection.y, report.intersection.z + epsilon));
                float perlinNoise = this->bumpMap->FetchPerlin(report.intersection);

                glm::vec3 perlinGradient = glm::vec3(xDiff - perlinNoise,
                                                     yDiff - perlinNoise,
                                                     zDiff - perlinNoise);

                glm::vec3 projectedGradient = perlinGradient - glm::dot(perlinGradient, report.normal) * report.normal;

                report.normal = report.normal - this->bumpMap->bumpFactor * (projectedGradient);
            }
        }

        if(this->emissionMap)
        {

        }

        if(this->roughnessMap)
        {

        }   		

		report.materialId   = materialId;
        return true;
	}

	return false;   	
}

bool Triangle::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpsilon, bool softShadingFlag, const glm::mat4& transformationMatrixInverseTransposed, bool backfaceCulling)
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

		float alpha = 1 - (beta + gamma);

        report.d            = t;
		report.intersection = ray.origin + t*ray.direction;

		report.texCoordA = texCoordA;
		report.texCoordB = texCoordB;
		report.texCoordC = texCoordC;

		report.coordA = a;
		report.coordB = b;
		report.coordC = c;
		report.texCoord = alpha*texCoordA + beta*texCoordB + gamma*texCoordC;


		if(softShadingFlag)
		{
			glm::vec3 normal = glm::normalize(alpha*aNormal + beta*bNormal + gamma*cNormal);
			report.normal = transformationMatrixInverseTransposed * glm::vec4(normal, 0.0f);
			report.normal = glm::normalize(report.normal);

			if(backfaceCulling)
			{
				if(glm::dot(ray.direction, normal) > 0)
					return false;
			}

		}
		else
		{
        	report.normal       = transformationMatrixInverseTransposed * glm::vec4(this->normal, 0.0f);
			report.normal       = glm::normalize(report.normal);

			if(backfaceCulling)
			{
				if(glm::dot(ray.direction, normal) > 0)
					return false;
			}

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