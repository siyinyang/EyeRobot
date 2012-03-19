#ifndef IMG_H
#define IMG_H

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <assert.h>
#include <time.h>
#include <sstream>
#include <limits>
#include <time.h>

#include <fstream>

#include <iostream>
//#include <Magick++.h>

#include <vector>
#include <list>

#include "dirent.h"
#include "IImage.h"

//#include <boost/array.hpp>



using namespace std;
//using namespace Magick;
//using namespace boost;

class Point2F{
public:
	double v[2];
	double operator [] (const int i) const;
	double& operator [] (const int i);
};

class Point3F{
public:
	double v[3];
	double operator [] (const int i) const;
	double& operator [] (const int i);
};

//typedef boost::array<double,2> Point2F;
//typedef boost::array<double,3> Point3F;


struct WarpIImage
{
  string rawimage_filename;
  string png_filename;
  string feature_filename;

  IImage *rawimage;
  IImage *pngimage;
  IImage *cylimage;

  vector<Point2F> imgCoords;
  vector<Point2F> warpCoords;
};

struct KeyFeature
{
  double xpos, ypos;             /* Subpixel location of keypoint. */
  double scale, orient;           /* Scale and orientation (range [-PI,PI]) */
  vector<unsigned char> descriptors;     /* Vector of descriptor values */
};

struct PCorrespond
{
   Point2F p1;  // Position on image-1
   Point2F p2;  // Position on image-2
};

void ransac_translation( const vector<PCorrespond> &pmatch, 
                         Point2F &vec);

IImage * cyl_wrap(const IImage * img, double f, double k1, double k2);

#endif

