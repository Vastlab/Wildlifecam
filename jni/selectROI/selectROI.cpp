#include "selectROI.h"

using namespace cv;
using namespace std;

int drag = 0;
int done = 0;
int key;

cv::Mat inputImage;
cv::Mat img1;

CvRect rect;
CvPoint point;
CvRect rectOut;
void mouseHandler(int event, int x, int y, int flags, void* param)
{
  /* user press left button */
  if (event == CV_EVENT_LBUTTONDOWN && !drag && !done)
    {
      point = cvPoint(x, y);
      drag = 1;
    }
  /* user drag the mouse */
  if (event == CV_EVENT_MOUSEMOVE && drag)
    {
      img1 = inputImage.clone();
      rectangle(img1,point,cvPoint(x, y),CV_RGB(255, 0, 0),3);
      imshow("input", img1);
    }
  /* user release left button */
  if (event == CV_EVENT_LBUTTONUP && drag)
    {

      if (point.x < 0)
	point.x = 0;

      if (point.y < 0)
	point.y = 0;

      if (x > img1.cols)
	x  = img1.cols;

      if (y > img1.rows)
        y = img1.rows;

      rect = cvRect(point.x,point.y,x-point.x,y-point.y);

      done = 1;
      drag = 0;
    }

  /* user double click left button */
  if (event == CV_EVENT_LBUTTONDBLCLK)
    {
      cv::Mat result(inputImage,rect);
      cvNamedWindow("result",CV_WINDOW_NORMAL);

	rectOut.x = rect.x;
	rectOut.y = rect.y;
	rectOut.width = rect.width;
	rectOut.height = rect.height;

      imshow("result", result);

    }

  /* user click right button: reset all */
  if (event == CV_EVENT_RBUTTONDOWN)
    {
      cvDestroyWindow("result");
      img1 = inputImage.clone();
      imshow("input", img1);
      done = 0;
    }
}


int main(int argc, char** argv)
{
  int good_read = 0;
  string answer;
  string output_file = "roiCoords.txt";

  while (good_read == 0)
    {
      cout << "Enter the image file name: " << endl;
      cin  >> answer;

      try 
	{
	  inputImage = imread(answer.c_str());

	  if (inputImage.data) 
	    good_read = 1;
	} 
      catch(std::exception e) 
	{
	  cout << "Exception: [" << e.what() << "]" << endl;
	}
    }

  cvNamedWindow("input",CV_WINDOW_NORMAL);
  cv::imshow("input",inputImage);
 
  while( key != 'q')
    {
      cvSetMouseCallback("input", mouseHandler, NULL);
      key = cvWaitKey(10) & 0Xff;
    }

  //open the roiFile and print the coords
  std::ofstream outFile;
  outFile.open(output_file.c_str());
  outFile << rectOut.x << std::endl;
  outFile << rectOut.y << std::endl;
  outFile << rectOut.width << std::endl;
  outFile << rectOut.height << std::endl;

  outFile.close();

  return 0;
}
