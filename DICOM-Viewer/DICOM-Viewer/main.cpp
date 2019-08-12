#include "DICOMViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon("./rsc/pxd_app_icon.png"));
	DICOMViewer w;
	w.show();
	return a.exec();
}
