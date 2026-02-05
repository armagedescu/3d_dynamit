#include "enabler.h"
#include <Dynamit.h>
#include <builders.h>

//#include <geometry.h>
using namespace dynamit;
using namespace dynamit::builders;


int main_cartesianFirst()
{
	std::vector<float> verts, norms, colors;
    std::vector<uint32_t> indices;

    // Build a surface z = sin(x) * cos(y)
    Builder::cartesian()
        .formula("sin(x) * cos(y)")
        //.domain(-3.14, 3.14, -3.14, 3.14)
        //.divisions(20, 20)
        .sectors_slices(100, 40)
        .smooth(true)
        .color(std::array<float, 3>{ 0.2f, 0.6f, 1.0f })
        .buildConeIndexedWithColor(verts, norms, colors, indices);

    // Build a simple plane
    Builder::cartesian()
        //.domain(-1.0f, 1.0f, -1.0f, 1.0f)
        .sectors_slices(100, 40)
        .color(std::array<float, 3>{ 0.5f, 0.5f, 0.5f })
        .buildCylinderIndexedWithColor(verts, norms, colors, indices, scaleMatrix(2.0f, 2.0f, 1.0f));

    //// Build a box
    //Builder::cartesian()
    //    //.domain(-0.5f, 0.5f, -0.5f, 0.5f)
    //    .color(std::array<float, 3>{ 1.0f, 0.0f, 0.0f })
    //    .buildBoxIndexedWithColor(verts, norms, colors, indices);

	return 0;
}

#include "enabler.h"
#ifdef __MAIN_CARTESIAN_FIRST_CPP__
int main() { return main_cartesianFirst(); }
#endif