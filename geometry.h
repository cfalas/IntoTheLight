#include "raylib.h"
#include <optional>
#include <tuple>
#include <math.h>
#include <string>
#include <iostream>

using namespace std;

#pragma once
int sgn(float x) { return (x > 0) - (x < 0); }

// typedef struct Vector2 {
//     float x;                // Vector x component
//     float y;                // Vector y component
// } Vector2;

class Point: public Vector2 {
public:
	typedef Point P;
	explicit Point(float x_=0, float y_=0){
        x=x_;
        y=y_;
    }
	bool operator<(P p) const { return tie(x,y) < tie(p.x,p.y); }
	bool operator==(P p) const { return tie(x,y)==tie(p.x,p.y); }
	P operator+(P p) const { return P(x+p.x, y+p.y); }
	P operator-(P p) const { return P(x-p.x, y-p.y); }
	P operator*(float d) const { return P(x*d, y*d); }
	P operator/(float d) const { return P(x/d, y/d); }
	float dot(P p) const { return x*p.x + y*p.y; }
	float cross(P p) const { return x*p.y - y*p.x; }
	float cross(P a, P b) const { return (a-*this).cross(b-*this); }
	float dist2() const { return x*x + y*y; }
	double dist() const { return sqrt((double)dist2()); }
	// angle to x-axis in interval [-pi, pi]
	double angle() const { return atan2(y, x); }
	P unit() const { return *this/dist(); } // makes dist()=1
	P perp() const { return P(-y, x); } // rotates +90 degrees
	P normal() const { return perp().unit(); }
	// returns point rotated 'a' radians ccw around the origin
	P rotate(double a) const {
		return P(x*cos(a)-y*sin(a),x*sin(a)+y*cos(a)); }
	friend ostream& operator<<(ostream& os, P p) {
		return os << "(" << p.x << "," << p.y << ")"; }
};

class Segment {
public:
    Point p1;
    Point p2;
    Segment(Point p1,Point p2) : p1(p1), p2(p2) {};

    float length(){
        return (p2-p1).dist();
    }
};


bool sameSide(Segment seg,Point p1,Point p2){
    return (seg.p1.cross(seg.p2,p1)>0) ==  (seg.p1.cross(seg.p2,p2)>0);
}

Point reflect(Segment seg, Point p){
	return (p-seg.p1).rotate((seg.p2-seg.p1).angle()*2-2*(p-seg.p1).angle()) + seg.p1;
}

optional<Point> lineInter(Segment a, Segment b) {
	auto d = (a.p2 - a.p1).cross(b.p2 - b.p1);
	if (d == 0) return std::nullopt;
	auto p = b.p1.cross(a.p2, b.p2), q = b.p1.cross(b.p2, a.p1);
	return (a.p1 * p + a.p2 * q) / d;
}

optional<Point> segInter(Segment s1, Segment s2) {
    Point a = s1.p1;
    Point b = s1.p2;
    Point c = s2.p1;
    Point d = s2.p2;
	auto oa = c.cross(d, a), ob = c.cross(d, b),
	     oc = a.cross(b, c), od = a.cross(b, d);
	// Checks if intersection is single non-endpoint point.
	if (sgn(oa) * sgn(ob) < 0 && sgn(oc) * sgn(od) < 0)
		return (a * ob - b * oa) / (ob - oa);
    return std::nullopt;
}