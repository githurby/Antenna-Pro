#include "MatchDiagram.h"
#include "resource.h"
#include <commctrl.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace {
struct DiagramData { ant::DesignInput in; ant::DesignResult result; };
constexpr COLORREF BG=RGB(245,248,251), INK=RGB(25,38,52), BLUE=RGB(20,104,180), ORANGE=RGB(224,112,24), GREY=RGB(110,124,138), RED=RGB(180,36,42), GREEN=RGB(25,135,88);

std::wstring wide(const std::string&s){int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0);std::wstring w(n?n:0,L'\0');if(n){MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,w.data(),n);w.pop_back();}return w;}
void line(HDC dc,int x1,int y1,int x2,int y2,COLORREF c,int width=2,int style=PS_SOLID){HPEN p=CreatePen(style,width,c);auto old=SelectObject(dc,p);MoveToEx(dc,x1,y1,nullptr);LineTo(dc,x2,y2);SelectObject(dc,old);DeleteObject(p);}
void txt(HDC dc,int x,int y,int w,int h,const std::wstring&s,COLORREF c=INK,UINT flags=DT_LEFT|DT_TOP|DT_WORDBREAK){RECT r{x,y,x+w,y+h};SetTextColor(dc,c);SetBkMode(dc,TRANSPARENT);DrawTextW(dc,s.c_str(),-1,&r,flags);}
void arrow(HDC dc,int x1,int y1,int x2,int y2,COLORREF c=BLUE){line(dc,x1,y1,x2,y2,c,1);if(y1==y2){line(dc,x1,y1,x1+7,y1-4,c,1);line(dc,x1,y1,x1+7,y1+4,c,1);line(dc,x2,y2,x2-7,y2-4,c,1);line(dc,x2,y2,x2-7,y2+4,c,1);}else{line(dc,x1,y1,x1-4,y1+7,c,1);line(dc,x1,y1,x1+4,y1+7,c,1);line(dc,x2,y2,x2-4,y2-7,c,1);line(dc,x2,y2,x2+4,y2-7,c,1);}}
void capacitor(HDC dc,int x,int y1,int y2){int m=(y1+y2)/2;line(dc,x,y1,x,m-7,ORANGE,3);line(dc,x-12,m-7,x+12,m-7,ORANGE,3);line(dc,x-12,m+7,x+12,m+7,ORANGE,3);line(dc,x,m+7,x,y2,ORANGE,3);}
void coil(HDC dc,int x1,int y,int x2){int n=5,span=(x2-x1)/n;line(dc,x1-10,y,x1,y,ORANGE,2);for(int i=0;i<n;i++){Arc(dc,x1+i*span,y-10,x1+(i+1)*span,y+10,x1+i*span,y,x1+(i+1)*span,y);}line(dc,x2,y,x2+10,y,ORANGE,2);}
void seriesComponent(HDC dc,int x1,int y,int x2,double x){
 if(std::abs(x)<.05){line(dc,x1,y,x2,y,BLUE,4);return;}
 if(x>0){coil(dc,x1+10,y,x2-10);return;}
 int m=(x1+x2)/2;line(dc,x1,y,m-7,y,BLUE,4);line(dc,m-7,y-13,m-7,y+13,ORANGE,3);line(dc,m+7,y-13,m+7,y+13,ORANGE,3);line(dc,m+7,y,x2,y,BLUE,4);
}
void toroid(HDC dc,int cx,int cy){HPEN p=CreatePen(PS_SOLID,12,RGB(70,86,100));auto old=SelectObject(dc,p);auto oldBrush=SelectObject(dc,GetStockObject(NULL_BRUSH));Ellipse(dc,cx-58,cy-58,cx+58,cy+58);SelectObject(dc,oldBrush);SelectObject(dc,old);DeleteObject(p);HPEN w=CreatePen(PS_SOLID,3,ORANGE);old=SelectObject(dc,w);for(int i=-35;i<=35;i+=14){MoveToEx(dc,cx-45,cy+i,nullptr);LineTo(dc,cx+45,cy+i+18);}SelectObject(dc,old);DeleteObject(w);}
std::wstring mm(double m){std::wostringstream o;o<<std::fixed<<std::setprecision(1)<<m*1000<<L" mm";return o.str();}
std::wstring component(double x,double fMHz){std::wostringstream o;o<<std::fixed<<std::setprecision(2);if(std::abs(x)<.05)return L"direct connection";if(x>0)o<<L"L = "<<x/(2*3.14159265358979323846*fMHz*1e6)*1e6<<L" µH";else o<<L"C = "<<1/(2*3.14159265358979323846*fMHz*1e6*std::abs(x))*1e12<<L" pF";return o.str();}

