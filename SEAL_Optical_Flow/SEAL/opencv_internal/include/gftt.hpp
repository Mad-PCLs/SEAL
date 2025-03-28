#pragma once

namespace cv_internal
{
    struct greaterThanPtr {
        bool operator()(const float * a, const float * b) const

        // Ensure a fully deterministic result of the sort
        { return (*a > *b) ? true : (*a < *b) ? false : (a > b); }
    };
}
