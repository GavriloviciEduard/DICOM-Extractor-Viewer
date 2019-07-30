#include "DICOMViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DICOMViewer w;
	w.show();
	return a.exec();
}