void insetYagi(HDC dc,int x,int y){
 txt(dc,x,y-25,290,22,L"POSITION IN THE YAGI ARRAY",GREY,DT_CENTER|DT_SINGLELINE);
 line(dc,x+25,y+70,x+270,y+70,GREY,5);
 line(dc,x+55,y+10,x+55,y+130,GREY,4); txt(dc,x+18,y+136,80,20,L"Reflector",GREY,DT_CENTER);
 line(dc,x+135,y+4,x+135,y+136,ORANGE,6);txt(dc,x+95,y+141,80,38,L"DRIVEN\nELEMENT",ORANGE,DT_CENTER);
 line(dc,x+215,y+20,x+215,y+120,GREY,4);txt(dc,x+178,y+126,75,20,L"Director",GREY,DT_CENTER);
}
void drawGamma(HDC dc,const DiagramData&d,int left,int top,int width,int height){
 (void)height;
 const auto&r=d.result;int x1=left+45,x2=left+610,y=top+260,feed=left+285,rodEnd=left+505,rodY=top+150;
 txt(dc,left,top,width,34,L"GAMMA MATCH — CONNECTION TO THE DRIVEN ELEMENT",BLUE,DT_CENTER|DT_SINGLELINE);
 line(dc,x1,y,x2,y,ORANGE,8);txt(dc,x1,y+15,560,25,L"Continuous driven element (radiator)",ORANGE,DT_CENTER);
 line(dc,feed,top+410,feed,y,BLUE,4);txt(dc,feed-105,top+415,210,40,L"50 Ω coax\nshield → element",BLUE,DT_CENTER);
 capacitor(dc,feed,rodY,y);line(dc,feed,rodY,rodEnd,rodY,BLUE,7);line(dc,rodEnd,rodY,rodEnd,y,BLUE,7);
 txt(dc,feed+45,rodY-36,220,25,L"Gamma rod",BLUE,DT_CENTER);
 txt(dc,rodEnd-55,y+42,120,40,L"Sliding shorting\nstrap",BLUE,DT_CENTER);
 arrow(dc,feed,rodY-58,rodEnd,rodY-58);txt(dc,feed+25,rodY-84,190,24,L"L = "+mm(r.gammaRodM),BLUE,DT_CENTER);
 arrow(dc,rodEnd+36,rodY,rodEnd+36,y);txt(dc,rodEnd+43,rodY+34,115,40,L"S = "+mm(r.gammaSpacingM),BLUE);
 std::wstring comp=r.gammaSeriesCapPf>0?(L"Series C ≈ "+std::to_wstring(r.gammaSeriesCapPf).substr(0,6)+L" pF"):(L"Series L ≈ "+std::to_wstring(r.gammaSeriesInductorUh).substr(0,6)+L" µH");
 txt(dc,feed-105,top+65,210,45,comp,ORANGE,DT_CENTER|DT_WORDBREAK);
 txt(dc,left+20,top+485,620,55,L"IMPORTANT: connect the gamma only to the DRIVEN ELEMENT. Directors and the reflector are parasitic and must never be connected to the coax or gamma rod.",RED,DT_CENTER|DT_WORDBREAK);
 insetYagi(dc,left+570,top+145);
}
void drawTransformer(HDC dc,const DiagramData&d,int left,int top,int width,int height){
 (void)height;
 const auto&r=d.result;txt(dc,left,top,width,34,r.matchBalanced?L"BALUN TRANSFORMER — INSTALLATION AT THE FEED POINT":L"UNUN TRANSFORMER — INSTALLATION AT THE FEED POINT",BLUE,DT_CENTER|DT_SINGLELINE);
 int cy=top+250,cx=left+360;toroid(dc,cx,cy);line(dc,left+70,cy-20,cx-62,cy-20,BLUE,4);line(dc,left+70,cy+20,cx-62,cy+20,BLUE,4);txt(dc,left+55,cy-85,190,55,L"50 Ω coax\nunbalanced input",BLUE,DT_CENTER);
 line(dc,cx+62,cy-25,left+650,cy-85,ORANGE,4);line(dc,cx+62,cy+25,left+650,cy+85,ORANGE,4);
 txt(dc,cx-95,cy+75,190,55,L"N50 : Nantenna = "+std::to_wstring(r.turns50)+L" : "+std::to_wstring(r.turnsAntenna),INK,DT_CENTER|DT_WORDBREAK);
 if(r.matchBalanced){line(dc,left+650,cy-85,left+800,cy-85,ORANGE,6);line(dc,left+650,cy+85,left+800,cy+85,ORANGE,6);txt(dc,left+650,cy-20,160,45,L"Two terminals to\ndriven element/loop",ORANGE,DT_CENTER);}
 else {line(dc,left+650,cy-85,left+850,cy-85,ORANGE,6);line(dc,left+650,cy+85,left+760,cy+85,GREY,5);txt(dc,left+650,cy-20,230,55,L"High-Z output → radiator\nReturn → counterpoise/RF ground",ORANGE,DT_CENTER);}
 txt(dc,left+75,top+450,790,70,r.matchBalanced?L"Install the BALUN directly at the feed point. Connect its two balanced terminals to the two sides of the driven element or loop; never connect directors or reflectors.":L"Install the UNUN at the fed end. Keep the common-mode choke and counterpoise separate as specified by the design.",RED,DT_CENTER|DT_WORDBREAK);
}
void drawLNetwork(HDC dc,const DiagramData&d,int left,int top,int width,int height){
 (void)height;
 const auto&r=d.result;txt(dc,left,top,width,34,L"L NETWORK + COMMON-MODE CONTROL",BLUE,DT_CENTER|DT_SINGLELINE);int y=top+245,x0=left+55,x1=left+230,x2=left+450,x3=left+720;
 line(dc,x0,y,x1-55,y,BLUE,4);txt(dc,x0, y-70,150,45,L"Coaxial\n50 Ω",BLUE,DT_CENTER);
 seriesComponent(dc,x1-55,y,x1+55,r.matchSeriesX);txt(dc,x1-80,y+28,160,45,component(r.matchSeriesX,d.in.fMHz),ORANGE,DT_CENTER);
 line(dc,x1+55,y,x2-60,y,BLUE,4);seriesComponent(dc,x2-60,y,x2+60,r.matchCompensationX);txt(dc,x2-100,y-72,200,48,L"X compensation\n"+component(r.matchCompensationX,d.in.fMHz),ORANGE,DT_CENTER);
 line(dc,x2+60,y,x3,y,ORANGE,4);
 if(std::abs(r.matchShuntX)>.05){int sx=r.shuntOnAntenna?x2+70:x1-65;capacitor(dc,sx,y,top+390);line(dc,sx,top+390,sx+90,top+390,GREY,3);txt(dc,sx-75,top+400,180,45,component(r.matchShuntX,d.in.fMHz)+L" shunt",ORANGE,DT_CENTER);}
 txt(dc,x3-20,y-80,180,55,d.result.matchBalanced?L"1:1 balun →\ndriven element":L"1:1 choke →\nradiator",GREEN,DT_CENTER);
 line(dc,x3,y,x3+120,y-55,ORANGE,6);line(dc,x3,y,x3+120,y+55,ORANGE,6);
 txt(dc,left+70,top+500,790,65,L"Install the network at the impedance measurement reference plane. If X compensation is specified, place that component in series at the antenna. Short interconnections are part of the RF circuit.",RED,DT_CENTER|DT_WORDBREAK);
}
void drawDirect(HDC dc,const DiagramData&d,int left,int top,int width,int height){
 (void)height;
 txt(dc,left,top,width,34,L"DIRECT 50 Ω CONNECTION WITH 1:1 CHOKE/BALUN",BLUE,DT_CENTER|DT_SINGLELINE);int y=top+260;line(dc,left+80,y,left+310,y,BLUE,5);toroid(dc,left+400,y);line(dc,left+460,y,left+760,y,ORANGE,5);txt(dc,left+70,y-70,210,40,L"Coaxial 50 Ω",BLUE,DT_CENTER);txt(dc,left+300,y+80,210,45,d.result.matchBalanced?L"1:1 current balun":L"1:1 common-mode choke",INK,DT_CENTER);txt(dc,left+650,y-70,210,45,L"Radiator\nfeed point",ORANGE,DT_CENTER);txt(dc,left+85,top+470,780,60,L"A 1:1 device does not transform impedance. It prevents the outside of the coax shield from radiating and distorting the pattern.",RED,DT_CENTER|DT_WORDBREAK);
}

LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){
 if(m==WM_NCCREATE){auto cs=(CREATESTRUCTW*)l;SetWindowLongPtrW(h,GWLP_USERDATA,(LONG_PTR)cs->lpCreateParams);}
 auto*d=(DiagramData*)GetWindowLongPtrW(h,GWLP_USERDATA);
 switch(m){
 case WM_GETMINMAXINFO:((MINMAXINFO*)l)->ptMinTrackSize={900,650};return 0;
 case WM_ERASEBKGND:return 1;
 case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT rc;GetClientRect(h,&rc);HBRUSH bg=CreateSolidBrush(BG);FillRect(dc,&rc,bg);DeleteObject(bg);SetBkMode(dc,TRANSPARENT);
  HFONT title=CreateFontW(-25,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");auto old=SelectObject(dc,title);txt(dc,20,14,rc.right-40,34,L"Antenna Pro — matching-network construction diagram",INK,DT_CENTER|DT_SINGLELINE);SelectObject(dc,old);DeleteObject(title);
  HFONT font=CreateFontW(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");old=SelectObject(dc,font);
  int clientWidth=static_cast<int>(rc.right-rc.left);
  int panel=clientWidth/3;
  if(panel<290) panel=290;
  if(panel>355) panel=355;
  int drawingW=clientWidth-panel-35;
  if(d){switch(d->result.matchKind){case ant::MatchKind::Gamma:drawGamma(dc,*d,20,65,drawingW,rc.bottom-80);break;case ant::MatchKind::Transformer:drawTransformer(dc,*d,20,65,drawingW,rc.bottom-80);break;case ant::MatchKind::LNetwork:drawLNetwork(dc,*d,20,65,drawingW,rc.bottom-80);break;default:drawDirect(dc,*d,20,65,drawingW,rc.bottom-80);break;}
   line(dc,rc.right-panel-10,60,rc.right-panel-10,rc.bottom-20,GREY,1);txt(dc,rc.right-panel+5,70,panel-20,25,L"CALCULATED DIMENSIONS AND INSTRUCTIONS",BLUE,DT_CENTER|DT_SINGLELINE);txt(dc,rc.right-panel+12,105,panel-30,rc.bottom-125,wide(d->result.matchingPlan),INK);
  }
  SelectObject(dc,old);DeleteObject(font);EndPaint(h,&ps);return 0;}
 case WM_NCDESTROY:delete d;SetWindowLongPtrW(h,GWLP_USERDATA,0);return DefWindowProcW(h,m,w,l);
 }
 return DefWindowProcW(h,m,w,l);
}
}

void showMatchDiagram(HWND owner,const ant::DesignInput&input,const ant::DesignResult&result){
 static bool registered=false;HINSTANCE hi=GetModuleHandleW(nullptr);
 if(!registered){WNDCLASSW c{};c.hInstance=hi;c.lpfnWndProc=proc;c.lpszClassName=L"AntennaMatchDiagram";c.hCursor=LoadCursor(nullptr,IDC_ARROW);c.hIcon=(HICON)LoadImageW(hi,MAKEINTRESOURCEW(IDI_APP_ICON),IMAGE_ICON,0,0,LR_DEFAULTSIZE);if(!c.hIcon)c.hIcon=LoadIcon(nullptr,IDI_APPLICATION);c.hbrBackground=nullptr;registered=RegisterClassW(&c)!=0||GetLastError()==ERROR_CLASS_ALREADY_EXISTS;}
 auto*data=new DiagramData{input,result};HWND h=CreateWindowExW(WS_EX_APPWINDOW,L"AntennaMatchDiagram",L"50-ohm matching construction diagram",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1280,800,owner,nullptr,hi,data);
 if(!h){delete data;return;}ShowWindow(h,SW_SHOW);UpdateWindow(h);
}
