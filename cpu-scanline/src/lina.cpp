#include "lina.hpp"



Bool invert_matrix2(M2f matrix, M2f* inverse, Float32 tolerance) {
    auto m00 = matrix.values[0][0];
    auto m01 = matrix.values[0][1];
    auto m10 = matrix.values[1][0];
    auto m11 = matrix.values[1][1];

    auto determinant = m00*m11 - m01*m10;
    if(abs(determinant) < tolerance) {
        return false;
    }

    if(inverse != nullptr) {
        auto s = 1.0f/determinant;
        inverse->values[0][0] =  s*m11;
        inverse->values[0][1] = -s*m01;
        inverse->values[1][0] = -s*m10;
        inverse->values[1][1] =  s*m00;
    }

    return true;
}
