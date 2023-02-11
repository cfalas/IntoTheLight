#include "geometry.h"
#include "raylib.h"
#include "entities.h"
#include<vector>
#include<queue>
#include<algorithm>
#include<iostream>

using namespace std;
#pragma once
float eps=1e-3;

enum ObstacleType {wall, single_mirror, double_mirror};

class ObstacleForSim {
public:
    Segment seg;
    ObstacleType type;
    ObstacleForSim(Segment seg,ObstacleType type) : seg(seg),type(type){};

    void draw(){
        DrawLine(seg.p1.x,seg.p1.y,seg.p2.x,seg.p2.y, RED);
    }
};

class LightFrustrumForSim {
    Point foc;
    Segment seg;

public:
    LightFrustrumForSim(Point foc_,Segment seg_) : foc(foc_),seg(seg_){
        seg.p1 = (seg.p1-foc)*(1.0001)+foc;
        seg.p2 = (seg.p2-foc)*(1.0001)+foc;
    };

    void draw(){
        //cout<<"simfrust"<<endl;
        //cout<<foc<<" "<<seg.p1<<" "<<seg.p2<< (seg.p2-foc)*100+foc<<(seg.p1-foc)*100+foc<<endl;
        DrawCircle(foc.x,foc.y,5,GREEN);
        DrawPolygon(seg.p1,seg.p2, (seg.p2-foc)*100+foc,(seg.p1-foc)*100+foc,{255,255,255,128});
    }

    bool pointInside(Point p){
        return foc.cross(seg.p1,p)>=0 && foc.cross(p,seg.p2)>=0 && !sameSide(seg,foc,p);
    }

    bool segmentIsCloser(Segment seg1, Segment seg2){
        if(sameSide(seg1,seg2.p1,foc) && sameSide(seg1,seg2.p2,foc)) return false;
        if(!sameSide(seg2,seg1.p1,foc) && !sameSide(seg2,seg1.p2,foc)) return false;
        return true;
    }

    float pointToFrac(Point p){
        if(auto intersection = lineInter(Segment(foc,p),seg)){
            return (*intersection - seg.p1).dist() / seg.length();
        } else return 0;
    }
    
    Point fracToPoint(float frac){
        return seg.p1*(1-frac) + seg.p2*frac;
    }

