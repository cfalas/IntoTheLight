#include "raylib.h"
#include "geometry.h"
#include "lightsim.h"
#include<math.h>
#include<stdio.h>
#include<algorithm>
#include<vector>
#include<set>
#include<string>
#include<cstring>
#include<sstream>
#include <iterator>
using namespace std;
//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

#pragma once

namespace std {
    template<typename T>
    std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
        using namespace std;
        copy(v.begin(), v.end(), ostream_iterator<T>(os, "\n"));
        return os;
    }
}

std::ostream& operator<<(std::ostream& os, const Rectangle r){
    os << r.x <<" "<<r.y<<" "<<r.width<<" "<<r.height;
    return os;
}
std::istream& operator>>(std::istream& is, Rectangle &r){
    is >> r.x >> r.y >> r.width >> r.height;
    return is;
}
std::ostream& operator<<(std::ostream& os, const Color c){
    os << (int)c.r << " " << (int)c.g << " " << (int)c.b << " " <<(int)c.a <<" ";
    return os;
}
std::istream& operator>>(std::istream& is, Color &c){
    int r, g, b, a;
    is >> r >> g >> b >> a;
    c = {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
    return is;
}



class SolidObject{
    public:
        Rectangle rec;
        Color color;
        bool active = true;

	Point midpoint(){
		return Point({rec.x + rec.width / 2, rec.y + rec.height / 2});
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
    bool inside(LightFrustrum other){
        vector<Point> inside_pts = {Point(rec.x, rec.y), Point(rec.x, rec.y+rec.height), Point(rec.x+rec.width, rec.y), Point(rec.x+rec.width, rec.y+rec.height)};
        vector<Segment> outside_segs = {other.seg1, Segment(other.seg1.p2, other.seg2.p2), Segment(other.seg2.p2, other.seg2.p1), Segment(other.seg2.p1, other.seg1.p1)};
        for(Point pt : inside_pts){
            bool found = false;
            for(Segment seg : outside_segs){
                //printf("Point (%f %f) in terms of Seg (%f %f) (%f %f): %d\n", pt.x, pt.y, seg.p1.x, seg.p1.y, seg.p2.x, seg.p2.y, seg.sideOf(pt));
                if(seg.sideOf(pt)!=-1) found = true;
            }
            //printf("---------------\n");
            if(!found) return true;

        }
        return false;
    }

    void draw(){
        if(active)
        DrawRectangleRec(rec, color);
    }
    friend std::ostream& operator<<(std::ostream& os, const SolidObject p) {
        os << p.rec<<" "<<p.color<<" ";
        return os;
    }
    friend std::istream& operator>>(std::istream& is, SolidObject &p) {
        is >> p.rec >> p.color;
        return is;
    }
};

class Player : public SolidObject{
    int maxhealth = 300;
    public:
    int health = maxhealth;
    Vector2 speed;
    float maxspeed;
    
    float getHealth(){
        return (float)health / maxhealth;
    }
    void move(){
        rec.x += speed.x;
        rec.y += speed.y;
    }

    bool alive(){
        return health > 0;
    }

    void damage(){
        health--;
        printf("Health %d\n", health);
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
    friend std::ostream& operator<<(std::ostream& os, const Player p) {
        os << p.rec<<" "<<p.color<<" "<<p.health;
        return os;
    }
    friend std::istream& operator>>(std::istream& is, Player &p) {
        is >> p.rec >> p.color >> p.health;
        return is;
    }
};

class Wall : public SolidObject{
    public:
    bool mirror;
    bool operator<(const Wall& rhs) const {
        return std::tie(rec.x, rec.y, rec.width, rec.height) < std::tie(rhs.rec.x, rhs.rec.y, rhs.rec.width, rhs.rec.height);
    }
};

class Mirror {
    public:
    bool active;
    Segment seg = Segment(Point(), Point());
    void draw(){
	    DrawLine(seg.p1.x, seg.p1.y, seg.p2.x, seg.p2.y, RED);
    }
    friend std::ostream& operator<<(std::ostream& os, const Mirror m){
        os << m.seg.p1.x<<" "<<m.seg.p1.y << " " << m.seg.p2.x<<" "<<m.seg.p2.y;
        return os;
    }
    friend std::istream& operator>>(std::istream& is, Mirror &m){
        is >> m.seg.p1.x >> m.seg.p1.y >> m.seg.p2.x >> m.seg.p2.y;
        return is;
    }
};



class Environment;
std::ostream& operator<<(std::ostream& os, const Environment e);
class Environment{

    public:
    Shader lightShader;
    vector<int> lightShaderFocusLocs;
    RenderTexture2D light_mask;
    Player player;
    Player opponent;
    set<Wall> walls;
    vector<Mirror> myMirrors;
    vector<Mirror> oppMirrors;
    vector<LightFrustrum> myLightFrustra;
    vector<LightFrustrum> oppLightFrustra;
    LightFrustrumForSim myStartingLight;
    LightFrustrumForSim oppStartingLight;
    set<Wall> backgroundWalls;
    
    Environment()
        : myStartingLight(Point(200,100),Segment(Point(300,200),Point(150,200))),
          oppStartingLight(Point(800,500),Segment(Point(750,450),Point(850,450))) {
        player.rec.x =  200;
        player.rec.y = 200;
        player.rec.width = 20;
        player.rec.height = 20;
        player.maxspeed = 5;
        player.color = GRAY;

        opponent.rec.x =  20;
        opponent.rec.y = 50;
        opponent.active = false;
        opponent.rec.width = 20;
        opponent.rec.height = 20;
        opponent.maxspeed = 5;
        opponent.color = {50,50,50,255};
    }

    void drawLightFrustra(vector<LightFrustrum> &lightFrustra, int color){
        for(int i = 0; i < lightFrustra.size(); i++){
            SetShaderValue(lightShader, lightShaderFocusLocs[i], (float[2]){ lightFrustra[i].foc.x,lightFrustra[i].foc.y }, SHADER_UNIFORM_VEC2);
        }
        BeginShaderMode(lightShader);
        BeginBlendMode(BLEND_ALPHA);
        // rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
        // rlSetBlendMode(BLEND_CUSTOM);
        for(int i = 0; i < lightFrustra.size(); i++){
            lightFrustra[i].draw(i,color);
        }
        EndBlendMode();
        EndShaderMode();
    }

    void draw(){
        player.draw();
        opponent.draw();
        for(Wall wall : walls) wall.draw();
        
        BeginTextureMode(light_mask);
        ClearBackground({0,0,0,255});
        drawLightFrustra(myLightFrustra,0);
        drawLightFrustra(oppLightFrustra,1);
        opponent.draw();
        DrawCircleGradient(player.midpoint().x,player.midpoint().y, 100, {255,255,255,50}, {255,255,255,0});
        EndTextureMode();
        BeginBlendMode(BLEND_MULTIPLIED);
        if(opponent.alive() && player.alive())
            DrawTextureRec(light_mask.texture, (Rectangle){ 0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight() }, {0,0}, WHITE);
        EndBlendMode();
        for(Mirror mirror : myMirrors) mirror.draw();
        for(Mirror mirror : oppMirrors) mirror.draw();

    }

    void merge(Environment &opp_env){
        //walls = opp_env.walls;
        if(myStartingLight.foc.x == opp_env.myStartingLight.foc.x){
            myMirrors = oppMirrors;
            myLightFrustra = oppLightFrustra;
            myStartingLight = oppStartingLight;
        }
        oppMirrors = opp_env.myMirrors;
        oppLightFrustra = opp_env.myLightFrustra;
        oppStartingLight = opp_env.myStartingLight;

        opponent.rec = opp_env.player.rec;
        opponent.health = opp_env.player.health;
        opponent.active = true;
    }

    friend std::istream& operator>>(std::istream& ss, Environment &e){
        ss >> e.player;
        ss >> e.opponent;
        int k;
        /*
        ss >> k;
        for(int i=0;i<k;i++){
            Wall w;
            ss >> w;
            e.walls.insert(w);
        }
        }
        ss >> k;
        for(int i=0;i<k;i++){
            Wall w;
            ss >> w;
            e.backgroundWalls.insert(w);
        }
    */
        ss >> k;
        e.myMirrors.resize(k);
        for(int i=0;i<k;i++) ss >> e.myMirrors[i];

        ss >> k;
        e.myLightFrustra.resize(k);
        for(int i=0;i<k;i++) ss >> e.myLightFrustra[i];

        ss >> k;
        e.oppMirrors.resize(k);
        for(int i=0;i<k;i++) ss >> e.oppMirrors[i];

        ss >> k;
        e.oppLightFrustra.resize(k);
        for(int i=0;i<k;i++) ss >> e.oppLightFrustra[i];
        return ss;
    }
    friend std::ostream& operator<<(std::ostream& os, const Environment e){
        os << e.player;
        os << " ";
        os << e.opponent;
        os << " ";
        /*
        os << e.walls.size();
        os << " ";
        for(Wall wall : e.walls) os << wall <<" ";


        os << e.backgroundWalls.size();
        os << " ";
        for(Wall wall : e.backgroundWalls) os << wall <<" ";
*/
        os << e.myMirrors.size();
        os << " ";
        for(Mirror mirror : e.myMirrors) os << mirror <<" ";

        os << e.myLightFrustra.size();
        os << " ";
        for(LightFrustrum lightFrustrum : e.myLightFrustra) os << lightFrustrum <<" ";

        os << e.oppMirrors.size();
        os << " ";
        for(Mirror mirror : e.oppMirrors) os << mirror <<" ";

        os << e.oppLightFrustra.size();
        os << " ";
        for(LightFrustrum lightFrustrum : e.oppLightFrustra) os << lightFrustrum <<" ";

        return os;
    }
	string serialize() {
        stringstream s;
        s << *this;
        return s.str();
	}
    static Environment deserialize(istringstream &ss){
        Environment e = Environment();
        ss >> e;
        return e;
    }
};