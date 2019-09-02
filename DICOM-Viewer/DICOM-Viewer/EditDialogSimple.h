#pragma once

#include <QObject>
#include <qdialog.h>
#include <ui_EditDialogSimple.h>

class EditDialogSimple final : public QDialog
{
	Q_OBJECT

	public:
		explicit EditDialogSimple(QDialog* parent);
		~EditDialogSimple() { }
		void setDescription(QString& description) const;
		void setValue(QString& value) const;
		QString getValue() const;

	private:
		Ui::simpleEditDialog ui{};
		bool valueModified = false;

	private slots:
		void OKPressed();
		void cancelPressed();
	
};

