/* File      : light.cpp
 * Program   : mrtp
 * Copyright : Mikolaj Feliks  <mikolaj.feliks@gmail.com>
 * License   : LGPL v3  (http://www.gnu.org/licenses/gpl-3.0.en.html)
 */
#include "light.hpp"

using namespace Eigen;


/*
================
CLight
================
*/
CLight::CLight (float *center) {
    center_ = (Vector3f *)center;
}

/*
================
~CLight
================
*/
CLight::~CLight () {
}

/*
================
CalculateRay

Calculates a ray between a hit point and 
a source of light
================
*/
Vector3f CLight::CalculateRay (Vector3f *hit) {
    return ((*center_) - (*hit));
}