#pragma once


#include <vector>
#include <string>
#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>

namespace dynamit::builders
{

class PolarBuilder
{
public:
    PolarBuilder();

    PolarBuilder& formula(const std::wstring& formula);
    PolarBuilder& formula(const std::string& formula);
    PolarBuilder& domain(float start, float end);
    PolarBuilder& domain(float end);
    PolarBuilder& sectors(int sectors);
    PolarBuilder& slices(int slices);
    PolarBuilder& slices_sectors(int slices, int sectors);
    PolarBuilder& sectors_slices(int sectors, int slices);

    void buildCone(std::vector<float>& verts, std::vector<float>& norms) const;
    void buildConeIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<uint32_t>& indices) const;

private:
    std::wstring m_formula;
    float m_domainStart;
    float m_domainEnd;
    int m_sectors;
    int m_slices;
};

} // namespace dynamit::builders