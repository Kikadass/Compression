/*******************************************************
Use: Demo program to show the use of PPM loader 
     / saver class. Lightens an image by 20 values

Input : PPM Image (1.ppm)
Output: PPM Image (1-lighter.ppm)
*******************************************************/

#include <iostream>
#include "PPM.h"
using namespace std;


int main()
{
_ppm ppm;

if(ppm.load_ppm("Images\\1.ppm")==-1) //check for loading error
	{	
	cout<<"!!! Error while loading image"<<endl<<endl;
	return 1;
	}

cout<<"Image Details:"<<endl<<endl;
cout<<"Height: "<<ppm.get_image_height()<<endl;
cout<<"Width: "<<ppm.get_image_width()<<endl;
cout<<"Depth: "<<ppm.get_image_depth()<<endl<<endl;

cout<<"Changing pixel values..."<<endl;

int b,g,r; //define some pixel variables 

for(int x=0;x<ppm.get_image_width()-1;x++)
	{
	for(int y=0;y<ppm.get_image_height()-1;y++) //loop to lighten an image
		{
		r = ppm.get_pixel(x,y,RED);
		g = ppm.get_pixel(x,y,GREEN);
		b = ppm.get_pixel(x,y,BLUE);

		if(r == -1 || g == -1 || b == -1) //check for error codes
			cout<<"Error at pixel "<<(y*ppm.get_image_width()+x)*3<<endl; //get pixel position

		if(g<235 && r<235 && b<235) //check that an overflow can't happen
			{
			ppm.set_pixel(x,y,GREEN,g+20);
			ppm.set_pixel(x,y,BLUE,b+20); 
			ppm.set_pixel(x,y,RED,r+20);
			}
		}
	}

cout<<"Saving..."<<endl;

if(ppm.save_ppm("Images\\1-lighter.ppm")!=0) //check for saving errors
	cout<<"!!! Error while saving"<<endl;

cout<<"DONE!"<<endl<<endl;

return 0;
}