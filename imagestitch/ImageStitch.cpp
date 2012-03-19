// IImageStitch.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "IImage.h"
#include "ImageStitch.h"
#include <algorithm>

using namespace std;
///////////////////////////////////////////////////////////////////////////////

double Point2F::operator[](const int i) const{
		return v[i];
}

double& Point2F::operator[](const int i){
		return v[i];
}

double Point3F::operator[](const int i) const{
		return v[i];
}

double& Point3F::operator[](const int i){
		return v[i];
}

void mk_dir(const string dir){
    string currdir = dir;

    DIR *dp;

    struct dirent *dirp;
    if ((dp = opendir(currdir.c_str())) == NULL) {
        //cout << "Error Cann't open directory " << currdir << endl;
        // make dir
	string cmd_mk_dir = "mkdir " + dir;
	system(cmd_mk_dir.c_str());
    }
}

void rm_dir(const string str){
   	 string cmd_rm_dir = "rm -rf " + str;
	 system(cmd_rm_dir.c_str());
}

int load_images(const string directory, vector<string> &files,
        vector<IImage*> &images) {
    string fname, currdir;

    images.clear();

    currdir = directory;

    DIR *dp;

    struct dirent *dirp;
    if ((dp = opendir(currdir.c_str())) == NULL) {
        cout << "Error Cann't open directory " << currdir << endl;
        return 1;
    }

    size_t pos;
    files.clear();
    IImage orgimage;
    while ((dirp = readdir(dp)) != NULL) {
        fname = dirp->d_name;
        pos = fname.find(".jpg");
        if (pos == string::npos) pos = fname.find(".jpeg");
        if (pos == string::npos) pos = fname.find(".JPG");
        if (pos == string::npos) pos = fname.find(".JPEG");
        if (pos != string::npos)
            files.push_back(currdir + "/" + fname);
        /*
                   string pgmfile = directory + "/" + fname.substr(0,pos) + ".pgm";
                }
         */
    }
    closedir(dp);

    sort(files.begin(), files.end());

    images.clear();
    for (int i = 0; i < files.size(); i++) {
        IImage *image = new IImage();
        cout<< " Read File " << files[i] << endl;
        image->read(files[i]);
		
        int nrows = image->rows();
        int ncols = image->columns();
		if(max(nrows, ncols)>640 && min(nrows, ncols)>480){

			if (nrows > ncols)
				image->resize(640, 480);
			else
				image->resize(480, 640);
		}
        //image->rotate(90.0);  ???????????????????????????????????????
        images.push_back(image);
    }
    cout << "Info: number of images loaded " << images.size() << endl;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int readSiftFile(const string &keyfile, vector<KeyFeature> &keyfeatures) {
    ifstream infile(keyfile.c_str(), ios::in);
    if (infile.fail()) {
        cout << "Warning: Cann't read feature file " << endl;
        return 0;
    }

    int numFeatures, numDescriptors;

    infile >> numFeatures >> numDescriptors;

    keyfeatures.resize(numFeatures);
    vector<unsigned char> descrip(numDescriptors);

    double xpos, ypos, scale, orient, val;
    for (int i = 0; i < numFeatures; i++) {
        infile >> ypos >> xpos >> scale >> orient;
        for (int j = 0; j < numDescriptors; j++) {
            infile >> val;
            assert(val >= 0 && val < 256);
            descrip[j] = (unsigned char) val;
        }
        keyfeatures[i].xpos = xpos;
        keyfeatures[i].ypos = ypos;
        keyfeatures[i].scale = scale;
        keyfeatures[i].orient = orient;
        keyfeatures[i].descriptors = descrip;
    }

	return 1;
}

///////////////////////////////////////////////////////////////////////////////

int closestNeighbour(const KeyFeature &srcKey, const vector<KeyFeature> &features) {
    int nSize = features.size();
    int nDesr = srcKey.descriptors.size();

    vector<double> dist2(nSize);

    for (int i = 0; i < nSize; i++) {
        double sum = 0.0;
        for (int j = 0; j < nDesr; j++)
            sum += (srcKey.descriptors[j] - features[i].descriptors[j]);
        dist2[i] = sum;
    }

    int index = 0;
    double mindist = dist2[0];

    for (int i = 0; i < nSize; i++) {
        if (dist2[i] < mindist) {
            mindist = dist2[i];
            index = i;
        }
    }
    return index;
}

////////////////////////////////////////////////////////////////////////////////

int DistSquared(const KeyFeature &k1, const KeyFeature &k2) {
    int i, dif, distsq = 0;

    for (i = 0; i < 128; i++) {
        dif = (int) k1.descriptors[i] - (int) k2.descriptors[i];
        distsq += dif * dif;
    }
    return distsq;
}

////////////////////////////////////////////////////////////////////////////////

int checkForMatch(KeyFeature &key, vector<KeyFeature> &klist) {
    int dsq, distsq1 = 100000000, distsq2 = 100000000;
    int minkey = 0;

    /* Find the two closest matches, and put their squared distances in
       distsq1 and distsq2.
     */
    for (int k = 0; k < klist.size(); k++) {
        dsq = DistSquared(key, klist[k]);

        if (dsq < distsq1) {
            distsq2 = distsq1;
            distsq1 = dsq;
            minkey = k;
        } else if (dsq < distsq2) {
            distsq2 = dsq;
        }
    }

    /* Check whether closest distance is less than 0.6 of second. */
    if (10 * 10 * distsq1 < 6 * 6 * distsq2)
        return minkey;
    return -1;
}

//////////////////////////////////////////////////////////////////////////

void matchFeatures(const string &k1, const string &k2,
        vector<PCorrespond> &pmatch) 
{

    vector<KeyFeature> k1features, k2features;

    readSiftFile(k1, k1features);
    readSiftFile(k2, k2features);

    pmatch.clear();

    ofstream ofile("match.dat", ios::out);

    ofile << "#Matching " << k1 << " " << k2 << endl;

    PCorrespond newpair;
    if (k1features.size() <= k2features.size()) {
        for (int i = 0; i < k1features.size(); i++) {
            int k = checkForMatch(k1features[i], k2features);
            if (k >= 0) {
                double x1 = k1features[i].xpos;
                double y1 = k1features[i].ypos;
                double x2 = k2features[k].xpos;
                double y2 = k2features[k].ypos;
                newpair.p1[0] = x1;
                newpair.p1[1] = y1;
                newpair.p2[0] = x2;
                newpair.p2[1] = y2;
                pmatch.push_back(newpair);
                ofile << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
            }
        }
        return;
    }

    if (k2features.size() <= k1features.size()) {
        for (int i = 0; i < k2features.size(); i++) {
            int k = checkForMatch(k2features[i], k1features);
            if (k >= 0) {
                double x1 = k1features[k].xpos;
                double y1 = k1features[k].ypos;
                double x2 = k2features[i].xpos;
                double y2 = k2features[i].ypos;
                newpair.p1[0] = x1;
                newpair.p1[1] = y1;
                newpair.p2[0] = x2;
                newpair.p2[1] = y2;
                pmatch.push_back(newpair);
                ofile << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
            }
        }
        return;
    }
}
///////////////////////////////////////////////////////////////////////////////
void feathering( IImage &imgA, IImage &imgB, IImage &blendIImage)
{
   int nrows = imgA.rows();
   int ncols = imgB.columns();

   assert( nrows == imgB.rows() );
   assert( ncols == imgB.columns() );

   Color pA, pB, pC, black(0, 0, 0);
   Quantum qA, qB, qC; // qC not used ???????????????????????????????
   double w;



   assert( nrows == blendIImage.rows() );
   assert( ncols == blendIImage.columns() );

   for( int i = 0; i < nrows; i++) {
	for( int j = 0; j < ncols; j++) {
        w  = 1.0*j/(double)(ncols-1);

        pA = imgA.pixelColor(i,j);
        pB = imgB.pixelColor(i,j);

        qA = pA.redQuantum();
        qB = pB.redQuantum();
        pC.redQuantum((1.0-w)*qA + w*qB);

        qA = pA.greenQuantum();
        qB = pB.greenQuantum();
        pC.greenQuantum((1.0-w)*qA + w*qB);

        qA = pA.blueQuantum();
        qB = pB.blueQuantum();
        pC.blueQuantum((1.0-w)*qA + w*qB);

        blendIImage.pixelColor(i,j, pC);
     }
    }
}

///////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[])
{
    if (argc != 3) {
        cout << "Usage: executable image_directory panorama_path" << endl;
        exit(0);
    }

    double cameraFocalLength = 682.05069;

    double K1 = -0.22892;
    double K2 = 0.27797;

    string directory = string(argv[1]);
    string panorama_path = string(argv[2]);
    //string siftcmd = "siftWin32.exe";
    string siftcmd = "./SIFT/sift ";
    string imgfile, execcmd, jpgfilename, pgmfilename, pngfilename;
    IImage jpgfile, pmgfile, pngfile;

    int numIImages = 4;

    vector<string> imagefiles, keyfiles;
    vector<IImage*> rawimages;
    ostringstream oss;
	
	
       //
       // Step I : Load all the images
       //          if necessary, resize to 680x480 image size.
       //
       load_images( directory, imagefiles, rawimages);

       // Generate Cylinderical warped images...
       numIImages = imagefiles.size();
       vector<IImage*>  warpimages;
       string warp_dir = directory + "/WarpImages";
       mk_dir(warp_dir);
       for( int i = 0; i < rawimages.size(); i++)  {
           IImage* img = cyl_wrap( rawimages[i], cameraFocalLength,
                                  K1, K2);
           warpimages.push_back(img);
           oss.str("");
           oss << warp_dir + "/warp_" << i << ".png";
           img->write( oss.str() );
		   
           oss.clear();
       }

       keyfiles.resize( numIImages );

       // Step II:  Calculate the SIFT features of all images.
       cout << " Calculating SIFT features ... " << endl;
       string png_dir = directory + "/PNGImages";
       mk_dir(png_dir);
       string key_dir = directory + "/Keys";
       mk_dir(key_dir);


       for( int i = 0; i < numIImages; i++) {
            oss.str("");
            oss << warp_dir + "/warp_" << i << ".png";
            pngfilename = oss.str();
            pngfile.read( pngfilename );
			
            oss.str("");
            oss << png_dir + "/warp_" << i << ".pgm";
            string pmgfilename = oss.str();
            pngfile.write( pmgfilename );

            oss.str("");
            oss <<key_dir + "/key_" << i << ".key";
            keyfiles[i] = oss.str();

            execcmd =  siftcmd + "<" + pmgfilename + ">" + keyfiles[i];
            system( execcmd.c_str() );
    }
	
    vector<PCorrespond> pmatch;
    Point2F trans;
    int minyshift = 1000000;
    int maxyshift = -minyshift;

    vector<int>  xpos(numIImages);
    vector<int>  ypos(numIImages);
    oss.str("");
    oss<<key_dir+"/imgpos.dat";
    string imgpos_path = oss.str();
    ofstream ofile(imgpos_path.c_str(), ios::out);
    xpos[0] = 0;
    ypos[0] = 0;
    ofile << xpos[0] << "  " << ypos[0] << endl;
	vector<int> outlier;
	for (int i = 0; i < numIImages; i++) {
			int j = (i+1) % numIImages;
			oss.str("");
			oss << key_dir + "/key_" << i << ".key";
			string k1 = oss.str();

			oss.str("");
			oss << key_dir + "/key_" << j << ".key";
			string k2 = oss.str();

			matchFeatures(k1, k2, pmatch);
			if(pmatch.size() > 1){
				ransac_translation(pmatch, trans);
				xpos[j] = xpos[i] + (int)(trans[0]);
				ypos[j] = ypos[i] + (int)(trans[1]);
				minyshift = min( minyshift, ypos[j] );
				maxyshift = max( maxyshift, ypos[j]);
				ofile << xpos[j] << "  " << ypos[j] << endl;
			}
			else{
				outlier.push_back(j);
			}
    }
    ofile.close();
	
	for(int i = 0; i <outlier.size(); i++)
		cout<<"no match "<<outlier[i]<<endl;
	
   ifstream ifile(imgpos_path.c_str(), ios::in);
   for( int i = 0; i <  numIImages; i++) {
	  
         ifile >> xpos[i] >> ypos[i];
         minyshift = min( minyshift, ypos[i]);
         maxyshift = max( maxyshift, ypos[i]);
   }
   
   IImage pngfile1, pngfile2;
   Color blankColor(0, 0, 0);
   
   string blend_dir = directory + "/BlendImages";
   mk_dir(blend_dir);
   // Do image blending:
   for( int ip = 0; ip < numIImages-1; ip++) {
        cout << " Blending "<<ip << endl;
        oss.str("");
        oss << warp_dir + "/warp_" << ip << ".png";
        pngfilename = oss.str();
        pngfile1.read( pngfilename );
        if( ip == 0) {
           oss.str("");
           oss << blend_dir + "/warp_" << ip << ".png";
           pngfile1.write( oss.str() );
        }
           
        int nrows = pngfile1.rows();
        int ncols = pngfile1.columns();
        int xt    = xpos[ip+1] - xpos[ip];
        int blendrows = nrows;
        int blendcols = ncols-xt;

        IImage imgA(blendrows, blendcols, blankColor );
         
        for( int i = 0; i < nrows; i++) {
          for( int j = 0; j < blendcols; j++) {
              Color pixelColor = pngfile1.pixelColor(i,j+xt);
              imgA.pixelColor(i, j, pixelColor);
          }
        }

        oss.str("");
        oss << warp_dir + "/warp_" << ip+1 << ".png";
        pngfilename = oss.str();
        pngfile2.read( pngfilename );

        IImage imgB(blendrows, blendcols, blankColor);
        for( int i = 0; i < nrows; i++) {
          for( int j = 0; j < blendcols; j++) {
              Color pixelColor = pngfile2.pixelColor(i,j);
              imgB.pixelColor(i, j, pixelColor);
          }
        }
		Color c(0, 0, 0);
		IImage imgC(imgA.rows(), imgB.columns(), c);
        feathering( imgA, imgB, imgC);

        for( int i = 0; i < nrows; i++) {
          for( int j = 0; j < blendcols; j++) {
              Color pixelColor = imgC.pixelColor(i,j);
              pngfile2.pixelColor(i, j, pixelColor);
          }
        }

        oss.str("");
        oss << blend_dir + "/warp_" << ip+1 << ".png";
        pngfile2.write( oss.str() );
   }

   int xsize = 600*numIImages;
   int ysize = 600;

    Color  pixelColor(0.0, 0.0, 0.0);
    IImage panimage(ysize, xsize, pixelColor);

    //int xt, yt; ??????????????????????????????????????????????

    for( int ip = 0; ip < numIImages; ip++) {
            oss.str("");
            oss << blend_dir + "/warp_" << ip << ".png";
			

            string pngfilename = oss.str();
            pngfile1.read( pngfilename );
			
            int nrows = pngfile1.rows();
            int ncols = pngfile1.columns();
            int xt= xpos[ip];
            int yt= ypos[ip] + abs(minyshift);
            for( int i = 0; i < nrows; i++) {
				for( int j = 0; j < ncols; j++) {
					Color c = pngfile1.pixelColor(i,j);
					
					panimage.pixelColor(i+yt, j+xt, c);
				}
            }
     }

     cout << " Write panormic images " << endl;
     panimage.write(panorama_path);

     // delete intermediate images
	rm_dir(warp_dir);
	rm_dir(png_dir);
	rm_dir(key_dir);
	rm_dir(blend_dir);
	system("rm match.dat");
	return 0;
}

