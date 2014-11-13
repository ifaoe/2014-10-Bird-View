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

#include "OpenCVWidget.h"
#include <assert.h>
#include <iostream>

OpenCVWidget::OpenCVWidget(QWidget *parent = 0, Ui::MainWindow *mUi = 0) : QWidget(parent), ui(mUi) {
	// TODO Auto-generated constructor stub

}

OpenCVWidget::~OpenCVWidget() {
	// TODO Auto-generated destructor stub
}

void OpenCVWidget::showImage(const cv::Mat& image) {
    // Von BGR nach RGB konvertieren.
	image.convertTo(tmp,0,1./256,0);
	// QImage needs the data to be stored continuously in memory
    assert(tmp.isContinuous());
    // Assign OpenCV's image buffer to the QImage. Note that the bytesPerLine parameter
    // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width because each pixel
    // has three bytes.
    qimage = QImage(tmp.data, tmp.cols, tmp.rows, tmp.cols*3, QImage::Format_RGB888);

    this->setFixedSize(image.cols, image.rows);

    repaint();
}

void OpenCVWidget::paintEvent(QPaintEvent* /*event*/) {
    // Display the image
    QPainter painter(this);
    painter.drawImage(QPoint(0,0), qimage);
    painter.end();
}
