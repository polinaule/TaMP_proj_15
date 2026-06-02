#ifndef CHORD_H
#define CHORD_H

#include <functional>
double chord_method(double a, double b, double eps, std::function<double(double)> f);

#endif
