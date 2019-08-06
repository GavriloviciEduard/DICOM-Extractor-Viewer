#pragma once
#include <QObject>
#include "ui_TagSelectDialog.h"
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/dcdicent.h>
#include "DcmWidgetElement.h"

class TagSelectDialog : public QDialog
{
	Q_OBJECT;
private:
	Ui::tagSelectDialog ui;
	DcmWidgetElement element;

public:
	explicit TagSelectDialog(QDialog* parent);
	~TagSelectDialog();

	DcmWidgetElement getElement();
	void clearTable();

	void populate();
private slots:
	void okPressed();
	void cancelPressed();
	void findText();
};