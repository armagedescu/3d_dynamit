#pragma once


#include <vector>
#include <string>
#include <cstdint>

namespace dynamit::builders
{

// Build indexed cone geometry - reduces vertex duplication
void buildConePolarIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices,
    const std::wstring& formula,
    float domain_start,
    float domain_end,
    int nsectors = 5,
    int nslices = 1);

void buildConePolarIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices,
    const std::wstring& formula,
    int nsectors = 5,
    int nslices = 1);

// Build cone geometry using indexed version internally, then expand to flat arrays
void buildConePolar(
    std::vector<float>& verts,
    std::vector<float>& norms,
    const std::wstring& formula,
    float domain_start,
    float domain_end,
    int nsectors = 5,
    int nslices = 1);

void buildConePolar(
    std::vector<float>& verts,
    std::vector<float>& norms,
    const std::wstring& formula,
    int nsectors = 5,
    int nslices = 1);

} // namespace dynamit::builders