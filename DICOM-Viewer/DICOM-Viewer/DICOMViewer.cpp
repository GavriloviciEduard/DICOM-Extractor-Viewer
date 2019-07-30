#include "DICOMViewer.h"

DICOMViewer::DICOMViewer(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	DcmFileFormat test;
}

void DICOMViewer::fileHovered(QAction * qaction)
{

}

void DICOMViewer::closeButtonClicked()
{
	this->close();
}

void DICOMViewer::fileTriggered(QAction* qaction)
{
	QString option = qaction->text();
	if (option == "Open")
	{
		QString fileName = QFileDialog::getOpenFileName(this);
	}
	else
	{

	}
}