    pair<vector<LightFrustrum>, vector<LightFrustrumForSim>> sim(const vector<ObstacleForSim> &obstacles){
        vector<ObstacleForSim> clippedObstacles;
        //cout<<"sim"<<" "<<foc<<" "<<seg.p1<<" "<<seg.p2<<endl;    

        for(auto &obstacle : obstacles){
            if(pointInside(obstacle.seg.p1) && pointInside(obstacle.seg.p2)) {
                clippedObstacles.push_back(obstacle);
            } else {
                auto p1 = obstacle.seg.p1;
                auto p2 = obstacle.seg.p2;
                bool movedp1 = false;
                bool movedp2 = false;
                //cout<<"obs1"<<" "<<obstacle.seg.p1<<" "<<obstacle.seg.p2<<endl;
                if(auto newP = segInter(obstacle.seg,Segment(seg.p1,(seg.p1-foc)*1000+foc))){
                    if(!pointInside(p1) && !movedp1){
                        p1 = *newP;
                        movedp1 = true;
                    } else if(!pointInside(p2) && !movedp2){
                        p2 = *newP;
                        movedp2 = true;
                    }
                }
                //cout<<"obs2"<<" "<<p1<<" "<<p2<<endl;
                if(auto newP = segInter(obstacle.seg,Segment(seg.p2,(seg.p2-foc)*1000+foc))){
                    if(!pointInside(p1) && !movedp1){
                        p1 = *newP;
                        movedp1 = true;
                    } else if(!pointInside(p2) && !movedp2){
                        p2 = *newP;
                        movedp2 = true;
                    }
                }
                //cout<<"obs3"<<" "<<p1<<" "<<p2<<endl;
                if(movedp1 || movedp2){
                    clippedObstacles.push_back(ObstacleForSim(Segment(p1,p2), obstacle.type));
                }
            }
        }

        for(auto &obstacle : clippedObstacles){
            if(foc.cross(obstacle.seg.p1,obstacle.seg.p2)<0) swap(obstacle.seg.p1,obstacle.seg.p2);
            
        }

        sort(clippedObstacles.begin(),clippedObstacles.end(),[=](ObstacleForSim a,ObstacleForSim b) -> bool {
            return segmentIsCloser(a.seg,b.seg);
        });

        vector<LightFrustrum> lightOutput;
        vector<LightFrustrumForSim> newlightSims;


        vector<pair<float,float>> activeIntervals;
        activeIntervals.push_back(pair<float,float>(0.0,1.0));

        for(auto &obstacle : clippedObstacles){
            pair<float,float> obstInterval = {pointToFrac(obstacle.seg.p1),pointToFrac(obstacle.seg.p2)};
            
            vector<pair<float,float>> newActiveIntervals;

            for(auto activeInt: activeIntervals){
                //cout<<activeInt.first<<" "<<activeInt.second<<" "<<obstInterval.first<<" "<<obstInterval.second<<endl;
                if(activeInt.second<=obstInterval.first+eps || obstInterval.second-eps<=activeInt.first){
                    newActiveIntervals.push_back(activeInt);
                } else{
                    pair<float,float> intervalIntersection = {max(activeInt.first,obstInterval.first),min(activeInt.second,obstInterval.second)};
                    
                    Segment outputStartSegment = Segment(fracToPoint(intervalIntersection.first),fracToPoint(intervalIntersection.second));
                    Segment outputEndSegment = Segment(*lineInter(obstacle.seg,Segment(foc,outputStartSegment.p1)),*lineInter(obstacle.seg,Segment(foc,outputStartSegment.p2)));
                    lightOutput.push_back(LightFrustrum(foc,outputStartSegment,outputEndSegment));

                    if((obstacle.type == single_mirror && obstacle.seg.p1.cross(obstacle.seg.p2,foc)>0) || obstacle.type == double_mirror){
                        newlightSims.push_back(LightFrustrumForSim(reflect(outputEndSegment,foc),Segment(outputEndSegment.p2,outputEndSegment.p1)));
                    }

                    if(activeInt.first+eps<obstInterval.first){
                        newActiveIntervals.push_back({activeInt.first,obstInterval.first});
                    }
                    if(obstInterval.second+eps<activeInt.second){
                        newActiveIntervals.push_back({obstInterval.second, activeInt.second});
                    }
                }
            }
            activeIntervals = newActiveIntervals;
        }
        return {lightOutput,newlightSims};
    }
};

vector<LightFrustrum> run_light_simulation(vector<ObstacleForSim> obstacles, LightFrustrumForSim startLight){
    vector<LightFrustrum> lightOutput;
    vector<LightFrustrumForSim> lightSims;
    lightSims.push_back(startLight);

    for(int i = 0; i<50;i++){
        vector<LightFrustrumForSim> newlightSims;
        for(auto lightSim : lightSims){
            auto [lightFrustra, lightFrustraForSim] = lightSim.sim(obstacles);
            for(auto x: lightFrustra) lightOutput.push_back(x);
            for(auto x: lightFrustraForSim) newlightSims.push_back(x);
        }
        //cout<<lightOutput.size()<<" "<<newlightSims.size()<<endl;
        lightSims = newlightSims;
    }

    return lightOutput;
}

void test_simulation(){

    static const int screenWidth = 800;
    static const int screenHeight = 450;
    LightFrustrumForSim lightForSim(Point(200,100),Segment(Point(300,200),Point(150,200)));
    vector<ObstacleForSim> obstacles;
    obstacles.push_back(ObstacleForSim(Segment(Point(150,310),Point(120,300)),double_mirror));
    obstacles.push_back(ObstacleForSim(Segment(Point(0,0),Point(0,screenHeight)),double_mirror));
    obstacles.push_back(ObstacleForSim(Segment(Point(screenWidth,screenHeight),Point(0,screenHeight)),double_mirror));
    obstacles.push_back(ObstacleForSim(Segment(Point(screenWidth,screenHeight),Point(screenWidth,0)),double_mirror));
    obstacles.push_back(ObstacleForSim(Segment(Point(0,0),Point(screenWidth,0)),double_mirror));

    vector<LightFrustrum> lightOutput = run_light_simulation(obstacles, lightForSim);

    for(auto x: obstacles) x.draw();
    //lightForSim.draw();
    // for (auto light : lightSims)
    //     light.draw();
    for (auto light : lightOutput)
        light.draw();
}

