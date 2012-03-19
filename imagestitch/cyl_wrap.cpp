#include "stdafx.h"
#include "ImageStitch.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
void cylpolar_to_imgxy( double theta, double h, double radius, 
                        double &x, double &y)
{
    x = radius*tan(theta);
    y = h*sqrt(x*x + radius*radius);
}

////////////////////////////////////////////////////////////////////////////

void cylxy_to_imgxy( double xc, double yc, double radius, 
                      double &x, double &y)
{
    double theta = xc/radius;
    double h     = yc/radius;
    cylpolar_to_imgxy( theta, h, radius, x, y);
}

////////////////////////////////////////////////////////////////////////////

void imgxy_to_cylpolar( double x, double y, double radius, double &theta, double &h)
{
    theta = atan(x/radius);
    h     = y/sqrt(x*x + radius*radius );
}

////////////////////////////////////////////////////////////////////////////

void imgxy_to_cylxy( double x, double y, double radius,  
                     double &xc, double &yc)
{
    double theta, h;
    imgxy_to_cylpolar( x, y, radius, theta, h);
    xc  = radius*theta;
    yc  = radius*h;
}

////////////////////////////////////////////////////////////////////////////

void getColor( const IImage *img, int x, int y, double *rgb)
{
   Color pixelColor = img->pixelColor(y,x);
   rgb[0] = pixelColor.redQuantum();
   rgb[1] = pixelColor.greenQuantum();
   rgb[2] = pixelColor.blueQuantum();
}

////////////////////////////////////////////////////////////////////////////

double getChannelColor( const IImage *img, int x, int y, int whichColor)
{
     Color pixelColor = img->pixelColor(y,x);

     switch( whichColor )
     {
         case 0:
			 return pixelColor.redQuantum();
         case 1:
			 return pixelColor.greenQuantum();
         case 2:
			 return pixelColor.blueQuantum();
     }
     return 0.0;
}
////////////////////////////////////////////////////////////////////////////

void bilinear_weights( double x, double y, double *weight)
{
   weight[0] = (1-x)*(1-y);
   weight[1] = x*(1-y);
   weight[2] = x*y;
   weight[3] = (1-x)*y;
}

////////////////////////////////////////////////////////////////////////////

int bilinear_interpolate( const IImage *img, double x, double y, double *rgb)
{
    int xmax     = img->columns();
    int ymax     = img->rows();
    double xmid   = 0.5*xmax;
    double ymid   = 0.5*ymax;

    x += xmid;
    y += ymid;

    if( x < 0.0 || x > xmax ) {
        rgb[0] = 0.0;
        rgb[1] = 0.0;
        rgb[2] = 0.0;
        return 1;
    }

    if( y < 0.0 || y > ymax ) {
        rgb[0] = 0.0;
        rgb[1] = 0.0;
        rgb[2] = 0.0;
        return 1;
    }

    int i = int( floor(x) );
    int j = int( floor(y) );
    assert( i >= 0 && i < xmax);
    assert( j >= 0 && j < ymax);

    double xi  = x - (double)i;
    double eta = y - (double)j;

    assert( xi  >= 0.0 && xi  < 1.0);
    assert( eta >= 0.0 && eta < 1.0);

    double weight[4];
    bilinear_weights( xi, eta, weight);

    double phi[4];
    for( int ic = 0; ic < 3; ic++) 
    {
         phi[0] = getChannelColor(img, i, j,   ic ); 
         phi[1] = getChannelColor(img, i+1, j,  ic ); 
         phi[2] = getChannelColor(img, i+1,j+1, ic ); 
         phi[3] = getChannelColor(img, i, j+1, ic ); 
         double sum = 0.0;
         for( int k = 0; k < 4; k++) 
              sum += phi[k]*weight[k];
         rgb[ic] = sum;
   }
   return 0;
}

////////////////////////////////////////////////////////////////////////////
IImage * cyl_forward_wrap(const IImage * img, double radius) 
{
    IImage *cylimg = NULL;
    int nrows = img->rows();
    int ncols = img->columns();
    double xi, yi, xc, yc, zc = 0.0, rgb[3];

    ofstream ofile( "cylimage.dat", ios::out);
    ofile << nrows << "  " << ncols << endl;

    for( int i = 0; i < nrows; i++) {
        for( int j = 0; j < ncols; j++) {
             xi = j - 0.5*ncols;
             yi = 0.5*nrows -i ;
             imgxy_to_cylxy( xi, yi, radius, xc, yc);
             getColor( img, j, i, rgb);
             ofile << xc     << " " << yc     << " " << zc << " " 
                   << rgb[0] << " " << rgb[1] << " " << rgb[2] << endl; 
        }
    }
    return cylimg;
}

////////////////////////////////////////////////////////////////////////////
IImage * cyl_backward_wrap(const IImage * img, double radius) 
{
    int nrows = img->rows();
    int ncols = img->columns();

    double xi, yi, xmin, ymin, xmax, ymax, xc, yc, zi = 0.0, rgb[3];
    xmin =  1.0E+10;
    xmax = -1.0E+10;
    ymin =  1.0E+10;
    ymax = -1.0E+10;
    for( int i = 0; i < nrows; i++) {
        for( int j = 0; j < ncols; j++) {
             xi = j - 0.5*ncols;
             yi = i - 0.5*nrows;
             imgxy_to_cylxy( xi, yi, radius, xc, yc);
             xmin = min(xc, xmin);
             xmax = max(xc, xmax);
             ymin = min(yc, ymin);
             ymax = max(yc, ymax);
        }
    }

    double dx = (xmax-xmin)/(double)(ncols-1);
    double dy = (ymax-ymin)/(double)(nrows-1);

    double xmid = 0.5*(xmin + xmax );
    double ymid = 0.5*(ymin + ymax );

    Color pixelColor(0.0, 0.0, 0.0);
    IImage *cylimg = new IImage(nrows, ncols, pixelColor);

	
    for( int i = 0; i < nrows; i++) {
        for( int j = 0; j < ncols; j++) {
             xc = xmin + j*dx; 
             yc = ymin + i*dy;
             cylxy_to_imgxy( xc, yc, radius, xi, yi);
             // Apply radial disctortion:

             bilinear_interpolate( img, xi, yi, rgb);
			 pixelColor.redQuantum( rgb[0] );
             pixelColor.greenQuantum( rgb[1] );
             pixelColor.blueQuantum( rgb[2] );
             cylimg->pixelColor( i, j, pixelColor);
			
        }
    }
	//img->write("image.png");
    //cylimg->write( "test.png");

    return cylimg;
}

////////////////////////////////////////////////////////////////////////////

IImage * cyl_wrap(const IImage * img, double f, double k1, double k2) 
{
    cout << " Warp Images " << endl;
    return cyl_backward_wrap(img, f);
}

