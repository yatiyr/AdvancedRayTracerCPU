#ifndef __PERLIN_NOISE_GENERATOR_H__
#define __PERLIN_NOISE_GENERATOR_H__

#include <glm/glm.hpp>
#include <array>
#include <algorithm>
#include <bits/stdc++.h>
#include <math.h>

class PerlinNoiseGenerator
{
public:

    std::array<glm::vec3, 16> gradientVectorArray = {glm::vec3(1,1,0), glm::vec3(-1,1,0), glm::vec3(1,-1,0), glm::vec3(-1,-1,0),
                                                     glm::vec3(1,0,1), glm::vec3(-1,0,1), glm::vec3(1,0,-1), glm::vec3(-1,0,-1),
                                                     glm::vec3(0,1,1), glm::vec3(0,-1,1), glm::vec3(0,1,-1), glm::vec3(0,-1,-1),
                                                     glm::vec3(1,1,0), glm::vec3(-1,1,0), glm::vec3(0,-1,1), glm::vec3(0,-1,-1)};

    std::array<int, 16> shuffledArray = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    float scalingFactor = 1.0f;

    PerlinNoiseGenerator()
    {
        unsigned seed = 0;
        std::shuffle(shuffledArray.begin(), shuffledArray.end(), std::default_random_engine(seed));
    }

    PerlinNoiseGenerator(float scalingFactor)
    {
        unsigned seed = 0;
        std::shuffle(shuffledArray.begin(), shuffledArray.end(), std::default_random_engine(seed));
        this->scalingFactor = scalingFactor;
    }    

    int GetIndex(int i)
    {
        return shuffledArray[std::abs(i) % 16];
    }

    float Weight(float x)
    {
        float absX = std::fabs(x);

        if(absX < 1)
            return -6 * std::pow(absX,5) + 15 * std::pow(absX,4) - 10 * std::pow(absX, 3) + 1;
        return 0;
    }

    glm::vec3 GetEdge(int i, int j, int k)
    {
        return gradientVectorArray[GetIndex(i + GetIndex(j + GetIndex(k)))];
    }

    float GenerateNoise(glm::vec3 point)
    {
        // left-down-back
        glm::vec3 p1(std::floor(point.x), std::floor(point.y), std::floor(point.z));

        // right-down-back
        glm::vec3 p2(std::ceil(point.x), std::floor(point.y), std::floor(point.z));

        // right-down-front
        glm::vec3 p3(std::ceil(point.x), std::floor(point.y), std::ceil(point.z));

        // left-down-front
        glm::vec3 p4(std::floor(point.x), std::floor(point.y), std::ceil(point.z));

        // left-up-back
        glm::vec3 p5(std::floor(point.x), std::ceil(point.y), std::floor(point.z));

        // right-up-back
        glm::vec3 p6(std::ceil(point.x), std::ceil(point.y), std::floor(point.z));

        // right-up-front
        glm::vec3 p7(std::ceil(point.x), std::ceil(point.y), std::ceil(point.z));

        // left-up-front
        glm::vec3 p8(std::floor(point.x), std::ceil(point.y), std::ceil(point.z));


        glm::vec3 e1 = GetEdge(p1.x, p1.y, p1.z);
        glm::vec3 e2 = GetEdge(p2.x, p2.y, p2.z);
        glm::vec3 e3 = GetEdge(p3.x, p3.y, p3.z);
        glm::vec3 e4 = GetEdge(p4.x, p4.y, p4.z);
        glm::vec3 e5 = GetEdge(p5.x, p5.y, p5.z);
        glm::vec3 e6 = GetEdge(p6.x, p6.y, p6.z);
        glm::vec3 e7 = GetEdge(p7.x, p7.y, p7.z);
        glm::vec3 e8 = GetEdge(p8.x, p8.y, p8.z);

        glm::vec3 v1 = point - p1;
        glm::vec3 v2 = point - p2;  
        glm::vec3 v3 = point - p3;  
        glm::vec3 v4 = point - p4;  
        glm::vec3 v5 = point - p5;  
        glm::vec3 v6 = point - p6;  
        glm::vec3 v7 = point - p7;  
        glm::vec3 v8 = point - p8;  

        float value = glm::dot(e1,v1) * Weight(v1.x) * Weight(v1.y) * Weight(v1.z) +
                      glm::dot(e2,v2) * Weight(v2.x) * Weight(v2.y) * Weight(v2.z) + 
                      glm::dot(e3,v3) * Weight(v3.x) * Weight(v3.y) * Weight(v3.z) + 
                      glm::dot(e4,v4) * Weight(v4.x) * Weight(v4.y) * Weight(v4.z) +
                      glm::dot(e5,v5) * Weight(v5.x) * Weight(v5.y) * Weight(v5.z) +
                      glm::dot(e6,v6) * Weight(v6.x) * Weight(v6.y) * Weight(v6.z) +
                      glm::dot(e7,v7) * Weight(v7.x) * Weight(v7.y) * Weight(v7.z) +
                      glm::dot(e8,v8) * Weight(v8.x) * Weight(v8.y) * Weight(v8.z);

        return value;
        
    }


};


#endif