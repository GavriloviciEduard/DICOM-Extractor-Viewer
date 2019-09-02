#include "EditDialogSimple.h"


EditDialogSimple::EditDialogSimple(QDialog * parent)
{
	ui.setupUi(this); 
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
}

//========================================================================================================================
void EditDialogSimple::setDescription(QString& description) const
{
	ui.DescriptionLabel->setText(description);
}

//========================================================================================================================
void EditDialogSimple::setValue(QString & value) const
{
	ui.ValueEdit->setText(value.trimmed());
}

//========================================================================================================================
QString EditDialogSimple::getValue() const
{
	return ui.ValueEdit->text();
}

//========================================================================================================================
void EditDialogSimple::cancelPressed()
{
	ui.ValueEdit->clear();
	this->close();
}

//========================================================================================================================
void EditDialogSimple::OKPressed()
{
	this->close();
}