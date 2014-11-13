//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CvImageProvider.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

CvImageProvider::CvImageProvider(std::string file) {
	filename = file;
	TIFF *tif = TIFFOpen(file.c_str(),"r");
    if (tif) {
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imageWidth);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imageLength);
        TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tileWidth);
        TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileLength);
    }
}

CvImageProvider::~CvImageProvider() {
	// TODO Auto-generated destructor stub
//	TIFFClose(tif);
}

cv::Mat CvImageProvider::getImage() {
	cv::Mat img(imageWidth, imageLength, CV_16UC3);
	img = cv::imread(filename);

	return img;
}

cv::Mat CvImageProvider::getSubImage(uint32 x, uint32 y, uint32 w, uint32 h) {
	w = 2*(w/2);
	h = 2*(h/2);
//	cv::Mat img(w, h, CV_16UC3);
	tdata_t buf;
    if (tif) {
        buf = _TIFFmalloc(TIFFTileSize(tif));
        uint32 i,j;
        for (i = 0;  (i < imageLength); i += tileLength)
            for (j = 0;  (j < imageWidth); j += tileWidth)
                TIFFReadTile(tif, buf, i, j, 2, 0);

//        _TIFFfree(buf);
    }
    cv::Mat img(imageLength,imageWidth,CV_16UC3, buf);
	return img;
}
