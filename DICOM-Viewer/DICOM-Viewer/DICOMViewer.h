#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DICOMViewer.h"
#include "dcmtk/dcmdata/dcfilefo.h"

class DICOMViewer : public QMainWindow
{
	Q_OBJECT

	public:
		DICOMViewer(QWidget *parent = Q_NULLPTR);

	private:
		Ui::DICOMViewerClass ui;
};
