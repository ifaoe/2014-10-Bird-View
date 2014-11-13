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

#ifndef CVIMAGEPROVIDER_H_
#define CVIMAGEPROVIDER_H_

#include <tiffio.h>
#include <opencv2/core.hpp>
#include <cstring>

class CvImageProvider {
public:
	CvImageProvider(std::string file);
	virtual ~CvImageProvider();
	cv::Mat getImage();
	cv::Mat getSubImage(uint32 x, uint32 y, uint32 w, uint32 h);
private:
	std::string filename;
	TIFF *tif = 0;
    uint32 imageWidth, imageLength;
    uint32 tileWidth, tileLength;
};

#endif /* CVIMAGEPROVIDER_H_ */
