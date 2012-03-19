#pragma once
#include <string> 
#include <fstream>
#include "corona.h"


using namespace std;
using namespace corona;

typedef double Quantum;
typedef double Ratio;
typedef unsigned char Byte;

class Color{
public:
	Quantum qrange;
	Ratio r;
	Ratio g;
	Ratio b;
	Ratio alpha; 
	Color(){
		this->r = 0;
		this->b = 0;
		this->g = 0;
		this->alpha = 0;
		this->qrange = QuantumRange;
	}

	Color(Quantum r, Quantum g, Quantum b, Quantum qrange){
		this->qrange = qrange;
		this->r = r/ qrange;
		this->b = b/ qrange;
		this->g = g/ qrange;
		this->alpha = 0;
	}

	Color(Ratio r, Ratio g, Ratio b){
		this->r = r;
		this->b = b;
		this->g = g;
		this->alpha = 0;
		this->qrange = QuantumRange;
	}
   
	void redQuantum(Quantum r){
		this->r = r / qrange;
	}
	void greenQuantum(Quantum g){
		this->g = g / qrange;
	}
	void blueQuantum(Quantum b){
		this->b = b / qrange;
	}

	Quantum redQuantum(){
		return r*qrange;
	}
	Quantum greenQuantum(){
		return g*qrange;
	}
	Quantum blueQuantum(){
		return b*qrange;
	}

	void red(Ratio r){
		this->r = r;
	}
	void green(Ratio g){
		this->g = g;
	}
	void blue(Ratio b){
		this->b = b;
	}
};


class IImage
{
private:
	//corona::Image* image;
	int _nrows;
	int _ncols;
	Byte* red;
	Byte* green;
	Byte* blue;
	Byte* alpha;

public:
	IImage(void);
	IImage(int height, int width, Color c);
	~IImage(void);
	int rows() const;
    int columns() const;
	Color pixelColor(int i , int j) const;
	void pixelColor(int i , int j, Color c);
	void read(string filename); // currently only supports jpeg and png image
	void resize(int height, int width);
	void rotate(double degree);
	void write(string filename) const;
};

