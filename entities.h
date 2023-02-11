#include "raylib.h"
#include<math.h>
#include<stdio.h>
#include<algorithm>
//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

class SolidObject{
    public:
        Rectangle rec;
        Color color;

	Vector2 midpoint(){
		return Vector2({rec.x + rec.width / 2, rec.y + rec.height / 2});
	}

    bool collides(SolidObject other){
        SolidObject *leftmost = this;
        SolidObject *rightmost = &other;
        if(other.rec.x < rec.x) leftmost = &other, rightmost = this;
        SolidObject *upmost = this;
        SolidObject *downmost = &other;
        if(other.rec.y < rec.y) upmost = &other, downmost = this;

        bool inters_x = leftmost->rec.x + leftmost->rec.width > rightmost->rec.x;
        bool inters_y = upmost->rec.y + upmost->rec.height > downmost->rec.y;
        return inters_x && inters_y;
    }
};

class Player : public SolidObject{
    public:
    Vector2 speed;
    float maxspeed;

    void move(){
        rec.x += speed.x;
        rec.y += speed.y;
    }

    void setSpeed(){
        int numKeys = 0;
        speed.x = 0;
        speed.y = 0;
		if (IsKeyDown(KEY_RIGHT)) speed.x += maxspeed, numKeys++;
		if (IsKeyDown(KEY_LEFT)) speed.x -= maxspeed, numKeys++;
		if (IsKeyDown(KEY_UP)) speed.y -= maxspeed, numKeys++;
		if (IsKeyDown(KEY_DOWN)) speed.y += maxspeed, numKeys++;
        if (numKeys == 2){
            speed.x /= sqrt(2);
            speed.y /= sqrt(2);
        }
    }

    void fixCollision(SolidObject other){
        SolidObject *leftmost = this;
        SolidObject *rightmost = &other;
        if(other.rec.x < rec.x) leftmost = &other, rightmost = this;
        SolidObject *upmost = this;
        SolidObject *downmost = &other;
        if(other.rec.y < rec.y) upmost = &other, downmost = this;

        float diff_x = leftmost->rec.x + leftmost->rec.width - rightmost->rec.x;
        float diff_y = upmost->rec.y + upmost->rec.height - downmost->rec.y;

        if(diff_x < diff_y){
            if(other.rec.x > rec.x) rec.x = other.rec.x - rec.width;
            else rec.x = other.rec.x + other.rec.width;
        }
        else{
            if(other.rec.y > rec.y) rec.y = other.rec.y - rec.height;
            else rec.y = other.rec.y + other.rec.height;
        }


    }
};

class Wall : public SolidObject{
    public:
    bool active;
};

class Mirror : public SolidObject{
    public:
    bool active;
	float angle;
};