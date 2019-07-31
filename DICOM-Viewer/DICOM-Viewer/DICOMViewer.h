#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include "ui_DICOMViewer.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcvr.h"
#include "dcmtk/dcmdata/dctag.h"

#include "DcmWidgetElement.h"
#include <algorithm>

class DICOMViewer : public QMainWindow
{
	Q_OBJECT

	public:
		DICOMViewer(QWidget *parent = Q_NULLPTR);

	private:
		Ui::DICOMViewerClass ui;
		DcmFileFormat file;
		std::vector<DcmWidgetElement> elements;

	protected:
		void insertInTable(DcmElement* element, int index);
		void extractData(DcmFileFormat file);
		void repopulate(std::vector<DcmWidgetElement> source);
		void getNestedSequences(DcmTagKey tag);
		void iterateItem(DcmItem *item);
	
	private slots:
		void fileTriggered(QAction* qaction);
		void closeButtonClicked();
		void findText();
};
