#include "utils.h"
#include <cmath>
#include <complex>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>  


#define M_PI 3.14159265358979323846

std::map<std::string, std::vector<std::string>> g_args;

// Very hacky and unsafe. Tests?
//
void ParseArguments(int argc, char *argv[]) {
  for (int i = 0; i < argc; ++i) {
    std::stringstream sin(argv[i]);
    std::string key;
    std::getline(sin, key, '=');
    if (!sin.eof()) {
      if (key.size() > 2 && key.substr(0, 2) == "--") {
        key = key.substr(2);
        std::string value;
        while (!sin.eof()) {
          getline(sin, value, ';');
          g_args[key].push_back(value);
        }
      }
    }
  }
}

// Intersect two 3d lines given in the form Pi + s * Vi.
// If the lines don't intersect return a midpoint of a shortest-distance vector
// connecting the lines.
// Source: http://geomalgorithms.com/a07-_distance.html
//
cv::Point3d LineIntersect(cv::Point3d p0, cv::Point3d v0, cv::Point3d p1, cv::Point3d v1) {
  cv::Point3d w0 = p0 - p1;
  double a = v0.dot(v0);
  double b = v0.dot(v1);
  double c = v1.dot(v1);
  double d = v0.dot(w0);
  double e = v1.dot(w0);
  if (std::abs(a * c - b * b) < 1e-7) {
    double t = d / b;
    return (p0 + p1 + t * v1) * 0.5;
  }
  double s = (b * e - c * d) / (a * c - b * b);
  double t = (a * e - b * d) / (a * c - b * b);
  // DEBUG("Line intersection distance: " << norm(p0 + s * v0 - p1 - t * v1));
  return (p0 + s * v0 + p1 + t * v1) * 0.5;
}

bool getParabola(std::vector<cv::Point2d> points, double *a, double *b, double *c) {
  assert(points.size() >= 3);
  double A = points[0].x * points[0].x - points[1].x * points[1].x;
  double B = points[0].x - points[1].x;
  double C = points[0].x * points[0].x - points[2].x * points[2].x;
  double D = points[0].x - points[2].x;
  double E = points[0].y - points[1].y;
  double F = points[0].y - points[2].y;
  double det = A * D - B * C;

  // DEBUG("Extracting parabola from points " << points[0] << " ; "
  //     << points[1] << " ; " << points[2]);

  if (std::abs(det) < 1e-7) {
    // No parabola;
    // This occurs way too often. Examine why!
    //
    // DEBUG("the first three points don't define a parabola");
    return false;
  }
  *a = (D * E - B * F) / det;
  *b = (A * F - C * E) / det;
  *c = points[0].y - *a * points[0].x * points[0].x - *b * points[0].x;
  return true;
}

void drawParabola(cv::Mat frame, double a, double b, double c, cv::Scalar col) {
  std::vector<cv::Point2d> parPoints;
  for (int x = 0; x < frame.cols; x += 10) {
    double y = a * x * x + b * x + c;
    parPoints.emplace_back(cv::Point2d(x, y));
  }
  for (size_t i = 0; i + 1 < parPoints.size(); ++i) {
	cv::line(frame, parPoints[i], parPoints[i + 1], col);
  }
}

double PointToParabolaDistance(cv::Point2d p, double a, double b, double c) {
  std::vector<std::complex<double>> roots;
  bool res = SolveCubic(2.0 * a * a, 3.0 * a * b, 2.0 * a * (c - p.y) + b * b + 1,
                        b * (c - p.y) - p.x, &roots);
  if (!res) {
    return nan("");
  }
  std::vector<double> poi;
  for (std::complex<double> rt : roots) {
    if (std::abs(rt.imag()) < 1e-3) {
      poi.push_back(rt.real());
    }
  }
  poi.push_back(-b * 0.5 / a);
  double best_dist = 1e300;
  for (double x : poi) {
    double y = a * x * x + b * x + c;
    best_dist = std::min(best_dist, cv::norm(cv::Point2d(x, y) - p));
  }
  return best_dist;
}

// General formula from http://en.wikipedia.org/wiki/Cubic_function
// No guarantees for numerical stability.
//
bool SolveCubic(double a, double b, double c, double d,
    std::vector<std::complex<double>> *roots) {
  if (std::abs(a) < 1e-7) {
    // DEBUG("Solving cubic " << a << "*x^3 + " << b << "*x^2 + " << c << "*x + " << d);
    // DEBUG("Equation is not cubic!");
    return SolveQuadratic(b, c, d, roots);
  }
  std::complex<double> w = std::exp(2.0 * M_PI * std::complex<double>(0, 1) / 3.0);
  std::complex<double> del0(b * b - 3.0 * a * c, 0);
  std::complex<double> del1(2.0 * std::pow(b, 3) - 9.0 * a * b * c + 27.0 * a * a * d, 0);
  std::complex<double> tmp = del1 * del1 - 4.0 * std::pow(del0, 3.0);
  // double del = tmp / (-27.0 * a * a);
  std::complex<double> C = std::pow(0.5 * (del1 + std::sqrt(tmp)), 1.0 / 3.0);
  std::complex<double> u[3];
  u[0] = 1;
  u[1] = u[0] * w;
  u[2] = u[1] * w;
  // DEBUG("Solving cubic " << a << "*x^3 + " << b << "*x^2 + " << c << "*x + " << d);
  roots->clear();
  for (int i = 0; i < 3; ++i) {
    roots->push_back(-1.0 / (3.0 * a) * (b + u[i] * C + del0 / (u[i] * C)));
    // DEBUG("Found root " << roots->back().real() << " + i*" << roots->back().imag());
  }
  return true;
}

