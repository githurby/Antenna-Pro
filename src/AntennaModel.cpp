#include "AntennaModel.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ant {
static constexpr double PI=3.14159265358979323846;
std::vector<std::pair<Kind,std::string>> catalog(){ return {
 {Kind::Dipole,"Half-wave Dipole"},
 {Kind::GroundPlane,"Quarter-wave Ground Plane"},
 {Kind::Astroplane,"Astroplane"},
 {Kind::DeltaLoop,"Delta Loop (1 to 5 elements)"},
 {Kind::Yagi,"Yagi-Uda (1 to 5 elements)"},
 {Kind::T2LT,"T2LT coaxial"},
 {Kind::Moxon,"Moxon / Skypper"},
 {Kind::InvertedV,"Inverted-V Dipole"},
 {Kind::EndFed,"End-Fed + UNUN"},
 {Kind::JPole,"J-Pole"},
 {Kind::CubicalQuad,"Cubical Quad (1 to 5 elements)"}
}; }
static void seg(DesignResult& r, Vec3 a, Vec3 b, double rad=.002){r.geometry.push_back({a,b,rad});}
static double clamp(double v,double a,double b){return std::max(a,std::min(b,v));}
static double swr(std::complex<double> z,double z0){auto g=(z-z0)/(z+z0); double m=clamp(std::abs(g),0.,.999999); return (1+m)/(1-m);}
static void patterns(DesignResult& r, Kind k, int n){
 for(int d=0;d<=360;d+=2){
  double a=d*PI/180., p=1;
  if(k==Kind::Yagi) {
   if(n==1) p=.02+.98*std::abs(std::cos(a));
   else p=std::pow(std::max(0.0,(1+std::cos(a))/2),1.2+n*.16)+std::pow(std::max(0.0,(1-std::cos(a))/2),2)*std::pow(10.,-r.fbDb/20.);
  } else if(k==Kind::Moxon) {
   p=std::pow(std::max(0.0,(1+std::cos(a))/2),1.5)+.08*std::pow(std::max(0.0,(1-std::cos(a))/2),2);
  } else if(k==Kind::CubicalQuad||k==Kind::DeltaLoop) {
   if(n==1) p=.18+.82*std::abs(std::cos(a));
   else p=std::pow(std::max(0.0,(1+std::cos(a))/2),1.15+n*.14)+std::pow(std::max(0.0,(1-std::cos(a))/2),2)*std::pow(10.,-r.fbDb/20.);
  } else p=1.0;
  r.azimuth.push_back({(double)d,20*std::log10(std::max(.001,p))});
  double e=std::abs(std::sin(a));
  if(k==Kind::GroundPlane||k==Kind::JPole||k==Kind::T2LT||k==Kind::Astroplane) e=std::abs(std::cos(a));
  r.elevation.push_back({(double)d,20*std::log10(std::max(.001,e))});
 }
}

