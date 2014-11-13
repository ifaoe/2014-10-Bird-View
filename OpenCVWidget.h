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


#ifndef OPENCVWIDGET_H_
#define OPENCVWIDGET_H_

#include <QObject>
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "ui_mainwindow.h"

class OpenCVWidget : public QWidget {
    Q_OBJECT
public:
    OpenCVWidget(QWidget *parent, Ui::MainWindow *mUi);
    virtual ~OpenCVWidget();

    QSize sizeHint() const { return qimage.size(); }
    QSize minimumSizeHint() const { return qimage.size(); }

public slots:
    void showImage(const cv::Mat& image);

protected:
    void paintEvent(QPaintEvent*);
    QImage qimage;
    cv::Mat tmp;
private:
    Ui::MainWindow * ui;
};

#endif /* OPENCVWIDGET_H_ */
