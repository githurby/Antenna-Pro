#pragma once
#include <windows.h>
#include <gl/gl.h>
#include "AntennaModel.h"
class GLRenderer {
 HDC dc_{}; HGLRC rc_{}; GLuint fontBase_{}; int w_{1},h_{1}; float yaw_{-30},pitch_{20},zoom_{1}; int mode_{0};
 ant::DesignResult result_; bool has_{false};
 void text(float x,float y,const char* s); void graph(bool pattern); void geometry();
public:
 bool init(HWND hwnd); void shutdown(); void resize(int w,int h); void draw();
 void setResult(const ant::DesignResult&r){result_=r;has_=true;} void setMode(int m){mode_=m;}
 void rotate(float dx,float dy){yaw_+=dx;pitch_+=dy;} void zoom(float d){zoom_*=d;if(zoom_<.25f)zoom_=.25f;if(zoom_>8)zoom_=8;}
};
