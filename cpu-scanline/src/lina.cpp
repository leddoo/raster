#include "lina.hpp"



Bool invert_matrix2(M2f matrix, M2f* inverse, Float32 zero_tolerance) {
    auto m00 = matrix.values[0][0];
    auto m01 = matrix.values[0][1];
    auto m10 = matrix.values[1][0];
    auto m11 = matrix.values[1][1];

    auto determinant = m00*m11 - m01*m10;
    if(abs(determinant) <= zero_tolerance) {
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


Bool find_lines_intersection(V2f a0, V2f a1, V2f b0, V2f b1, V2f* ts, Float32 zero_tolerance) {
    auto matrix = M2f::from_columns(a1 - a0, b0 - b1);
    auto inverse = M2f();
    if(invert_matrix2(matrix, &inverse, zero_tolerance) == false) {
        return false;
    }

    if(ts != nullptr) {
        *ts = inverse*(b0 - a0);
    }
    return true;
}

