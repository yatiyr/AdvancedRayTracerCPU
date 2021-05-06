#include <BVH.h>



SplittedTriangles BVH::splitMidpoint(const std::vector<Triangle>& triangleList, int axis)
{
    SplittedTriangles splittedResult;

    glm::vec3 totalVec(0.0, 0.0, 0.0);
    std::vector<VertexIndex> vivec;

    for(size_t i=0; i<triangleList.size(); i++)
    {
        glm::vec3 v = triangleList[i].GiveCenter();
        totalVec += v;

        VertexIndex vi;
        vi.vertex = v;
        vi.index  = i;
        vivec.push_back(vi);
    }

    totalVec /= triangleList.size();


    if(axis == 0)
    {
        for(size_t i=0; i<vivec.size(); i++)
        {
            if(vivec[i].vertex.x <= totalVec.x)
            {
                splittedResult.p1.push_back(triangleList[vivec[i].index]);
            }
            else
            {
                splittedResult.p2.push_back(triangleList[vivec[i].index]);
            }
        }
    }
    else if(axis == 1)
    {
        for(size_t i=0; i<vivec.size(); i++)
        {
            if(vivec[i].vertex.y <= totalVec.y)
            {
                splittedResult.p1.push_back(triangleList[vivec[i].index]);                }
            else
            {
                splittedResult.p2.push_back(triangleList[vivec[i].index]);
            }
        }
    }
    else if(axis == 2)
    {
        for(size_t i=0; i<vivec.size(); i++)
        {
            if(vivec[i].vertex.z <= totalVec.z)
            {
                splittedResult.p1.push_back(triangleList[vivec[i].index]);
            }
            else
            {
                splittedResult.p2.push_back(triangleList[vivec[i].index]);
            }
        }
    }

    return splittedResult;    
}


SplittedTriangles BVH::splitMidpoint(const std::vector<Triangle>& triangleList)
{
    SplittedTriangles splittedResult;

    float xlength = box.xmax - box.xmin;
    float ylength = box.ymax - box.ymin;
    float zlength = box.zmax - box.zmin;

    glm::vec3 totalVec(0.0, 0.0, 0.0);
    std::vector<VertexIndex> vivec;

    for(size_t i=0; i<triangleList.size(); i++)
    {
        glm::vec3 v = triangleList[i].GiveCenter();
        totalVec += v;

        VertexIndex vi;
        vi.vertex = v;
        vi.index  = i;
        vivec.push_back(vi);
    }

    totalVec /= triangleList.size();

        if(xlength >= ylength && xlength >= zlength)
        {
            for(size_t i=0; i<vivec.size(); i++)
            {
                if(vivec[i].vertex.x <= totalVec.x)
                {
                    splittedResult.p1.push_back(triangleList[vivec[i].index]);
                }
                else
                {
                    splittedResult.p2.push_back(triangleList[vivec[i].index]);
                }
            }
        }
        else if(ylength >= xlength && ylength >= zlength)
        {
            for(size_t i=0; i<vivec.size(); i++)
            {
                if(vivec[i].vertex.y <= totalVec.y)
                {
                    splittedResult.p1.push_back(triangleList[vivec[i].index]);
                }
                else
                {
                    splittedResult.p2.push_back(triangleList[vivec[i].index]);
                }
            }
        }
        else if(zlength >= ylength && zlength >= xlength)
        {
            for(size_t i=0; i<vivec.size(); i++)
            {
                if(vivec[i].vertex.z <= totalVec.z)
                {
                    splittedResult.p1.push_back(triangleList[vivec[i].index]);
                }
                else
                {
                    splittedResult.p2.push_back(triangleList[vivec[i].index]);
                }
            }
        }

        return splittedResult;    
}

BVH::BVH(const std::vector<Triangle>& triangleList, int depth, int maxdepth) : box(triangleList)
{
    if(depth == maxdepth || triangleList.size() <= 1)
    {

        for(size_t i=0; i<triangleList.size(); i++)
        {
            this->primitives.push_back(triangleList[i]);
        }

        this->leftChild  = nullptr;
        this->rightChild = nullptr;

        return;
    }

    SplittedTriangles st = splitMidpoint(triangleList);

    if(st.p1.empty() || st.p2.empty())
    {
        for(size_t i=0; i<triangleList.size(); i++)
        {
            this->primitives.push_back(triangleList[i]);
        }

        this->leftChild  = nullptr;
        this->rightChild = nullptr;

        return;
    }

    this->leftChild = new BVH(st.p1, depth + 1, maxdepth);
    this->rightChild = new BVH(st.p2, depth + 1, maxdepth);
}

BVH::BVH(const std::vector<Triangle>& triangleList, int depth, int maxdepth, int axis) : box(triangleList)
{
    if(depth == maxdepth || triangleList.size() <= 1)
    {

        for(size_t i=0; i<triangleList.size(); i++)
        {
            this->primitives.push_back(triangleList[i]);
        }

        this->leftChild  = nullptr;
        this->rightChild = nullptr;

        return;
    }

    SplittedTriangles st = splitMidpoint(triangleList);

    if(st.p1.empty() || st.p2.empty())
    {
        for(size_t i=0; i<triangleList.size(); i++)
        {
            this->primitives.push_back(triangleList[i]);
        }

        this->leftChild  = nullptr;
        this->rightChild = nullptr;

        return;
    }

    this->leftChild = new BVH(st.p1, depth + 1, maxdepth, (axis+1)%3);
    this->rightChild = new BVH(st.p2, depth + 1, maxdepth, (axis+1)%3);
}

bool BVH::Intersect(const Ray& ray, IntersectionReport& report, float tmin, float tmax, float intersectionTestEpslion, bool softShadingFlag, const glm::mat4& transformationMatrixTransposed)
{
    bool boxTest = box.Intersect2(ray, tmin, tmax);

    if(!boxTest)
        return false;

    IntersectionReport report1, report2;

    report.d = FLT_MAX;

    if(this->leftChild == nullptr && this->rightChild == nullptr)
    {
        for(size_t i=0; i<primitives.size(); i++)
        {
            IntersectionReport r;
            bool hitTest = primitives[i].Intersect(ray, r, tmin, tmax, intersectionTestEpslion, softShadingFlag, transformationMatrixTransposed);
            if(hitTest)
            {
                report = r.d < report.d ? r : report;
            }
            
        }

        return report.d != FLT_MAX;
    }    

    bool leftTest  = leftChild ->Intersect(ray, report1, tmin, tmax, intersectionTestEpslion, softShadingFlag, transformationMatrixTransposed);
    bool rightTest = rightChild->Intersect(ray, report2, tmin, tmax, intersectionTestEpslion, softShadingFlag, transformationMatrixTransposed);

    if(leftTest)
    {
        report = report1;
    }
    
    if(rightTest)
    {
        report = report2.d < report.d ? report2 : report;
    }

    return (leftTest || rightTest);
    
}