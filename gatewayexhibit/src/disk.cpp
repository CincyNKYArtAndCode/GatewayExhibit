//
//  disk.cpp
//  gateway
//
//  Created by Patrick Down on 11/4/18.
//

#include "disk.h"
#include "mathutil.h"

inline float getNoise(glm::vec2 sample) {
    return ofNoise(sample) * 2 - 1;
}

Disk::Disk(glm::vec3 center, float innerRadius, float outerRadius, int numSegments, int numRings)
:   center(center),
    angle(0),
    disorder(0),
    disorderChangeRate(0.04f),
    innerRadius(innerRadius),
    outerRadius(outerRadius),
    expiration(0)
{
    buildDisk(innerRadius, outerRadius, numSegments, numRings);
    mesh = make_unique<ofMesh>();
    prevTime = ofGetElapsedTimef();
    
    // shininess is a value between 0 - 128, 128 being the most shiny //
    material.setShininess( 120 );
    material.setDiffuseColor(ofColor(255, 255, 0, 255));
    // the light highlight of the material //
    //material.setSpecularColor(ofColor(255, 255, 255, 255));
    
}

void Disk::update(float t) {
    
    auto diff = glm::abs(t - disorder);
    diff = diff < disorderChangeRate ? disorderChangeRate : diff;
    disorder = glm::lerp(disorder, t, disorderChangeRate / diff);
    float elpasedTime = ofGetElapsedTimef() - prevTime;
    prevTime = ofGetElapsedTimef();
    float rs = glm::lerp(0.0f, rotationSpeed, disorder);
    angle = angle + rs * elpasedTime;
    angle = angle < TAU ? angle : angle - TAU;
    resetTransform();
    auto zChange = glm::lerp(glm::vec3(0, 0, 0), glm::vec3(0, 0, -50), disorder);
    setPosition(center + zChange);
    rotateRad(angle, 0, 0, 1);
    updateMesh(angle, disorder);
    
    ofColor distortedDiff(192, 192, 192, 255);
    ofColor orderedDiff(255, 255, 0, 255);
    
    ofColor distortedAmb(128, 128, 192, 255);
    ofColor orderedAmb(128, 128, 0, 255);

    auto matDiffColor = orderedDiff.getLerped(distortedDiff, disorder);
    auto matAmbColor = orderedAmb.getLerped(distortedAmb, disorder);

    //material.setAmbientColor(matAmbColor);
    material.setDiffuseColor(matDiffColor);
}

void Disk::customDraw() {
    material.begin();
    ofFill();
    mesh->draw();
    material.end();
}

void Disk::buildDisk(float innerRadius, float outerRadius, int numSegments, int numRings) {
    
    triangles = make_unique<FlyingTriangleArr>();
    triangles->reserve(numSegments * numRings);
    
    float ringInc = (outerRadius - innerRadius)/numRings;
    for(int ring = 0; ring < numRings; ++ring) {
        float inr = innerRadius + ring * ringInc;
        float outr = innerRadius + (ring + 1) * ringInc;
        for(int seg = 0; seg < numSegments; ++seg) {
            float angle1 = float(seg)*TAU/numSegments;
            float angle2 = float(seg + 1)*TAU/numSegments;
            float angle3 = float(seg - 1)*TAU/numSegments;
            
            float r1 = (seg + ring) % 2 == 0 ? inr : outr;
            float r2 = (seg + ring) % 2 == 0 ? outr : inr;
            
            auto pt1 = lengthAtAngle(r1, angle1);
            auto pt2 = lengthAtAngle(r2, angle2);
            auto pt3 = lengthAtAngle(r2, angle3);
            
            FlyingTriangle tri;
            tri.position = pt1;
            if((seg + ring) % 2 == 0) {
                tri.edge1 = pt2 - pt1;
                tri.edge2 = pt3 - pt1;
            }
            else {
                tri.edge1 = pt3 - pt1;
                tri.edge2 = pt2 - pt1;
            }
            tri.normXNoise.x = ofRandom(100);
            tri.normXNoise.y = ofRandom(100);
            tri.normYNoise.x = ofRandom(100);
            tri.normYNoise.y = ofRandom(100);
            tri.normZNoise.x = ofRandom(100);
            tri.normZNoise.y = ofRandom(100);
            tri.distNoise.x = ofRandom(100);
            tri.distNoise.y = ofRandom(100);
            
            triangles->push_back(tri);
        }
    }
}

void Disk::updateMesh(float angle, float t) {
    
    mesh->clear();
    if(mesh->getVertices().capacity() < triangles->size() * 3) {
        mesh->getVertices().reserve(triangles->size() * 3);
        mesh->getNormals().reserve(triangles->size() * 3);
        mesh->getIndices().reserve(triangles->size() * 3);
    }
    
    auto zAxis = glm::vec3(0,0,1);
    
    glm::vec2 sample = glm::rotate(glm::vec2(1, 0), angle);
    int i = 0;
    for(auto& tri : *triangles) {
        auto dist = getNoise(tri.distNoise + sample);
        dist = glm::lerp(0.0f, dist, t);
        
        glm::vec3 normal;
        normal.x = getNoise(tri.normXNoise + sample);
        normal.y = getNoise(tri.normYNoise + sample);
        normal.z = ofNoise(tri.normZNoise + sample);
        //normal.z = getNoise(tri.normZNoise + sample);
        
        normal = glm::normalize(normal);
        normal = glm::slerp(zAxis, normal, t);
        
        auto position = tri.position + normal * dist * 40;
        
        auto rot = glm::rotation(zAxis, normal);
        
        auto edge1 = glm::rotate(rot, tri.edge1);
        auto edge2 = glm::rotate(rot, tri.edge2);
        
        mesh->addVertex(position);
        mesh->addVertex(position + edge1);
        mesh->addVertex(position + edge2);
        mesh->addNormal(normal);
        mesh->addNormal(normal);
        mesh->addNormal(normal);
        mesh->addIndex(i++);
        mesh->addIndex(i++);
        mesh->addIndex(i++);
    }
}
