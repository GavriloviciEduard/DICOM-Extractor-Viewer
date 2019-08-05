#include "EditDialogSimple.h"


EditDialogSimple::EditDialogSimple(QDialog * parent)
{
	ui.setupUi(this); 
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	this->layout()-> setSizeConstraint(QLayout::SetFixedSize);
}


EditDialogSimple::~EditDialogSimple()
{
	delete this;
}

void EditDialogSimple::setDescription(QString& description)
{
	ui.DescriptionLabel->setText(description);
}

void EditDialogSimple::setValue(QString & value)
{
	ui.ValueEdit->setText(value.trimmed());
}

QString EditDialogSimple::getValue()
{
	return ui.ValueEdit->text();
}

void EditDialogSimple::cancelPressed()
{
	ui.ValueEdit->clear();
	this->close();
}

void EditDialogSimple::OKPressed()
{
	this->close();
}