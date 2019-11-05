//
//  mathutil.h
//  gateway
//
//  Created by Patrick Down on 11/4/18.
//

#ifndef mathutil_h
#define mathutil_h

const float TAU = 2 * PI;

inline float linear(float v) {
    return v;
}

inline glm::vec3 lengthAtAngle(float length, float angle) {
    glm::vec3 vec(cos(angle) * length, sin(angle)*length, 0);
    return vec;
}

template<typename T, float S(float)>
class Signal {
    float period;
    float phase;
    T begin;
    T end;
    
public:
    Signal() {
        period = 1;
        phase = 0;
        begin = 0;
        end = 1;
    }
    
    Signal(float period, T begin, T end) {
        this->period = period;
        this->phase = glm::fract(ofGetElapsedTimef()/period)*period;
        this->begin = begin;
        this->end = end;
    }
    
    void setPeriod(float value) {
        float fp = glm::fract((ofGetElapsedTimef() + phase)/period);
        period = value;
        phase = value * fp;
    }
    
    float getValue() {
        float t = S(glm::fract((ofGetElapsedTimef() + phase)/period));
        
        return glm::lerp(begin, end, t);
    }
};

#endif /* mathutil_h */
