//
// Created by kevin on 17/05/2024.
//

#ifndef PONGGAME_VEC2_H
#define PONGGAME_VEC2_H


class Vec2 {
public:
    Vec2(): x(0.0f), y(0.0f) {}

    Vec2(float x, float y) : x(x), y(y){}

    Vec2 operator+(Vec2 const& rhs){
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2& operator+=(Vec2 const& rhs){
        x += rhs.x;
        y += rhs.y;

        return *this;
    }

    Vec2 operator*(float rhs){
        return Vec2(x * rhs, y * rhs);
    }


    float x, y;
};


#endif //PONGGAME_VEC2_H
