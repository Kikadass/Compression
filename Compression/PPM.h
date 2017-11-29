#include <stdio.h>
#include <string.h>

#define RED   0
#define GREEN 1
#define BLUE  2

class _ppm{
public:
	_ppm();
	~_ppm();

	int load_ppm(const char* filename, int xx = 0, int yy = 0);
	int save_ppm(const char* filename);

	int get_image_height(){return height;};
	int get_image_width(){return width;};
	int get_image_depth(){return depth;};

	int get_pixel(int x, int y, int channel);
	int set_pixel(int x, int y, int channel, int value);

protected:

	void clear();

	int* data;
	int width,height,depth;

private:

	bool loaded;
};