#include "stdafx.h"
#include "IImage.h"

using namespace corona;

IImage::IImage(void)
{
	_nrows = 0;
	_ncols = 0;
	red = NULL;
	green = NULL;
	blue = NULL;
	alpha = NULL;
}


IImage::~IImage(void)
{
	if(red!=NULL) free(red);
	if(green!=NULL) free(green);
	if(blue!=NULL) free(blue);
	if(alpha!=NULL) free(alpha);
}



IImage::IImage(int height, int width, Color c){
	_nrows = height;
	_ncols = width;
	int npixels = _ncols * _nrows;
	
	red = (Byte*)malloc(sizeof(Byte)*npixels);
	green = (Byte*)malloc(sizeof(Byte)*npixels);
	blue = (Byte*)malloc(sizeof(Byte)*npixels);
	alpha = (Byte*)malloc(sizeof(Byte)*npixels);
	for (size_t i = 0; i < npixels; ++i) {
		red[i]   = c.redQuantum();
		green[i] = c.greenQuantum();
		blue[i]  = c.blueQuantum();
		alpha[i] = 0;
	}
}
	
int IImage::rows() const{
	return _nrows;
}
    
int IImage::columns() const{
	return _ncols;
}
	
Color IImage::pixelColor(int i , int j) const{
	Color c(red[i*_ncols+j], green[i*_ncols+j], blue[i*_ncols+j], QuantumRange);
	
	return c;
}
	
void IImage::pixelColor(int i , int j, Color c){
	red[i*_ncols+j] = (Byte)c.redQuantum();
	green[i*_ncols+j] = (Byte)c.greenQuantum();
	blue[i*_ncols+j] = (Byte)c.blueQuantum();
	if(red[i*_ncols+j]+green[i*_ncols+j]+blue[i*_ncols+j] > 1E-5)
		alpha[i*_ncols+j] = QuantumRange;
	else
		alpha[i*_ncols+j] = 0;
}
	
void IImage::read(string filename){
	string ext = filename.substr(filename.find_last_of(".")+1, 3);
	corona::Image* image;
	bool hasAlpha = false;
	if(ext == "jpg")
		image = corona::OpenImage(filename.c_str(), corona::PF_DONTCARE, corona::FF_AUTODETECT);
	else if(ext == "png"){
		image = corona::OpenImage(filename.c_str(), corona::FF_PNG, corona::PF_R8G8B8A8);
		hasAlpha = true;
	}
	else if(ext == "pgm"){
		ifstream pgmFile;
		pgmFile.open (filename.c_str(), ios::in | ios::binary);
		
		string type, comment, size, qrange;
		getline(pgmFile, type);
		getline(pgmFile, comment);
		if(comment[0] != '#')
			size = comment;
		else
			getline(pgmFile, size);
		
		getline(pgmFile, qrange);
		
		size_t len = pgmFile.tellg();
		Byte* memblock = new Byte[len];
		pgmFile.seekg(0, ios::beg);
		pgmFile.read((char*)memblock, len);
		pgmFile.close();

		delete[] memblock;
	}else
		return;

	void* pixels = image->getPixels();
	

	_nrows = image->getHeight();
	_ncols = image->getWidth();
	Byte* p = (Byte*)pixels;
	int npixels = _ncols * _nrows;
	if(red!=NULL) free(red);
	if(green!=NULL) free(green);
	if(blue!=NULL) free(blue);
	if(alpha!=NULL) free(alpha);
	red = (Byte*)malloc(sizeof(Byte)*npixels);
	green = (Byte*)malloc(sizeof(Byte)*npixels);
	blue = (Byte*)malloc(sizeof(Byte)*npixels);
	alpha = (Byte*)malloc(sizeof(Byte)*npixels);
	for (size_t i = 0; i < npixels; ++i) {
		red[i]   = *p++;
		green[i] = *p++;
		blue[i]  = *p++;
		if(hasAlpha)
			alpha[i] = *p++;
		else
			alpha[i] = QuantumRange;
	}
}
	
void IImage::resize(int newHeight, int newWidth){
	if(red == NULL) 
		return;
    //
    // Get a new buuffer to interpolate into
	if(newHeight != _nrows || newWidth != _ncols){
		Byte* _red = new unsigned char [_nrows * _ncols];
		Byte* _blue = new unsigned char [_nrows * _ncols];
		Byte* _green = new unsigned char [_nrows * _ncols];
		Byte* _alpha = new unsigned char [_nrows * _ncols];

        double scaleWidth =  (double)newHeight / (double)_ncols;
        double scaleHeight = (double)newHeight / (double)_nrows;

        for(int cy = 0; cy < newHeight; cy++)
        {
            for(int cx = 0; cx < newWidth; cx++)
            {
                int pixel = cy * newWidth + cx;
                int nearestMatch =  ((int)(cy / scaleHeight) * _ncols) + (int)(cx / scaleWidth);
                
                _red[pixel] =  red[nearestMatch];
                _blue[pixel] =  blue[nearestMatch];
                _green[pixel] =  green[nearestMatch];
				_alpha[pixel] =  alpha[nearestMatch];
            }
        }

        //
        free(red);
		free(green);
		free(blue);
		free(alpha);
		red = _red;
		green = _green;
		blue = _blue;
		alpha = _alpha;

        _ncols = newWidth;
        _nrows = newHeight; 
	}
}
	
void IImage::rotate(double degree){
	
}
	
void IImage::write(string filename) const{
	string ext = filename.substr(filename.find_last_of(".")+1, 3);
	bool hasAlpha = false;
	if(ext=="png")
		hasAlpha = true;

	int npixels = _ncols * _nrows;
	Byte* pixels = (Byte*)malloc(sizeof(Byte)*npixels*4);
	Byte* p = (Byte*)pixels;
	for (size_t i = 0; i < npixels; ++i) {
		*p++ = red[i] ;
		*p++ = green[i] ;
		*p++ = blue[i];
		if(hasAlpha)
			*p++ = alpha[i];
	}
	
	
	corona::Image* image;
	if(ext == "jpg"){
		image = corona::CreateImage(_ncols, _nrows, corona::PF_R8G8B8, pixels);
		corona::SaveImage(filename.c_str(), corona::FF_JPEG, image);
	}
	else if(ext == "pgm"){
		//ofstream pgmFile;
		//pgmFile.open (filename.c_str(), ios::out);
		//pgmFile<<"P5"<<"\n"<<_ncols<<"\n"<<_nrows<<"\n"<<"255"<<"\n";
		
		FILE* fp;
		fp = fopen(filename.c_str(),"wb");
		fprintf(fp, "P5\n%d\n%d\n255\n", _ncols, _nrows);
		
		Byte* p = (Byte*)calloc(_nrows*_ncols, sizeof(Byte));
		
		for (int r = 0; r < _nrows; r++){
			for (int c = 0; c < _ncols; c++){
				int ind = r*_ncols + c;
				int val = (int) (/*255.0 */ red[ind] / 3.0);
				//fputc(max(0, min(255, val)), fp);
				p[ind] = (Byte)max(0, min((int)QuantumRange, val));
				//pgmFile<<(Byte)max(0, min((int)QuantumRange, val));
			}
		}
		//pgmFile.write((char*)p, _nrows*_ncols);
		//pgmFile.close();
		fwrite((void*)p, sizeof(Byte), _nrows*_ncols, fp);
		free(p);
		fclose(fp);
	}
	else{
		image = corona::CreateImage(_ncols, _nrows, corona::PF_R8G8B8A8, pixels);
		corona::SaveImage(filename.c_str(), corona::FF_PNG, image);
	}
	
}
