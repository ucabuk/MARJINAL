#include <regex>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <opencv2/imgproc/imgproc_c.h>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <complex>
#include <sstream>
#include <iterator>
#include <boost/filesystem.hpp>
#include <thread>
#include <boost/thread.hpp>
#include <chrono>
#include <ctime>
#include <ratio>
#include <limits>
#define MAX_N 22000

using namespace cv;
using namespace std;
using namespace boost::filesystem;
using namespace std::chrono;

int nL, mL; // size of chaincode; line 49-71
vector <int> xone, yone; //line 217-226
string Aleft,Bright; //chaincode string; line 49-71
int dp[MAX_N][MAX_N]; line 49-71
Mat imgLaplacian; // globally output; line 148-211
Mat draw; //draw image from the chaincode without size; line 209
Mat draw2(600,600,CV_8UC3,Scalar(0)); //draw image from chaincode with const size. This changes in line 209.
string directory; //globally
int minimum(int a, int b, int c){
	if(a<=b&&a<=c)
		return a;
	if(b<=a&&b<=c)
		return b;
	if(c<=a&&c<=b)
		return c;
	return 0;
}
inline int levensthein_distance()
{
	for (int i=0;i<nL;i++) dp[i][0] = i;
	for (int i=0;i<mL;i++) dp[0][i] = i;
	for (int i=1;i<=nL;i++)
	{
		for (int j=1;j<=mL;j++)
		{
			if (Aleft[i-1]==Bright[j-1]){
				dp[i][j] = dp[i-1][j-1];
			}
			else{
				dp[i][j] = minimum
            			  (
					dp[i-1][j] + 1,
					dp[i][j-1] + 1,
    					dp[i-1][j-1] + 1
				);
			}
		}
	}
	return dp[nL][mL];
}
//Point-based matrix
Point next(const Point &p, uchar code){
	int o[8*2] = {0,-1, 1,-1, 1,0, 1,1, 0,1, -1,1, -1,0, -1,-1};
	return Point(p.x+o[code*2], p.y+o[code*2+1]);
}
void reconstruct(const vector<uchar> &_chain, vector<Point> &contours, Point offset) {
	contours.push_back(offset);
	for (size_t i=0; i<_chain.size(); i++) {
		Point p = next(offset, _chain[i]);
		contours.push_back(p);
		offset = p;
	}
}
//Freeman rotation
uchar encode(const Point &a, const Point &b) {
	uchar up    = (a.y > b.y);
	uchar left  = (a.x > b.x);
	uchar down  = (a.y < b.y);
	uchar right = (a.x < b.x);
	uchar equx  = (a.y == b.y);
	uchar equy  = (a.x == b.x);

	return (up    and equy)  ? 0 :  // the north
	(up    and right) ? 1 :  // the north east
	(right and equx)  ? 2 : // the east
	(down  and right) ? 3 : // the south eas
	(down  and equy)  ? 4 : // the south
	(left  and down)  ? 5 : // the south west
	(left  and equx)  ? 6 : // the west
	7 ;  // the north west
}
//counting countour and matching with freeman-chain-code
void cv_vector(vector<uchar> relative3, string filename){ // vector u
	ostringstream vts;
	if (!relative3.empty())
	{
		// Convert all but the last element to avoid a trailing ","
		copy(relative3.begin(), relative3.end(),
		     ostream_iterator<int>(vts));
		// Now add the last element with no delimiter
	}
	string converted = vts.str();
	std::ofstream relative_convert;
	relative_convert.open(filename,std::ios_base::app);
	relative_convert << converted << endl;
	relative_convert.close();
}
void chain_freeman(const vector<Point> &contours, vector<uchar> &_chain) {
	int i=0;
	for (size_t i; i<contours.size()-1; i++) {
		_chain.push_back(encode(contours[i],contours[i+1]));
	}
	_chain.push_back(encode(contours[i],contours[0]));
}
void image_processing(Mat source, const string xa, const string xb){
	Mat dst = source;
	Mat gray;
	cvtColor(dst,gray,CV_BGR2GRAY); //convert it to gray
	Mat binary;
	Mat se1 = getStructuringElement(MORPH_RECT, Size(1,1)); //matrices size for noisings in the image
	Mat se2 =  getStructuringElement(MORPH_RECT, Size(5,5)); //matrices size for noisings in the image
	Mat mask;
	morphologyEx(gray,binary,MORPH_CLOSE,se1); //gray to binary
	threshold(binary, mask, 200,255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU); //binary threshold. This was the diffucult part and the best selection.
	//here I created manual matrices to avoid noise in the image, because other matrices sizes was not suitable to fix it.
	Mat kernel = (Mat_<float>(3,3) <<
		      1, 1,1,
		      1,-8,1,
		      1, 1,1);
	morphologyEx(mask,mask,MORPH_DILATE,se1); //se1
	morphologyEx(mask,mask,MORPH_ERODE,kernel); //manual matrices;kernel
	morphologyEx(mask,mask,MORPH_CLOSE,kernel); //manual matrices;kernel
	morphologyEx(mask,mask,MORPH_OPEN,kernel); //manual matrices;kernel
	
	Mat sharp = mask;

   	filter2D(mask,imgLaplacian, CV_32F, kernel);
   	mask.convertTo(sharp, CV_32F);
   	Mat imgResult = sharp - imgLaplacian;
   	imgResult.convertTo(imgResult, CV_8UC1);
   	imgLaplacian.convertTo(imgLaplacian, CV_8UC3);

	vector<vector<Point>> contours;
	vector<vector<Point>> contours2(1); //Extra
	vector <uchar> chaincode;
	vector <uchar> relative;
	vector <int> shape;
	vector<Vec4i> hierarchy;

	int largest_area=0;
	int largest_contour_index=0;
	Mat canny;
	Canny(imgLaplacian, canny, 50,255,5);
	morphologyEx(imgLaplacian,imgLaplacian,MORPH_CLOSE,se1); //again, close noises in the image using se1 matrices.
	findContours(imgLaplacian,contours,hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE); //find and store all contours in the binary outline images using CV_CHAIN_APPROX_NONE (the other options are absent or does not work).
	double a = 0;

	for( size_t i = 0; i< contours.size(); i++ ) // iterate through each contour.
	{
		a=contourArea( contours[i],false); //  Find the area of contour
		if(a>largest_area)
		{
			largest_area=a;
			largest_contour_index=i;
		}
	}
	vector<vector<Point>> contourss;
	contourss.push_back(contours[largest_contour_index]); //store largest contour in the contourss
	for (size_t i=0; i<contourss.size();i++){
		chain_freeman(contourss[i],chaincode);
	}
	for (size_t i=0; i<chaincode.size(); i++){
		int relative1 = ((chaincode[i+1] - chaincode[i]) + 8) % 8;
		for (int j=0; j<relative1; j++){
		}
		relative.push_back(relative1);
	}
	int shape1;
	for (size_t i=0; i<chaincode.size(); i++){
		if(abs(chaincode[i+1] - chaincode[i])<=2){
			shape1=(chaincode[i+1] - chaincode[i]);
		}else if(chaincode[i+1] - chaincode[i]>=6){
			shape1=(chaincode[i+1] - chaincode[i])-8;
		}else if(chaincode[i+1] - chaincode[i]<=-6){
			shape1=(chaincode[i+1] - chaincode[i]) + 8;
		}else
			continue;
		for (int j=0; j<shape1; j++){
		}
		shape.push_back(shape1);
	}
	cv_vector(chaincode,xa);
	/// convert vector to string for freeman chain
	cv_vector(relative,xb);
	/// convert vector to string for relative chain
	
	Size size(imgLaplacian.cols+200,imgLaplacian.rows+200);
	resize(draw2,draw,size);
	
	reconstruct(chaincode,contours2[0],Point(imgLaplacian.cols/2,10));
	drawContours(draw, contours2, -1, Scalar(255, 255, 255), 1);
	
	//until this line, the images are drawn by the chain code.


	for(size_t i=0; i<relative.size(); i++){
		int z = relative[i];
		xone.push_back(z);
	}
	
	for(size_t j=0; j<relative.size(); j++){
		int c = relative[j];
		yone.push_back(c);
	}
}
string convertor_string(string file){
	stringstream line;
	std::ifstream data(file);
	line << data.rdbuf();
	return line.str();
}
void current_directory(){
	
	boost::filesystem::path full_path(boost::filesystem::current_path()); // the path you are working on.
	path p(full_path/directory);
	string result_directory = "results_of_" + directory; //the folder where is created the results.
	string output_directory = "output_image"; //the folder where is created the images.

	boost::filesystem::create_directory(full_path/directory/result_directory); //the folder where is created the results.
	boost::filesystem::create_directory(full_path/directory/result_directory/output_directory); //the folder where is created the images.
	path directoryy(full_path/directory);

	std::ofstream x(directory+"\\"+"name.txt"); //created txt file to keep file names into array.
	int sample_size; //for how many files in the dir
	
	string extension = ".bmp";
	string extension2 = ".jpg";
	string extension3 = ".png";//possible extensions
	
	for (auto i = directory_iterator(p); i != directory_iterator(); i++)
	{
		if (!is_directory(i->path()) && (i -> path().extension() == extension)) // the all extensions are considered.
		{
			x << i->path().filename().string() << endl;
			++sample_size;
		}
		else if(!is_directory(i->path()) && (i -> path().extension() == extension2)){
			x << i->path().filename().string() << endl;
			++sample_size;
		}
		else if(!is_directory(i->path()) && (i -> path().extension() == extension3)){
			x << i->path().filename().string() << endl;
			++sample_size;
		}
		else
			continue;
	}

	vector<string> array;
	array.resize(sample_size);
	std::ifstream y(directory+"\\"+"name.txt");

	for(int i = 0; i<sample_size; i++){
		std::getline(y,array[i],'\n');
	}

	string chain= directory+"\\"+result_directory+"\\"+"chain.txt";
	string relative=directory+"\\"+result_directory+"\\"+"relative.txt";
	string shape=directory+"\\"+result_directory+"\\"+"shape.txt";

	for(int i=0; i<sample_size; i++){
		Mat src = imread(directory+"\\"+array[i]);
		cout << array[i] << " is processing " << endl;
		
		image_processing(src,chain,relative);//?
		string o_image_name = directory+"\\"+result_directory+"\\"+output_directory+"\\"+array[i];
		cv::imwrite(o_image_name, draw);
	}

	std::ifstream openfile(chain);
	string token[sample_size];
	double distance_matrix[sample_size][sample_size];


	string length[sample_size];
	std::ofstream(l_chain);
	l_chain.open(directory+"\\"+result_directory+"\\"+"length_of_chain.txt", std::ios_base::app);
	int z[sample_size];
	
	for (size_t i = 0; i<sizeof(token)/sizeof(*token); i++){
		std::getline(openfile, token[i], '\n');
		length[i] = token[i];
		z[i] = length[i].size();
		
		l_chain << array[i] << ": ";
		l_chain << z[i] << endl;
	}

	double xy = 99999;
	int xyy[sample_size];
	double distance_matrix2[sample_size][sample_size];

	int result;
	float progress = 0.0;
	for( int i = 0; i<sample_size; i++ ){
		for( int j = i; j<sample_size; j++ ){
			Aleft = token[i];
			nL = Aleft.size();
			Bright = token[j];
			mL = Bright.size();
	    	//if(nL > (maxx - minn) && mL > (maxx - minn)){
	    		//result = levensthein_distance() / (maxx/minn);
	    	//}else
			result =  levensthein_distance();
			
			distance_matrix[i][j] = distance_matrix[j][i] = result;
		}
		cout << "[";
		int bar = 100;
		int pos = (bar * progress);
		for(int i = 0; i<bar; ++i){
			if((i*100) < pos) cout << "=";
			else if((i*100) == pos) cout << ">";
			else cout << " ";
		}
		cout << "] " << int(progress) << "%\r";
		cout.flush();
		progress +=100/sample_size;
    }
	std::ofstream offs(directory+"\\"+result_directory+"\\"+"old_distance.txt");
	offs << "\t" << sample_size;
	for ( int i = 0; i<sample_size; i++){
		offs << array[i] << "\t";
	}
	offs << endl;
	for ( int i = 0; i<sample_size; i++){
		offs << array[i] << "  ";
		for ( int j = 0; j<sample_size; j++){
			offs << "\t" << distance_matrix[i][j];
		}
		offs << endl;
	}

	for( int i = 0; i<sample_size; i++ ){
		for( int j = 0; j<sample_size; j++ ){
			if(xy > distance_matrix[i][j] && distance_matrix[i][j] != 0){
				xy = distance_matrix[i][j];
			}
			xyy[i] = xy;
			xy = 99999;
		}
	}

	for( int i = 0; i<sample_size; i++ ){
		for( int j = 0; j<sample_size; j++ ){
			distance_matrix2[i][j] = (double) distance_matrix[i][j] / ((z[i] + z[j]) + (abs(z[i] - z[j])));
		}
	}

	std::vector<std::vector<double >> matrix; //convert distance matrices into vector format
	matrix.resize(sample_size, vector<double >(sample_size));

	for(int i = 0; i<matrix.size(); i++){
		for(int j = 0; j<matrix.size(); j++){
			matrix[i][j] = distance_matrix2[i][j];
		}
	}


	cout << "All The Processes have been done ! " << endl;


	std::ofstream of(directory+"\\"+result_directory+"\\"+"distance.txt");
	of << "\t" << sample_size;
	for ( int i = 0; i<sample_size; i++){
		of << array[i] << "\t";
	}
	of << endl;
	for ( int i = 0; i<sample_size; i++){
		of << array[i] << "  ";
		for ( int j = 0; j<sample_size; j++){
			of << "\t" << distance_matrix2[i][j];
		}
		of << endl;
	}
    	    
	std::vector<std::vector<double >> c_matrix;
	c_matrix = matrix;
	int den = 999999;
	vector<int> count;
	count.resize(matrix.size());
	for(int i = 0; i<matrix.size(); i++) count[i] = 1;

	do
	{
		long double xy1 = 99999;
		int row1 = 0;
		int col1 = 0;
		int temp = 0;
		for(int i = 0; i<matrix.size(); i++){
			for(int j = 0; j<matrix.size(); j++){
				if(matrix[i][j] < xy1 && matrix[i][j] != 0){
					xy1 = matrix[i][j];
					row1 = i;
					col1 = j;
				}
			}
		}

    	    	//Second index is erased after added using insert,
    	    	//rest sequence, new one is added using push_back.
    	    	//

		matrix.erase(matrix.begin() + col1); //related rows and columns are erased. 
		for(int i = 0; i<matrix.size(); i++) matrix[i].erase(matrix[i].begin() + col1);
		string leftparent = "(";
		string comma = ",";
		string rightparent = ")";
		
		array[row1] = leftparent + array[row1] + comma + array[col1] + rightparent;
		array.erase(array.begin() + col1);

		std::vector <double > template_row;
		template_row.resize(c_matrix.size());
    	    	std::vector <double > template_col;
    	    	template_col.resize(c_matrix.size());
    	    	std::vector <double> total;
    	    	total.resize(c_matrix.size());
    	    	 //it is copied into matrices, then cleaned.



		for(size_t k = 0; k < c_matrix.size(); k++){
			template_row[k] = (c_matrix[row1][k]);
			template_col[k] = (c_matrix[k][col1]);
			if((template_row[k] != 0 && template_col[k] != xy1) ||  (template_row[k] != xy1 && template_col[k] != 0) ){
				total[k] = ((count[row1]*template_row[k]) + (count[col1]*template_col[k])) / (count[row1] + count[col1]) ;
			}
		}

		total.erase(total.begin() + col1 );
		for(size_t i = 0; i<matrix.size(); i++){
			for(size_t j = 0; j<matrix.size(); j++){
				if(matrix[row1][j] != 0 || matrix[j][row1] != 0){
					matrix[row1][j] = total[j];
					matrix[j][row1] = total[j];
					//As the minimum is added in in the previous matrice, [j][j] is different with 0
				}
				/*else if(matrix[i][j] != 0 and matrix[j][i] != 0){
				matrix[i][col1] = (c_matrix[row1][j] + c_matrix[i][col1]) / 2;
				matrix[row1][j] = (c_matrix[j][row1] + c_matrix[col1][j]) / 2;
				matrix[j][j] = 0; //As the minimum is added in in the previous matrice, [j][j] is different with 0
				}*/
				else
					matrix[i][j] = 0;
				matrix[j][j] = 0;
			}
		}

		count[row1] = count[row1] + count[col1];
		count.erase(count.begin() + col1);

		c_matrix.clear();
		c_matrix = matrix;
	}
	while(matrix.size() > 1);
	
	//jstree file is provided by Yasin, and will be opened and added the result in.
	string UPGMA = directory+"\\"+"jstree.html";
	string UPGMA_1 = directory+"\\"+"jstree1.html";
	std::ofstream ofp;
	std::ifstream ofp1;
	ofp1.open(UPGMA);
	ofp.open(UPGMA_1);


	string line;
	string pattern = "<p><b>Newick Tree:</b></p>";
	string pattern2 = "<textarea id=\"nhx-ex\" style=\"display: none\">";
	size_t pos;
	size_t pos2;
	size_t pos3;
	size_t pos4;
	
	while(getline(ofp1,line))
	{
		while(true)
		{
			pos = line.find(pattern);
			pos3 = line.find(pattern2);
			if(pos != string::npos){
				ofp << line << endl;
				getline(ofp1,line);
				ofp << line << endl;
				getline(ofp1,line);

				size_t len = line.length();
				pos2 = line.find(line);
				if(pos2 != string::npos){
					line.replace(line.begin(), line.end(),array[0]);
					line.append(";");
				}
			}
			if(pos3 != string::npos){
				ofp << line << endl;
				getline(ofp1,line);
				size_t len2 = line.length();
				pos4 = line.find(line);
				if(pos4 != string::npos){
					line.replace(line.begin(), line.end(), array[0]);
					line.append(";");
				}
			}
			else
				break;
		}
		ofp << line << '\n';
	}
}
int main(int argc, char** argv){
	
	cout << "Enter the name of the folder containing the images: (e.g.: Images)" << endl;
	cin >> directory;
	
	string result_directory = "results_of_"+directory;


	std::ofstream relative;
	std::ofstream chain;
	std::ofstream l_chain;

	l_chain.open(directory+"\\"+result_directory+"\\"+"length_of_chain.txt", ios::out | ios:: trunc); //the content of created files are cleaned.
	relative.open(directory+"\\"+result_directory+"\\"+"relative.txt", ios::out | ios::trunc);
	chain.open(directory+"\\"+result_directory+"\\"+"chain.txt", ios::out | ios::trunc);

	string UPGMA = directory+"\\"+"jstree.html";
	string UPGMA_1 = directory+"\\"+"jstree1.html";

	current_directory();
	boost::filesystem::rename(UPGMA_1,UPGMA);
	return 0;
}