static double swrRX(double r,double x,double z0){return swr({r,x},z0);}
static const char* rangeAdvice(double fMHz){
 if(fMHz<10) return "Lower HF: use mix 31/43 ferrite and short leads; check heating at full power.";
 if(fMHz<30) return "HF: use mix 43 ferrite and short leads; validate insertion loss and temperature.";
 if(fMHz<300) return "VHF: prefer transmission-line/coaxial structures and ferrite sleeves; lumped components need an SRF well above the operating frequency.";
 return "UHF/SHF: parasitic dimensions dominate; use printed/coaxial structures and validate with a VNA rather than conventional coils.";
}
static std::string componentForReactance(double x,double fHz,const char* position){
 std::ostringstream o;o<<std::fixed<<std::setprecision(2);
 if(std::abs(x)<.05){o<<position<<": direct connection";return o.str();}
 if(x>0){double uH=x/(2*PI*fHz)*1e6;o<<position<<": inductor "<<uH<<" uH (X="<<x<<" ohm)";}
 else {double pF=1.0/(2*PI*fHz*std::abs(x))*1e12;o<<position<<": capacitor "<<pF<<" pF (X="<<x<<" ohm)";}
 return o.str();
}
static void makeGamma(const DesignInput& in,DesignResult& r,double ra,double xa){
 const double fHz=in.fMHz*1e6, lam=r.wavelengthM, D=std::max(.001,in.conductorMm/1000.0);
 const double d=std::max(.001,D*.5), S=std::max(.007*lam,1.1*(D+d)/2);
 double arg=(4*S*S-D*D-d*d)/(2*D*d); arg=std::max(1.000001,arg);
 const double zstub=59.9585*std::acosh(arg);
 double a1=(4*S*S-D*D+d*d)/(4*S*d),a2=(4*S*S+D*D-d*d)/(4*S*D);
 a1=std::max(1.000001,a1);a2=std::max(1.000001,a2);
 const double su=std::pow(1+std::acosh(a1)/std::acosh(a2),2);
 double bestErr=1e99,bestL=.05*lam;std::complex<double> bestZ;
 for(int i=0;i<=2000;i++){
  double L=lam*(.005+.115*i/2000.0),xg=zstub*std::tan(2*PI*L/lam);
  if(std::abs(xg)<1e-9)continue;
  std::complex<double> za=su*std::complex<double>(ra,xa);
  std::complex<double> zout=1.0/(1.0/std::complex<double>(0,xg)+1.0/za);
  double err=std::abs(zout.real()-in.targetZ);
  if(err<bestErr){bestErr=err;bestL=L;bestZ=zout;}
 }
 double seriesX=-bestZ.imag();
 r.matchKind=MatchKind::Gamma;r.matchBalanced=false;r.gammaRodM=bestL;r.gammaSpacingM=S;r.gammaRodDiameterM=d;r.matchSeriesX=seriesX;
 if(seriesX<0)r.gammaSeriesCapPf=1.0/(2*PI*fHz*std::abs(seriesX))*1e12;else if(seriesX>0)r.gammaSeriesInductorUh=seriesX/(2*PI*fHz)*1e6;
 r.matchedR=bestZ.real();r.matchedX=bestZ.imag()+seriesX;r.matchedSWR=swrRX(r.matchedR,r.matchedX,in.targetZ);
 std::ostringstream o;o<<std::fixed<<std::setprecision(3);
 o<<"CALCULATED GAMMA MATCH (starting point):\r\n"
  <<"- Native impedance used: "<<ra<<(xa>=0?" + j":" - j")<<std::abs(xa)<<" ohm\r\n"
  <<"- Driven element tube D: "<<D*1000<<" mm; gamma rod: "<<d*1000<<" mm\r\n"
  <<"- Center-to-center spacing: "<<S*1000<<" mm\r\n"
  <<"- Initial gamma rod length: "<<bestL*1000<<" mm ("<<bestL/lam<<" lambda)\r\n"
  <<"- Geometric step-up factor SU: "<<su<<"; stub Z0: "<<zstub<<" ohm\r\n"
  <<"- "<<componentForReactance(seriesX,fHz,"Series component in gamma arm")<<"\r\n"
  <<"- Theoretical result: "<<r.matchedR<<" + j"<<r.matchedX<<" ohm; SWR "<<r.matchedSWR<<":1\r\n"
  <<"Use an adjustable rod and sliding shorting strap. Use a variable capacitor spanning approximately 0.5x to 1.5x the calculated value. Add a 1:1 common-mode choke to the coax. ";
 o<<rangeAdvice(in.fMHz);
 r.matchingPlan=o.str();
}
static void makeTransformer(const DesignInput& in,DesignResult& r,double ra,double xa,bool balanced,const char* preferred){
 double ideal=std::sqrt(std::max(.001,ra/in.targetZ));int low=1,high=1;double err=1e99;
 for(int n=2;n<=14;n++){int h=std::max(1,(int)std::lround(n*ideal));double ratio=(double)h/n;double e=std::abs(ratio-ideal);if(e<err){err=e;low=n;high=h;}}
 double n=(double)high/low, zr=n*n;r.matchKind=MatchKind::Transformer;r.matchBalanced=balanced;r.turns50=low;r.turnsAntenna=high;r.matchCompensationX=-xa;r.matchedR=ra/zr;double residualX=xa/zr;
 r.matchedX=std::abs(xa)>.05?0:residualX;r.matchedSWR=swrRX(r.matchedR,r.matchedX,in.targetZ);
 std::ostringstream o;o<<std::fixed<<std::setprecision(3);
 o<<(balanced?"BALUN":"UNUN")<<" impedance transformer:\r\n"
  <<"- Native impedance used: "<<ra<<(xa>=0?" + j":" - j")<<std::abs(xa)<<" ohm\r\n"
  <<"- Ideal impedance ratio: "<<ra/in.targetZ<<":1; ideal turns ratio: "<<ideal<<":1\r\n"
  <<"- Practical integer-turn approximation: "<<low<<":"<<high<<" turns (50-ohm side : antenna side), Z-ratio "<<zr<<":1\r\n";
 if(preferred&&*preferred)o<<"- Recommended configuration: "<<preferred<<"\r\n";
 o<<"- Before reactance compensation: "<<r.matchedR<<(residualX>=0?" + j":" - j")<<std::abs(residualX)<<" ohm\r\n";
 if(std::abs(xa)>.05)o<<"- "<<componentForReactance(-xa,in.fMHz*1e6,"Series compensation on antenna side")<<"\r\n";
 o<<"- After compensation: "<<r.matchedR<<(r.matchedX>=0?" + j":" - j")<<std::abs(r.matchedX)<<" ohm; SWR "<<r.matchedSWR<<":1\r\n";
 o<<"First test with an equivalent non-inductive resistive load, then measure at the antenna feed reference plane. "<<rangeAdvice(in.fMHz);
 r.matchingPlan=o.str();
}
static void makeLNetwork(const DesignInput& in,DesignResult& r,double ra,double xa,bool balanced){
 const double z0=in.targetZ,fHz=in.fMHz*1e6;double hi=std::max(ra,z0),lo=std::min(ra,z0);
 if(std::abs(ra-z0)<.5){r.matchedR=ra;r.matchedX=0;r.matchedSWR=swrRX(ra,0,z0);}
 double qn=std::sqrt(std::max(0.0,hi/lo-1)),xs=qn*lo,xp=qn>1e-9?hi/qn:1e99;
 r.matchKind=MatchKind::LNetwork;r.matchBalanced=balanced;r.matchSeriesX=qn>.001?xs:0;r.matchShuntX=qn>.001?-xp:0;r.matchCompensationX=-xa;r.shuntOnAntenna=ra>z0;
 std::ostringstream o;o<<std::fixed<<std::setprecision(3);
 o<<"LOW-PASS L NETWORK for 50 ohms"<<(balanced?" + 1:1 current balun":" + 1:1 choke")<<":\r\n"
  <<"- Native impedance used: "<<ra<<(xa>=0?" + j":" - j")<<std::abs(xa)<<" ohm\r\n";
 if(std::abs(xa)>.05)o<<"- "<<componentForReactance(-xa,fHz,"Series compensation at antenna feed")<<"\r\n";
 if(qn>.001){o<<"- Minimum loaded Q: "<<qn<<"\r\n";
  if(ra>z0)o<<"- "<<componentForReactance(xs,fHz,"Series arm on 50-ohm side")<<"\r\n- "<<componentForReactance(-xp,fHz,"Shunt arm on antenna side")<<"\r\n";
  else o<<"- "<<componentForReactance(xs,fHz,"Series arm on antenna side")<<"\r\n- "<<componentForReactance(-xp,fHz,"Shunt arm on 50-ohm side")<<"\r\n";
 }
 r.matchedR=z0;r.matchedX=0;r.matchedSWR=1;
 o<<"- Ideal center-frequency result: "<<z0<<" + j0 ohm; SWR 1,000:1\r\n";
 o<<(balanced?"Place the 1:1 current balun after the network on the balanced side. ":"The 1:1 choke blocks common-mode current without transforming impedance. ");
 o<<rangeAdvice(in.fMHz);
 r.matchingPlan=o.str();
}
static void designMatching(const DesignInput& in,DesignResult& r){
 double ra=in.measuredR>0?in.measuredR:r.designZ,xa=in.measuredR>0?in.measuredX:0;
 ra=std::max(.1,ra);r.nativeR=ra;r.nativeX=xa;
 bool balanced=in.kind==Kind::Dipole||in.kind==Kind::InvertedV||in.kind==Kind::DeltaLoop||in.kind==Kind::Yagi||in.kind==Kind::Moxon||in.kind==Kind::CubicalQuad;
 if(in.kind==Kind::Yagi&&in.elements>1&&ra<in.targetZ*1.25){makeGamma(in,r,ra,xa);}
 else if(in.kind==Kind::EndFed){makeTransformer(in,r,ra,xa,false,"49:1 UNUN when the load is near 2450 ohms; 2:14 turns plus a separate common-mode choke");}
 else if(in.kind==Kind::DeltaLoop||in.kind==Kind::CubicalQuad){makeTransformer(in,r,ra,xa,true,"Current transformer; for 100 ohms use approximately 10:14 turns (2:1 impedance ratio)");}
 else if(std::abs(ra-in.targetZ)<=in.targetZ*.1&&std::abs(xa)<=5){r.matchKind=MatchKind::Direct;r.matchBalanced=balanced;r.matchedR=ra;r.matchedX=xa;r.matchedSWR=swrRX(ra,xa,in.targetZ);std::ostringstream o;o<<std::fixed<<std::setprecision(3)<<"Direct 50-ohm connection using a "<<(balanced?"1:1 current balun":"1:1 common-mode choke")<<". Predicted Z: "<<ra<<(xa>=0?" + j":" - j")<<std::abs(xa)<<" ohm; SWR "<<r.matchedSWR<<":1. "<<rangeAdvice(in.fMHz);r.matchingPlan=o.str();}
 else makeLNetwork(in,r,ra,xa,balanced);
 double p=std::max(0.1,in.powerW),vrms=std::sqrt(p*in.targetZ),irms=std::sqrt(p/in.targetZ);
 std::ostringstream safety;safety<<std::fixed<<std::setprecision(2)<<"\r\nIdeal electrical ratings at "<<p<<" W: 50-ohm side = "<<vrms<<" Vrms, "<<vrms*std::sqrt(2.0)<<" Vpeak, "<<irms<<" Arms. Use at least a 2x voltage/current safety margin and account for SWR, losses, and modulation.";
 r.matchingPlan+=safety.str();
}
DesignResult calculate(const DesignInput& in){
 DesignResult r; const double f=std::max(.1,in.fMHz), vf=clamp(in.velocityFactor,.5,1.0); r.wavelengthM=C0/(f*1e6);
 const double lam=r.wavelengthM, half=lam*.475, q=8.0+clamp(2.0/in.conductorMm,0.,15.); int ne=std::max(1,std::min(5,in.elements));
 auto dim=[&](const char* n,double v){r.dimensions.push_back({n,v});};
 switch(in.kind){
 case Kind::Dipole:{r.name="Half-wave Dipole";r.designZ=72;r.gainDbi=2.15;r.fbDb=0;r.bandwidthKHz=f*1000/q;dim("Total length",half);dim("Each leg",half/2);seg(r,{-half/2,0,0},{-.01,0,0});seg(r,{.01,0,0},{half/2,0,0});r.matching="1:1 current balun; a free-space dipole is approximately 72 ohms (1.44:1 SWR on 50 ohms). Use suitable geometry/height or the calculated L network for a true 50-ohm system.";break;}
 case Kind::GroundPlane:{double el=lam*.238,ra=lam*.25;r.name="Quarter-wave Ground Plane";r.designZ=50;r.gainDbi=2.1;r.bandwidthKHz=f*1000/(q*.75);dim("Vertical element",el);dim("Each radial",ra);dim("Radial angle (degrees)",45);seg(r,{0,0,0},{0,0,el});for(int i=0;i<4;i++){double a=i*PI/2;seg(r,{0,0,0},{ra*cos(a),ra*sin(a),-ra*.707});}r.matching="Radials sloped by 45 degrees bring the feed resistance close to 50 ohms.";break;}
 case Kind::Astroplane:{double v=lam*.24, rr=lam*.18;r.name="Astroplane";r.designZ=50;r.gainDbi=3.0;r.bandwidthKHz=f*1000/(q*.8);dim("Upper vertical section",v);dim("Basket radius",rr);dim("Overall height",lam*.52);seg(r,{0,0,0},{0,0,v});for(int i=0;i<4;i++){double a=i*PI/2;seg(r,{0,0,0},{rr*cos(a),rr*sin(a),-v});seg(r,{rr*cos(a),rr*sin(a),-v},{0,0,-v*1.15});}r.matching="Adjust the feed/shorting point on the ring using a vector network analyzer.";break;}
 case Kind::DeltaLoop:{
  double per=lam*.99, side=per/3, triH=side*.866, spacing=lam*.15;
  r.name="Delta Loop "+std::to_string(ne)+(ne==1?" element":" elements");
  r.band="Based on the selected frequency"; r.designZ=100;
  r.gainDbi=3.2+1.15*(ne-1); r.fbDb=ne==1?0:10+2*(ne-2);
  r.bandwidthKHz=f*1000/(q*.65);
  dim("Number of elements",ne); dim("Perimeter of each loop",per);
  dim("Each side",side); dim("Triangle height",triH);
  if(ne>1){dim("Element spacing",spacing);dim("Boom length",spacing*(ne-1));}
  for(int i=0;i<ne;i++){
   double x=i*spacing;
   seg(r,{x,-side/2,0},{x,side/2,0});
   seg(r,{x,side/2,0},{x,0,triH});
   seg(r,{x,0,triH},{x,-side/2,0});
   if(i) seg(r,{(i-1)*spacing,0,triH*.45},{x,0,triH*.45},.006);
  }
  r.matching="Driven loop with a 2:1 impedance transformer (approximately 100 to 50 ohms). For arrays, optimize parasitic-loop spacing and perimeter with a full-wave solver and VNA.";
  break;
 }
 case Kind::Yagi:{
  r.name="Yagi-Uda "+std::to_string(ne)+(ne==1?" element":" elements");
  r.band="Based on the selected frequency";
  r.designZ=ne==1?50:(ne==2?35:28);
  static const double gains[5]={2.15,4.5,6.0,7.2,8.2};
  r.gainDbi=gains[ne-1]; r.fbDb=ne==1?0:(ne==2?8:12+2*(ne-3));
  r.bandwidthKHz=f*1000/(q*.7); double spacing=lam*.18;
  dim("Number of elements",ne); dim("Driven element",lam*.475);
  if(ne>=2) dim("Reflector",lam*.51);
  if(ne>=3) dim("Director 1",lam*.45);
  dim("Boom length",ne==1?0:spacing*(ne-1));
  // Driven element at x=0; reflector behind it; the remaining elements are directors.
  seg(r,{0,-lam*.475/2,0},{0,lam*.475/2,0},.004);
  if(ne>=2){seg(r,{-spacing,-lam*.51/2,0},{-spacing,lam*.51/2,0},.004);seg(r,{-spacing,0,0},{0,0,0},.008);}
  for(int i=0;i<ne-2;i++){double x=(i+1)*spacing,len=lam*(.45-.006*i);seg(r,{x,-len/2,0},{x,len/2,0},.004);seg(r,{x-spacing,0,0},{x,0,0},.008);}
  if(ne==1) r.matching="Single driven element; use a 1:1 current balun and tune for 50 ohms.";
  else r.matching="Use an adjusted gamma or hairpin match to transform the calculated feed impedance to 50 ohms; a feed-line choke is recommended.";
  break;
 }
 case Kind::T2LT:{double rad=lam*.25*vf,ch=lam*.25*vf;r.name="T2LT coaxial";r.designZ=50;r.gainDbi=2.15;r.bandwidthKHz=f*1000/(q*.85);dim("Coaxial radiator",rad);dim("Sleeve choke",ch);dim("Active length",rad+ch);seg(r,{0,0,0},{0,0,rad});seg(r,{0,0,0},{0,0,-ch},.008);r.matching="The outer sleeve forms the quarter-wave choke; confirm the cable actual velocity factor.";break;}
 case Kind::Moxon:{double w=lam*.36,h=lam*.14,g=lam*.02;r.name="Moxon / Skypper";r.designZ=50;r.gainDbi=5.8;r.fbDb=20;r.bandwidthKHz=f*1000/(q*.8);dim("Width",w);dim("Depth",h);dim("End gap",g);seg(r,{-w/2,0,0},{w/2,0,0});seg(r,{-w/2,0,0},{-w/2,h,0});seg(r,{w/2,0,0},{w/2,h,0});seg(r,{-w/2,h+g,0},{w/2,h+g,0});r.matching="Normally close to 50 ohms; a 1:1 current balun is recommended.";break;}
 case Kind::InvertedV:{double side=half/2,ang=110*PI/180;r.name="Inverted-V Dipole";r.designZ=50;r.gainDbi=1.8;r.bandwidthKHz=f*1000/q;dim("Total length",half);dim("Each leg",side);dim("Included angle (degrees)",110);seg(r,{0,0,0},{-side*sin(ang/2),0,-side*cos(ang/2)});seg(r,{0,0,0},{side*sin(ang/2),0,-side*cos(ang/2)});r.matching="1:1 current balun; the included angle brings the feed resistance close to 50 ohms.";break;}
 case Kind::EndFed:{r.name="End-Fed Half-Wave";r.designZ=2450;r.gainDbi=2.0;r.bandwidthKHz=f*1000/(q*1.3);dim("Main wire",half);dim("Initial counterpoise",lam*.05);seg(r,{0,0,0},{half,0,0});seg(r,{0,0,0},{0,-lam*.05,0});r.matching="49:1 UNUN (7:1 turns ratio), common-mode choke, and counterpoise; expect high end voltage.";break;}
 case Kind::JPole:{double l=lam*.75,s=lam*.25,g=lam*.02;r.name="J-Pole";r.designZ=50;r.gainDbi=2.4;r.bandwidthKHz=f*1000/(q*.7);dim("Long element",l);dim("Short stub",s);dim("Spacing",g);dim("Initial tap height",lam*.025);seg(r,{0,0,0},{0,0,l},.006);seg(r,{g,0,0},{g,0,s},.006);seg(r,{0,0,0},{g,0,0},.006);r.matching="Move both feed taps symmetrically along the stub until 50 ohms is obtained; add a coax choke.";break;}
 case Kind::CubicalQuad:{
  double side=lam*.25, spacing=lam*.18;
  r.name="Cubical Quad "+std::to_string(ne)+(ne==1?" element":" elements");
  r.band="Based on the selected frequency"; r.designZ=ne==1?110:75;
  r.gainDbi=3.1+1.25*(ne-1); r.fbDb=ne==1?0:12+2*(ne-2);
  r.bandwidthKHz=f*1000/(q*.75);
  dim("Number of elements",ne); dim("Driven-element side",side);
  dim("Driven-element perimeter",4*side);
  if(ne>1){dim("Element spacing",spacing);dim("Boom length",spacing*(ne-1));}
  for(int i=0;i<ne;i++){
   double x=i*spacing, scale=(i==0?1.0:(.98-.008*(i-1))), ss=side*scale;
   seg(r,{x,-ss/2,-ss/2},{x,ss/2,-ss/2});
   seg(r,{x,ss/2,-ss/2},{x,ss/2,ss/2});
   seg(r,{x,ss/2,ss/2},{x,-ss/2,ss/2});
   seg(r,{x,-ss/2,ss/2},{x,-ss/2,-ss/2});
   if(i) seg(r,{(i-1)*spacing,0,0},{x,0,0},.008);
  }
  r.matching=ne==1?"Use a roughly 2:1 impedance transformer or a gamma match for approximately 110 to 50 ohms.":"Use a gamma/matching network for 50 ohms; optimize parasitic-element spacing and perimeters.";
  break;
 }
 }
 if(f<3.0) r.band="MF / lower frequencies";
 else if(f<30.0) r.band="HF (3–30 MHz)";
 else if(f<300.0) r.band="VHF (30–300 MHz)";
 else if(f<3000.0) r.band="UHF (300–3000 MHz)";
 else r.band="SHF (above 3 GHz)";
 designMatching(in,r);
 r.notes="Dimensions are electromagnetic starting points; ground, masts, insulators, and nearby objects alter tuning.";
 r.warning="DO NOT use this as the only structural or electrical validation. Confirm with a VNA, dummy load, and low power before transmitting.";
 // Local equivalent RLC sweep model, including the ideal recommended matching network.
 double transformedR=std::max(.1,r.matchedR);
 for(int i=0;i<=240;i++){double ratio=.88+i*(.24/240.),ff=f*ratio;double rr=transformedR*(1+.22*std::pow((ratio-1)*q,2));double xx=r.matchedX+transformedR*q*(ratio-1/ratio);std::complex<double> z(rr,xx);double s=swr(z,in.targetZ);double gl=(s-1)/(s+1);r.sweep.push_back({ff,rr,xx,s,-20*log10(std::max(1e-9,gl))});}
 auto best=std::min_element(r.sweep.begin(),r.sweep.end(),[](const SweepPoint&a,const SweepPoint&b){return a.swr<b.swr;});
 r.resonanceMHz=best->fMHz;r.minSWR=best->swr;r.swr2LowMHz=r.resonanceMHz;r.swr2HighMHz=r.resonanceMHz;
 bool found=false;for(const auto&p:r.sweep)if(p.swr<=2.0){if(!found){r.swr2LowMHz=p.fMHz;found=true;}r.swr2HighMHz=p.fMHz;}
 r.bandwidthKHz=(r.swr2HighMHz-r.swr2LowMHz)*1000.0;
 double gamma=(r.matchedSWR-1)/(r.matchedSWR+1);r.reflectedPercent=100*gamma*gamma;r.mismatchLossDb=-10*std::log10(std::max(1e-12,1-gamma*gamma));
 patterns(r,in.kind,ne); return r;
}
std::string makeReport(const DesignInput& in,const DesignResult& r){std::ostringstream o;o<<std::fixed<<std::setprecision(3);o<<"ANTENNA PRO - ENGINEERING DESIGN SHEET\r\n================================\r\n"<<"Model: "<<r.name<<"\r\nFrequency: "<<in.fMHz<<" MHz\r\nSystem impedance: "<<in.targetZ<<" ohm\r\nWavelength: "<<r.wavelengthM<<" m\r\nEstimated gain: "<<r.gainDbi<<" dBi\r\nEstimated F/B: "<<r.fbDb<<" dB\r\n"<<"Estimated resonance: "<<r.resonanceMHz<<" MHz; Minimum SWR: "<<r.minSWR<<":1\r\n"<<"SWR <= 2 range: "<<r.swr2LowMHz<<" to "<<r.swr2HighMHz<<" MHz ("<<r.bandwidthKHz<<" kHz)\r\n"<<"Reflected power: "<<r.reflectedPercent<<" %; mismatch loss: "<<r.mismatchLossDb<<" dB\r\n\r\nDIMENSIONS\r\n";for(auto&d:r.dimensions){
 if(d.first=="Number of elements") o<<"- "<<d.first<<": "<<(int)d.second<<"\r\n";
 else if(d.first.find("Angle")!=std::string::npos) o<<"- "<<d.first<<": "<<d.second<<" degrees\r\n";
 else o<<"- "<<d.first<<": "<<d.second<<" m\r\n";
}o<<"\r\nBASE MATCHING GUIDANCE\r\n"<<r.matching<<"\r\n\r\nAUTOMATIC 50-OHM MATCHING DESIGN\r\n"<<r.matchingPlan<<"\r\n\r\nZ before: "<<r.nativeR<<(r.nativeX>=0?" + j":" - j")<<std::abs(r.nativeX)<<" ohm\r\nZ after: "<<r.matchedR<<(r.matchedX>=0?" + j":" - j")<<std::abs(r.matchedX)<<" ohm; SWR "<<r.matchedSWR<<":1\r\n\r\nNOTES\r\n"<<r.notes<<"\r\n"<<r.warning<<"\r\n\r\nSWR model: local equivalent RLC circuit, not a full-wave field solver.\r\n";return o.str();}
}
