#include "GLRenderer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
bool GLRenderer::init(HWND hwnd){dc_=GetDC(hwnd);PIXELFORMATDESCRIPTOR p={sizeof(p),1,PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,24,0,0,0,0,0,0,0,0,0,0,0,0,24,8,0,PFD_MAIN_PLANE,0,0,0,0};int pf=ChoosePixelFormat(dc_,&p);if(!pf||!SetPixelFormat(dc_,pf,&p))return false;rc_=wglCreateContext(dc_);if(!rc_||!wglMakeCurrent(dc_,rc_))return false;fontBase_=glGenLists(96);HFONT f=CreateFontA(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,FF_DONTCARE,"Segoe UI");auto old=SelectObject(dc_,f);wglUseFontBitmapsA(dc_,32,96,fontBase_);SelectObject(dc_,old);DeleteObject(f);glEnable(GL_DEPTH_TEST);return true;}
void GLRenderer::shutdown(){if(rc_){wglMakeCurrent(dc_,rc_);if(fontBase_)glDeleteLists(fontBase_,96);wglMakeCurrent(nullptr,nullptr);wglDeleteContext(rc_);rc_=nullptr;}}
void GLRenderer::resize(int w,int h){w_=std::max(1,w);h_=std::max(1,h);}
void GLRenderer::text(float x,float y,const char*s){glRasterPos2f(x,y);glListBase(fontBase_-32);glCallLists((GLsizei)strlen(s),GL_UNSIGNED_BYTE,s);}
void GLRenderer::geometry(){
 glViewport(0,0,w_,h_);glMatrixMode(GL_PROJECTION);glLoadIdentity();double ar=(double)w_/h_;glOrtho(-ar,ar,-1,1,-20,20);glMatrixMode(GL_MODELVIEW);glLoadIdentity();glScalef(zoom_,zoom_,zoom_);glRotatef(pitch_,1,0,0);glRotatef(yaw_,0,0,1);
 glLineWidth(1);glBegin(GL_LINES);glColor3f(.35f,.38f,.43f);for(int i=-10;i<=10;i++){glVertex3f(i/10.f,-1,0);glVertex3f(i/10.f,1,0);glVertex3f(-1,i/10.f,0);glVertex3f(1,i/10.f,0);}glEnd();
 if(!has_) {
  return;
 }
 double maxv=.001;for(auto&s:result_.geometry)maxv=std::max(maxv,std::max({fabs(s.a.x),fabs(s.a.y),fabs(s.a.z),fabs(s.b.x),fabs(s.b.y),fabs(s.b.z)}));float k=.78f/(float)maxv;
 glLineWidth(5);glColor3f(1.0f,.66f,.12f);glBegin(GL_LINES);for(auto&s:result_.geometry){glVertex3f((float)s.a.x*k,(float)s.a.y*k,(float)s.a.z*k);glVertex3f((float)s.b.x*k,(float)s.b.y*k,(float)s.b.z*k);}glEnd();
 glPointSize(7);glColor3f(.1f,.9f,.85f);glBegin(GL_POINTS);for(auto&s:result_.geometry){glVertex3f((float)s.a.x*k,(float)s.a.y*k,(float)s.a.z*k);glVertex3f((float)s.b.x*k,(float)s.b.y*k,(float)s.b.z*k);}glEnd();
 glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,w_,h_,0,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();glColor3f(.88f,.92f,.96f);text(18,25,result_.name.c_str());glColor3f(.55f,.65f,.72f);text(18,47,"Drag: rotate | Mouse wheel: zoom | Automatic scale");
}
void GLRenderer::graph(bool pattern){
 glViewport(0,0,w_,h_);glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,w_,h_,0,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();float L=72,R=w_-28,T=42,B=h_-55;glColor3f(.18f,.21f,.25f);glBegin(GL_QUADS);glVertex2f(L,T);glVertex2f(R,T);glVertex2f(R,B);glVertex2f(L,B);glEnd();glLineWidth(1);glColor3f(.32f,.36f,.41f);glBegin(GL_LINES);for(int i=0;i<=10;i++){float x=L+(R-L)*i/10;glVertex2f(x,T);glVertex2f(x,B);}for(int i=0;i<=6;i++){float y=T+(B-T)*i/6;glVertex2f(L,y);glVertex2f(R,y);}glEnd();
 if(!has_) {
  return;
 }
 glColor3f(.9f,.94f,.97f);text(18,24,pattern?"RADIATION PATTERN (normalized cuts)":"SWR AND IMPEDANCE SWEEP");
 if(!pattern){double f0=result_.sweep.front().fMHz,f1=result_.sweep.back().fMHz;glLineWidth(3);glColor3f(.1f,.88f,.72f);glBegin(GL_LINE_STRIP);for(auto&p:result_.sweep){float x=L+(float)((p.fMHz-f0)/(f1-f0))*(R-L);float s=(float)std::min(6.0,p.swr);float y=B-(s-1)/5*(B-T);glVertex2f(x,y);}glEnd();char z[80];sprintf(z,"%.3f MHz",f0);glColor3f(.65f,.72f,.78f);text(L,B+22,z);sprintf(z,"%.3f MHz",f1);text(R-78,B+22,z);text(12,T+5,"6:1");text(12,B,"1:1");glColor3f(.1f,.88f,.72f);text(L+8,T+22,"SWR");
 // R e X usam escala independente de -150 a +150 ohm (indicada a direita).
 glLineWidth(2);glColor3f(1.0f,.42f,.28f);glBegin(GL_LINE_STRIP);for(auto&p:result_.sweep){float x=L+(float)((p.fMHz-f0)/(f1-f0))*(R-L);float y=T+(float)(150-std::max(-150.0,std::min(150.0,p.r)))/300*(B-T);glVertex2f(x,y);}glEnd();
 glColor3f(.25f,.58f,1.0f);glBegin(GL_LINE_STRIP);for(auto&p:result_.sweep){float x=L+(float)((p.fMHz-f0)/(f1-f0))*(R-L);float y=T+(float)(150-std::max(-150.0,std::min(150.0,p.x)))/300*(B-T);glVertex2f(x,y);}glEnd();
 glColor3f(1.0f,.42f,.28f);text(L+60,T+22,"R");glColor3f(.25f,.58f,1.0f);text(L+82,T+22,"X");glColor3f(.65f,.72f,.78f);text(R-34,T+5,"+150");text(R-28,B,"-150");}
 else {auto polar=[&](const std::vector<ant::PatternPoint>& p,float cr,float cg,float cb){glColor3f(cr,cg,cb);glLineWidth(3);glBegin(GL_LINE_STRIP);for(auto&a:p){float db=(float)std::max(-30.0,a.gainDb),rad=(db+30)/30;float x=L+(float)(a.angleDeg/360.)*(R-L),y=B-rad*(B-T);glVertex2f(x,y);}glEnd();};polar(result_.azimuth,1,.65f,.15f);polar(result_.elevation,.1f,.8f,1);glColor3f(1,.65f,.15f);text(L+8,T+20,"Azimuth");glColor3f(.1f,.8f,1);text(L+85,T+20,"Elevation");glColor3f(.65f,.72f,.78f);text(18,T+5,"0 dB");text(12,B,"-30");text(L,B+22,"0 degrees");text(R-70,B+22,"360 degrees");}
}
void GLRenderer::draw(){if(!rc_)return;wglMakeCurrent(dc_,rc_);glViewport(0,0,w_,h_);glClearColor(.055f,.065f,.078f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);if(mode_==0)geometry();else {glDisable(GL_DEPTH_TEST);graph(mode_==2);glEnable(GL_DEPTH_TEST);}SwapBuffers(dc_);}
