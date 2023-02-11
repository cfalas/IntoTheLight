#include "raylib.h"
#include "geometry.h"
#include<math.h>
#include<stdio.h>
#include<algorithm>
#include<vector>
#include<string>
#include<cstring>
#include<sstream>
using namespace std;
//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

class SolidObject{
    public:
        Rectangle rec;
        Color color;
        bool active = true;

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

    void draw(){
        if(active)
        DrawRectangleRec(rec, color);
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
    void draw(){
	    if(active) DrawRectanglePro(rec, {rec.width/2, rec.height/2}, angle, color);
    }
};

void DrawPolygon(Point a, Point b,Point c,Point d, Color color){
    DrawTriangle(a,b,c,color);
    DrawTriangle(c,d,a,color);
}
class LightFrustrum {
public:
    Point foc;
    Segment seg1;
    Segment seg2;
    LightFrustrum(Point foc,Segment seg1,Segment seg2) : foc(foc),seg1(seg1),seg2(seg2){};
    
    void draw(){
        //cout<<foc<<" "<<seg1.p1<<" "<<seg1.p2<<" "<<seg2.p1<<" "<<seg2.p2<<endl;
        DrawCircle(foc.x,foc.y,5,BLUE);
        DrawPolygon(seg1.p1,seg1.p2, seg2.p2,seg2.p1,{255,255,255,128});
    }
};

class Environment{
	// string serialize() {
    //     stringstream s;
    //     s << player.serialize();
    //     s << opponent.serialize();
    //     s << " ";
    //     s << walls.size();
    //     s << " ";
    //     return s.str();
	// }
    public:
    Player player;
    Player opponent;
    vector<Wall> walls;
    vector<Mirror> mirrors;
    vector<LightFrustrum> lightFrustra;
    
    Environment(){
        player.rec.x =  20;
        player.rec.y = 50;
        player.rec.width = 20;
        player.rec.height = 20;
        player.maxspeed = 5;
        player.color = BLACK;

        opponent.rec.x =  20;
        opponent.rec.y = 50;
        opponent.active = false;
        opponent.rec.width = 20;
        opponent.rec.height = 20;
        opponent.maxspeed = 5;
        opponent.color = BLUE;
    }

    void draw(){
        player.draw();
        opponent.draw();
        for(Wall wall : walls) wall.draw();
        for(Mirror mirror : mirrors) mirror.draw();
        for(LightFrustrum light : lightFrustra) light.draw();
    }

    void merge(Environment opp_env){
        walls = opp_env.walls;
        if(opp_env.mirrors.size() > mirrors.size()){
            mirrors.clear();
            for(Mirror m : opp_env.mirrors) mirrors.push_back(m);
        }

        opponent.rec = opp_env.player.rec;
        opponent.active = true;
    }
};