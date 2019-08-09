#pragma once

#include <QObject>
#include <qdialog.h>
#include <ui_EditDialogSimple.h>

class EditDialogSimple : public QDialog
{
	Q_OBJECT

	private:
		Ui::simpleEditDialog ui;
		bool valueModified = false;

	public:
		explicit EditDialogSimple(QDialog* parent);
		~EditDialogSimple() { }
		void setDescription(QString& description);
		void setValue(QString& value);
		QString getValue();

	private slots:
		void OKPressed();
		void cancelPressed();
	
};

