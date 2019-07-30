#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qfiledialog.h>
#include "ui_DICOMViewer.h"
#include "dcmtk/dcmdata/dcfilefo.h"

class DICOMViewer : public QMainWindow
{
	Q_OBJECT

	public:
		DICOMViewer(QWidget *parent = Q_NULLPTR);

	private:
		Ui::DICOMViewerClass ui;
	
	private slots:
		void fileTriggered(QAction* qaction);
		void fileHovered(QAction* qaction);
		void closeButtonClicked();
};