bool SolveQuadratic(double a, double b, double c,
    std::vector<std::complex<double>> *roots) {
  if (std::abs(a) < 1e-7) {
    // DEBUG("Solving quadratic " << a << "*x^2 + " << b << "*x + " << c);
    // DEBUG("Equation is not a quadratic!");
    return SolveLinear(b, c, roots);
  }
  std::complex<double> det = b * b - 4.0 * a * c;
  double s[2];
  s[0] = 1;
  s[1] = -1;

  roots->clear();
  for (int i = 0; i < 2; ++i) {
    roots->push_back((-b + s[i] * std::sqrt(det)) * 0.5 / a);
  }
  return true;
}

bool SolveLinear(double a, double b, std::vector<std::complex<double>> *roots) {
  // DEBUG("Solving linear " << a << "*x + " << b);
  if (std::abs(a) < 1e-7) {
    // DEBUG("Solving linear " << a << "*x + " << b);
    return false;
  }
  roots->clear();
  roots->push_back(-b / a);
  return true;
}

//===================================================================
// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2  on the line
//            <0 for P2  right of the line
inline double isLeft(cv::Point P0, cv::Point P1, cv::Point P2)
{
	return ((P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y));
}

//===================================================================
// cn_PnPoly(): crossing number test for a point in a polygon
//      Input:   P = a point,
//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
//      Return:  0 = outside, 1 = inside
// This code is patterned after [Franklin, 2000]
// Taken from: http://geomalgorithms.com/a03-_inclusion.html
int cn_PnPoly(cv::Point P, const cv::Point* V, int n)
{
	int    cn = 0;    // the  crossing number counter

					  // loop through all edges of the polygon
	for (int i = 0; i<n; i++) {    // edge from V[i]  to V[i+1]
		if (((V[i].y <= P.y) && (V[i + 1].y > P.y))     // an upward crossing
			|| ((V[i].y > P.y) && (V[i + 1].y <= P.y)))   // a downward crossing
		{
			// compute  the actual edge-ray intersect x-coordinate
			double vt = (double)(P.y - V[i].y) / (V[i + 1].y - V[i].y);
			if (P.x <  V[i].x + vt * (V[i + 1].x - V[i].x)) // P.x < intersect
				++cn;   // a valid crossing of y=P.y right of P.x
		}
	}

	return (cn & 1);    // 0 if even (out), and 1 if  odd (in)
}


//===================================================================
// wn_WindingNumber(): winding number test for a point in a polygon
//      Input:   P = a point,
//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
//      Return:  wn = the winding number (=0 only when P is outside)
// Taken from: http://geomalgorithms.com/a03-_inclusion.html
int wn_WindingNumber(cv::Point P, const cv::Point* V, int n)
{
	int    wn = 0;    // the  winding number counter

					  // loop through all edges of the polygon
	for (int i = 0; i<n; i++) {   // edge from V[i] to  V[i+1]
		if (V[i].y <= P.y) {          // start y <= P.y
			if (V[i + 1].y  > P.y)      // an upward crossing
				if (isLeft(V[i], V[i + 1], P) > 0)  // P left of  edge
					++wn;            // have  a valid up intersect
		}
		else {                           // start y > P.y (no test needed)
			if (V[i + 1].y <= P.y)        // a downward crossing
				if (isLeft(V[i], V[i + 1], P) < 0)  // P right of  edge
					--wn;            // have  a valid down intersect
		}
	}

	return wn;
}

//===================================================================
/*! This function gives answer whether the given point is inside or outside the predefined polygon
*  Unlike standard ray-casting algorithm, this one works on edges! (with performance benefit)
* arguments:
* Polygon - searched polygon
* Point - an arbitrary point that can be inside or outside the polygon
* length - the number of point in polygon (Attention! The list itself has an additional member - the last point coincides with the first)
*
*
* return value:
*  1 - the point is inside the polygon (including the case when point lies on some polygon's line)
*  0 - the point is outside the polygon
* -1 - the point is on edge
*/

int is_inside_sm(const cv::Point* polygon, const cv::Point point, int length)
{
	int intersections = 0;
	double F, /*DY,*/ dy, dy2 = point.y - polygon[0].y;

	for (int ii = 0, jj = 1; ii<length; ++ii, ++jj)
	{
		dy = dy2;
		dy2 = point.y - polygon[jj].y;

		// consider only lines which are not completely above/bellow/right from the point
		if (dy*dy2 <= 0. && (point.x >= polygon[ii].x || point.x >= polygon[jj].x))
		{
			// non-horizontal line
			if ((dy<0.) || (dy2<0.)) // => dy*dy2 != 0
			{
				F = dy*(polygon[jj].x - polygon[ii].x) / (dy - dy2) + polygon[ii].x;

				if (point.x > F) // line is left from the point - the ray moving towards left, will intersect it
					++intersections;
				else if (point.x == F) // point on line
					return -1;
			}

			// # point on upper peak (dy2=dx2=0) or horizontal line (dy=dy2=0 and dx*dx2<=0)
			else if (dy2 == 0. && (point.x == polygon[jj].x ||
				(dy == 0. && (point.x - polygon[ii].x)*(point.x - polygon[jj].x) <= 0.)))
				return -1;
		}
	}

	return intersections & 1;
}
