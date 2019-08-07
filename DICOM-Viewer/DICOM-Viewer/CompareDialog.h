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

	std::vector<DcmWidgetElement> table1Elements;
	std::vector<DcmWidgetElement> table2Elements;
	std::vector<DcmWidgetElement> nestedElements;

	int globalIndex;

public:
	explicit CompareDialog(QDialog* parent);
	~CompareDialog();

	void loadFile(DcmFileFormat file, QTableWidget* table);
	void extractData(DcmFileFormat file, QTableWidget* table);
	void alertFailed(std::string message);

	void insertInTable(QTableWidget* table, DcmElement* element);
	void getNestedSequences(DcmTagKey tag);
	void iterateItem(DcmItem* item, int &depth);

private slots:
	void loadFile1();
	void loadFile2();
};