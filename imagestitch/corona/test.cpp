#include <iostream>
#include <corona.h>

using namespace corona;

//However, this document assumes you don't want to clutter the global
//namespace, and thus will prefix Corona symbols with corona::.

//The most basic way to load an image is as follows:

int main(){
	
	corona::Image* image = corona::OpenImage("image.bmp");
	corona::Image* image2 = corona::OpenImage("image.png");
	corona::Image* image3 = corona::OpenImage("image.jpg");
	if (!image && !image2 && !image3) {
		std::cout<<"error";  
	}

	int width2 = image2->getWidth();
	int height2 = image2->getHeight();
	int width3 = image2->getWidth();
	int height3 = image2->getHeight();
	
	
	std::cout<<width2<<" "<<height2<<std::endl;
	std::cout<<width3<<" "<<height3<<std::endl;
	
	image->write("output.png");	

	corona::PixelFormat format = image->getFormat();
	void* pixels = image->getPixels();
	switch (format){
 		case corona::PF_R8G8B8A8: break; // process image data
 		case corona::PF_R8G8B8:   break; // process image data
	}
	
	delete image;
}
