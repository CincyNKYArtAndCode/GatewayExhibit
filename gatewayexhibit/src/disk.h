//
//  disk.hpp
//  gateway
//
//  Created by Patrick Down on 11/4/18.
//

#ifndef disk_hpp
#define disk_hpp

#include "ofMain.h"
#include <vector>


struct FlyingTriangle {
    
    glm::vec3 position;
    glm::vec3 edge1;
    glm::vec3 edge2;
    
    glm::vec2 normXNoise;
    glm::vec2 normYNoise;
    glm::vec2 normZNoise;
    glm::vec2 distNoise;
};

using FlyingTriangleArr = vector<FlyingTriangle>;

class Disk : public ofNode {
public:
    Disk(glm::vec3 center, float innerRadius, float outerRadius, int numSegments, int numRings);
    
    void setRotationSpeed(float value) { rotationSpeed = value; }
    void update(float t);
    void customDraw() override;
    
    glm::vec3 getCenter() { return center; }
    
private:
    float expiration;
    float innerRadius;
    float outerRadius;
    float prevTime;
    float rotationSpeed;
    float disorder;
    float disorderChangeRate;
    float angle;
    glm::vec3 center;
    ofMaterial material;
    
    unique_ptr<ofMesh> mesh;
    unique_ptr<FlyingTriangleArr> triangles;
    
    void buildDisk(float innerRadius, float outerRadius, int numSegments, int numRings);
    
    void updateMesh(float angle, float t);
    
};

#endif /* disk_hpp */
