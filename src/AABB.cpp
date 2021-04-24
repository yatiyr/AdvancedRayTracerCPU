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

bool AABB::Intersect(const Ray& r)
{

    float t1 = (xmin - r.origin.x)*r.rcp.x;
	float t2 = (xmax - r.origin.x)*r.rcp.x;

	float tMin = std::min(t1, t2);
	float tMax = std::max(t1, t2);

	t1 = (ymin - r.origin.y)*r.rcp.y;
	t2 = (ymax - r.origin.y)*r.rcp.y;

	tMin = std::max(tMin, std::min(t1, t2));
	tMax = std::min(tMax, std::max(t1, t2));

	t1 = (zmin - r.origin.z)*r.rcp.z;
	t2 = (zmax - r.origin.z)*r.rcp.z;

	tMin = std::max(tMin, std::min(t1, t2));
	tMax = std::min(tMax, std::max(t1, t2));	

	return tMax >= std::max(tMin, 0.0f);
}
