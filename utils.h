#ifndef UTILS_H__
#define UTILS_H__

#include <complex>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <opencv2/core/core.hpp>

// Debug prints stuff only when not in release mode
//
#ifdef NDEBUG
#define DEBUG(x) do {} while (0)
#else
#define DEBUG(x) do { std::cerr << x << std::endl; } while (0)
#endif

// Info always prints stuff to stderr
#define INFO(x) do { std::cerr << x << std::endl; } while (0)

extern std::map<std::string, std::vector<std::string>> g_args;

void ParseArguments(int argc, char *argv[]);

// Maybe split out maths utils in separate file.
//
cv::Point3d LineIntersect(cv::Point3d p0, cv::Point3d v0, cv::Point3d p1, cv::Point3d v1);

// For now just use the first three points. Do least squares for >3 points if
// needed. Parabola is y = a * x^2 + b * x + c
//
bool getParabola(std::vector<cv::Point2d> points, double *a, double *b, double *c);

// Draws a parabola on frame. Probably not useful at all.
//
void drawParabola(cv::Mat frame, double a, double b, double c, cv::Scalar col);

// Finds the shortest distance from a point to a parabola.
// Simply bashes out the algebra and finds the roots of the derivative (3rd degree)
//
double PointToParabolaDistance(cv::Point2d p, double a, double b, double c);

bool SolveCubic(double a, double b, double c, double d,
    std::vector<std::complex<double>> *roots);

bool SolveQuadratic(double a, double b, double c,
    std::vector<std::complex<double>> *roots);

bool SolveLinear(double a, double b, std::vector<std::complex<double>> *roots);

#endif
