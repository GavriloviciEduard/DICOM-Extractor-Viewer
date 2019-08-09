#pragma once

#include <QObject>
#include "ui_CompareDialog.h"
#include <QtWidgets/qtablewidget.h>
#include "DcmWidgetElement.h"
#include "DcmWidgetElement.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dctagkey.h"

class CompareDialog : public QDialog
{
	Q_OBJECT;

	private:
		Ui::dialogCompare ui;
		DcmFileFormat file1;
		DcmFileFormat file2;
		std::vector<DcmWidgetElement> elements1;
		std::vector<DcmWidgetElement> elements2;
		std::vector<DcmWidgetElement> nestedElements;
		int depthRE = 0;
		int globalIndex = 0;
		bool firstFile = false;
		int loaded = 0;

	public:
		explicit CompareDialog(QDialog* parent);
		~CompareDialog() { }

		void loadFile(DcmFileFormat* file, QString fileName, bool first);
		void alertFailed(std::string message);
		void extractData(DcmFileFormat* file);
		void insertInTable(DcmElement* element, DcmFileFormat* file);
		void getNestedSequences(DcmTagKey tag, DcmSequenceOfItems* sequence, DcmFileFormat* file);
		void indent(DcmWidgetElement& element, int depth);
		void iterateItem(DcmItem *item, int& depth, DcmFileFormat* file);
		void insert(DcmWidgetElement element, int &index);
		void insertRight(DcmWidgetElement element, int &index);
		DcmWidgetElement createElement(DcmElement* element = nullptr, DcmSequenceOfItems* sequence = nullptr, DcmItem* item = nullptr);
		void insertBoth(DcmWidgetElement el1, DcmWidgetElement el2, int &index, int status);
		void insertSequence(int status, int& index1, int& index2);
		void merge();
		void clearTable();
		bool isDelimitation(DcmWidgetElement& el);

	private slots:
		void loadFile1();
		void loadFile2();
};