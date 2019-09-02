#pragma once

#include <QObject>
#include "ui_TagSelectDialog.h"
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/dcdicent.h>
#include "DcmWidgetElement.h"

class TagSelectDialog final : public QDialog
{
	Q_OBJECT;

	public:
		explicit TagSelectDialog(QDialog* parent);
		~TagSelectDialog();
		DcmWidgetElement getElement() const;
		void populate() const;
		
	private:
		Ui::tagSelectDialog ui{};
		DcmWidgetElement element;
		void clearTable() const;
		

	private slots:
		void okPressed();
		void cancelPressed();
		void findText() const;
};