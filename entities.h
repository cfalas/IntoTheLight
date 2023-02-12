#include "raylib.h"
#include "geometry.h"
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
    LightFrustrum() : seg1(Point(0, 0), Point(0, 0)),seg2(Point(0, 0), Point(0, 0)){};
    
    void draw(unsigned char i){
        //cout<<foc<<" "<<seg1.p1<<" "<<seg1.p2<<" "<<seg2.p1<<" "<<seg2.p2<<endl;
        DrawCircle(foc.x,foc.y,5,BLUE);
        DrawPolygon(seg1.p1,seg1.p2, seg2.p2,seg2.p1,{i,i,i,i});
        //DrawPolygon(seg1.p1,seg1.p2, seg2.p2,seg2.p1,{i,i,i,i});
    }
    friend std::ostream& operator<<(std::ostream& os, const LightFrustrum m){
        os << m.seg1.p1.x<<" "<<m.seg1.p1.y << " " << m.seg1.p2.x<<" "<<m.seg1.p2.y << " ";
        os << m.seg2.p1.x<<" "<<m.seg2.p1.y << " " << m.seg2.p2.x<<" "<<m.seg2.p2.y << " ";
        os << m.foc.x << " "<< m.foc.y;
        return os;
    }
    friend std::istream& operator>>(std::istream& is, LightFrustrum &m){
        is >> m.seg1.p1.x >> m.seg1.p1.y >> m.seg1.p2.x >> m.seg1.p2.y;
        is >> m.seg2.p1.x >> m.seg2.p1.y >> m.seg2.p2.x >> m.seg2.p2.y;
        is >> m.foc.x >> m.foc.y;
        return is;
    }
};

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
	    if(active) DrawLine(seg.p1.x, seg.p1.y, seg.p2.x, seg.p2.y, RED);
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
    vector<Mirror> mirrors;
    vector<LightFrustrum> lightFrustra;
    
    Environment(){
        player.rec.x =  20;
        player.rec.y = 50;
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

    void draw(){
        player.draw();
        for(Wall wall : walls) wall.draw();
        for(Mirror mirror : mirrors) mirror.draw();
        
        for(int i = 0; i < lightFrustra.size(); i++){
            SetShaderValue(lightShader, lightShaderFocusLocs[i], (float[2]){ lightFrustra[i].foc.x,lightFrustra[i].foc.y }, SHADER_UNIFORM_VEC2);
        }
        BeginTextureMode(light_mask);
        ClearBackground({0,0,0,255});
        BeginShaderMode(lightShader);
        BeginBlendMode(BLEND_ADDITIVE);
        // rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
        // rlSetBlendMode(BLEND_CUSTOM);
        for(int i = 0; i < lightFrustra.size(); i++){
            lightFrustra[i].draw(i);
        }
        EndBlendMode();
        EndShaderMode();
        opponent.draw();
        EndTextureMode();
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawTextureRec(light_mask.texture, (Rectangle){ 0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight() }, {0,0}, WHITE);
        EndBlendMode();
        
    }

    void merge(Environment &opp_env){
        walls = opp_env.walls;
        if(opp_env.mirrors.size() > mirrors.size()){
            mirrors.clear();
            for(Mirror m : opp_env.mirrors) mirrors.push_back(m);
        }
        if(opp_env.lightFrustra.size() > lightFrustra.size()){
            lightFrustra.clear();
            for(LightFrustrum m : opp_env.lightFrustra) lightFrustra.push_back(m);
        }

        opponent.rec = opp_env.player.rec;
        opponent.health = opp_env.player.health;
        opponent.active = true;
    }

    friend std::istream& operator>>(std::istream& ss, Environment &e){
        ss >> e.player;
        ss >> e.opponent;
        int k;
        ss >> k;
        for(int i=0;i<k;i++){
            Wall w;
            ss >> w;
            e.walls.insert(w);
        }
        ss >> k;
        e.mirrors.resize(k);
        for(int i=0;i<k;i++) ss >> e.mirrors[i];

        ss >> k;
        e.lightFrustra.resize(k);
        for(int i=0;i<k;i++) ss >> e.lightFrustra[i];
        return ss;
    }
    friend std::ostream& operator<<(std::ostream& os, const Environment e){
        os << e.player;
        os << " ";
        os << e.opponent;
        os << " ";
        os << e.walls.size();
        os << " ";
        for(Wall wall : e.walls) os << wall <<" ";

        os << e.mirrors.size();
        os << " ";
        for(Mirror mirror : e.mirrors) os << mirror <<" ";

        os << e.lightFrustra.size();
        os << " ";
        for(LightFrustrum lightFrustrum : e.lightFrustra) os << lightFrustrum <<" ";
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