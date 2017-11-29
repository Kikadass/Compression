#include "PPM.h"

_ppm::_ppm()
{
loaded = false;
}

_ppm::~_ppm()
{
if(loaded)
	clear();
}

/* save_ppm - Saves the image as PPM format to the file stream.
 * Arguments: filename to save into, x1, y1 to x2, y2 area to save.
 * Returns: 0 on success, -1 on error
 */

int _ppm::save_ppm(const char* filename)
{
  if(!loaded)
	  return -1;

  FILE * fi;
  int x, y, rgb;

  int x1 = 0;
  int y1 = 0;
  int x2 = width;
  int y2 = height;

  errno_t err;
  err = fopen_s(&fi, filename, "w+b");
  if(err==0)
	  return -2;
  
  if (x1 < 0 || x1 > width || y1 < 0 || y1 > height ||
      x2 < 0 || x2 > width || y2 < 0 || y2 > height)
		return -1;
  
  
  /* Dump PPM header: */
  
  fprintf(fi, "P6\n %d %d\n%d\n", x2 - x1, y2 - y1, depth);
  

  /* Dump data: */
  
  for (y = y1; y < y2; y++)
    for (x = x1; x < x2; x++)
      for (rgb = 0; rgb < 3; rgb++)
			fputc(data[((y*width+x)*3)+rgb], fi);
  
  return 0;
}

/* load_ppm - Loads a PPM format image.
 * Arguments: filename to load from,x and y locations to start 
 * loading into, and opacity with which to add the data to the 
 * existing image (default is 255 - full opacity).
 * Returns: 0 on success, -1 on error
 */

int _ppm::load_ppm(const char* filename, int xx, int yy)
{
  if(loaded)
	  clear();

  FILE * fi;
  char temp[10240];
  int x, y, rgb, m; 
  errno_t err;
  err = fopen_s(&fi, filename, "rb");
  if (err == 0)
	  return -1;
  
  /* Load PPM header: */
  
  fscanf_s(fi, "%s %d %d %d", &temp, &width, &height, &m); 
  depth = m;

  /* Create storage array for data */

  data = new int[(width*height)*3];

  /* Get some data from the ppm */

    fgetc(fi);

  /* Load real data: */

  for (y = 0; y < height; y++)
	{
	for (x = 0; x < width; x++)
		{
		for (rgb = 0; rgb < 3; rgb++)
			{
			data[((y*width+x)*3)+rgb] = fgetc(fi);
			}
		}
	}

  loaded = true;
  return 1;
}

int _ppm::set_pixel(int x, int y, int channel, int value)
{
if(!loaded)
	return -1;

if (x < 0 || x > width || y < 0 || y > height)
	return -1;

if(channel > 2 || channel < 0)
	return -1;

if(value < 0 || value > 255)
	return -1;

data[(((y*width)+x)*3)+channel] = value;

return 0;
}

int _ppm::get_pixel(int x, int y, int channel)
{
if(!loaded)
	return -1;

if (x < 0 || x > width || y < 0 || y > height)
	return -1;

return data[(((y*width)+x)*3)+channel];
}

void _ppm::clear()
{
delete[] data;
height = 0;
width = 0;
depth = 0;
loaded = false;
}