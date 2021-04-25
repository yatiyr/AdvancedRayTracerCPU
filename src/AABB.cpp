#include <AABB.h>


AABB::AABB(const std::vector<Triangle>& triangleList)
{

    this->xmin = FLT_MAX;
    this->ymin = FLT_MAX;
    this->zmin = FLT_MAX;

    this->xmax = -FLT_MAX;
    this->ymax = -FLT_MAX;
    this->zmax = -FLT_MAX;


    for(size_t i=0; i<triangleList.size(); i++)
    {
        float ax = triangleList[i].a.x;
        float bx = triangleList[i].b.x;
        float cx = triangleList[i].c.x;

        float ay = triangleList[i].a.y;
        float by = triangleList[i].b.y;
        float cy = triangleList[i].c.y;

        float az = triangleList[i].a.z;
        float bz = triangleList[i].b.z;
        float cz = triangleList[i].c.z;

            if(ax < xmin)
            {
                xmin = ax;
            }
            if(ax > xmax)
            {
                xmax = ax;
            }

            if(bx < xmin)
            {
                xmin = bx;
            }
            if(bx > xmax)
            {
                xmax = bx;
            }

            if(cx < xmin)
            {
                xmin = cx;
            }
            if(cx > xmax)
            {
                xmax = cx;
            }

        // --------------------------- //


            if(ay < ymin)
            {
                ymin = ay;
            }
            if(ay > ymax)
            {
                ymax = ay;
            }

            if(by < ymin)
            {
                ymin = by;
            }
            if(by > ymax)
            {
                ymax = by;
            }

            if(cy < ymin)
            {
                ymin = cy;
            }
            if(cy > ymax)
            {
                ymax = cy;
            }

        // ------------------------ //


            if(az < zmin)
            {
                zmin = az;
            }
            if(az > zmax)
            {
                zmax = az;
            }

            if(bz < zmin)
            {
                zmin = bz;
            }
            if(bz > zmax)
            {
                zmax = bz;
            }

            if(cz < zmin)
            {
                zmin = cz;
            }
            if(cz > zmax)
            {
                zmax = cz;
            }         

    }

    bounds[0] = glm::vec3(xmin, ymin, zmin);
    bounds[1] = glm::vec3(xmax, ymax, zmax);

}

bool AABB::Intersect(const Ray& r)
{
    float t1 = (xmin - r.origin.x)*r.invDirection.x;
	float t2 = (xmax - r.origin.x)*r.invDirection.x;

	float tMin = std::min(t1, t2);
	float tMax = std::max(t1, t2);

	t1 = (ymin - r.origin.y)*r.invDirection.y;
	t2 = (ymax - r.origin.y)*r.invDirection.y;

	tMin = std::max(tMin, std::min(t1, t2));
	tMax = std::min(tMax, std::max(t1, t2));

	t1 = (zmin - r.origin.z)*r.invDirection.z;
	t2 = (zmax - r.origin.z)*r.invDirection.z;

	tMin = std::max(tMin, std::min(t1, t2));
	tMax = std::min(tMax, std::max(t1, t2));	

	return tMax >= std::max(tMin, 0.0f);
}


bool AABB::Intersect2(const Ray& r, float t0, float t1)
{
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (bounds[r.sign[0]].x - r.origin.x) * r.invDirection.x;
    tmax = (bounds[1 - r.sign[0]].x - r.origin.x) * r.invDirection.x;

    tymin = (bounds[r.sign[1]].y - r.origin.y) * r.invDirection.y;
    tymax = (bounds[1 - r.sign[1]].y - r.origin.y) * r.invDirection.y;

    if((tmin > tymax) || (tymin > tmax))
        return false;
    if(tymin > tmin)
        tmin = tymin;
    if(tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[r.sign[2]].z - r.origin.z) * r.invDirection.z;
    tzmax = (bounds[1 - r.sign[2]].z - r.origin.z) * r.invDirection.z;

    if((tmin > tzmax) || (tzmin > tmax))
        return false;
    if(tzmin > tmin)
        tmin = tzmin;
    if(tzmax < tmax)
        tmax = tzmax;

    return ( (tmin < t1) && (tmax > t0) );
}
