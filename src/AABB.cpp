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

}

bool AABB::Intersect(const Ray& r, float tmin, float tmax)
{
    float t1x = glm::clamp((xmin - r.origin.x) / r.direction.x, tmin, tmax);
    float t2x = glm::clamp((xmax - r.origin.x) / r.direction.x, tmin, tmax);

	if(t1x > t2x)
	{
		float tmp = t1x;
		t1x = t2x;
		t2x = tmp;
	}

    float t1y = glm::clamp((ymin - r.origin.x) / r.direction.y, tmin, tmax);
    float t2y = glm::clamp((ymax - r.origin.x) / r.direction.y, tmin, tmax);

	if(t1y > t2y)
	{
		float tmp = t1y;
		t1y = t2y;
		t2y = tmp;
	}

	float t1z = glm::clamp((zmin - r.origin.z) / r.direction.z, tmin, tmax);
	float t2z = glm::clamp((zmax - r.origin.z) / r.direction.z, tmin, tmax); 

	if(t1z > t2z)
	{
		float tmp = t1z;
		t1z = t2z;
		t2z = tmp;
	}

	float t1max = t1x;
	float t2min = t2x;

	if(t1max < t1y)
		t1max = t1y;
	if(t1max < t1z)
		t1max = t1z;

	if(t2min > t2y)
		t2min = t2y;
	if(t2min > t2z)
		t2min = t2z;

	if(t1max > t2min)
	{	
		return false;
	}
	else if(t1max < t2min)
	{
		return true;
	}
	else if(t1max == t2min)
	{
		return true;
	}
	else
	{
		return false;
	}    
}
