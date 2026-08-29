#pragma once
#include <string>
#include <vector>
#include <complex>

namespace ant {
constexpr double C0 = 299792458.0;
struct Vec3 { double x{}, y{}, z{}; };
struct Segment { Vec3 a, b; double radius{0.002}; };
struct SweepPoint { double fMHz{}, r{}, x{}, swr{}, returnLossDb{}; };
struct PatternPoint { double angleDeg{}, gainDb{}; };

enum class MatchKind { Direct, LNetwork, Transformer, Gamma };

enum class Kind {
 Dipole, GroundPlane, Astroplane, DeltaLoop, Yagi, T2LT, Moxon,
 InvertedV, EndFed, JPole, CubicalQuad
};
struct DesignInput {
 Kind kind{Kind::Dipole}; double fMHz{27.205}; double targetZ{50.0};
 double conductorMm{2.0}; double velocityFactor{0.95}; double heightM{8.0};
 int elements{5};
 double measuredR{0.0}; double measuredX{0.0}; double powerW{100.0};
};
struct DesignResult {
 std::string name, band, matching, matchingPlan, notes, warning;
 double wavelengthM{}, designZ{}, gainDbi{}, fbDb{}, bandwidthKHz{};
 double nativeR{}, nativeX{}, matchedR{}, matchedX{}, matchedSWR{};
 double resonanceMHz{}, minSWR{}, swr2LowMHz{}, swr2HighMHz{}, reflectedPercent{}, mismatchLossDb{};
 MatchKind matchKind{MatchKind::Direct}; bool matchBalanced{}; bool shuntOnAntenna{};
 double gammaRodM{}, gammaSpacingM{}, gammaRodDiameterM{}, gammaSeriesCapPf{}, gammaSeriesInductorUh{};
 int turns50{}, turnsAntenna{};
 double matchSeriesX{}, matchShuntX{}, matchCompensationX{};
 std::vector<std::pair<std::string,double>> dimensions;
 std::vector<Segment> geometry;
 std::vector<SweepPoint> sweep;
 std::vector<PatternPoint> azimuth, elevation;
};
std::vector<std::pair<Kind,std::string>> catalog();
DesignResult calculate(const DesignInput& in);
std::string makeReport(const DesignInput& in, const DesignResult& r);
}
