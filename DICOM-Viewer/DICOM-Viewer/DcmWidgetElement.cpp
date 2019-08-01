#include "DcmWidgetElement.h"

DcmWidgetElement::DcmWidgetElement(const QString &itemTag, const QString &itemVR, const QString &itemVM, const QString &itemLength, const QString &itemDescription, const QString &itemValue)
{
	this->itemTag = itemTag;
	this->itemVR = itemVR;
	this->itemVM = itemVM;
	this->itemLength = itemLength;
	this->itemDescription = itemDescription;
	this->itemValue = itemValue;
}


QString DcmWidgetElement::getItemTag()
{
	return this->itemTag;
}

QString DcmWidgetElement::getItemVR()
{
	return this->itemVR;
}

QString DcmWidgetElement::getItemVM()
{
	return this->itemVM;
}

QString DcmWidgetElement::getItemLength()
{
	return this->itemLength;
}

QString DcmWidgetElement::getItemDescription()
{
	return this->itemDescription;
}

QString DcmWidgetElement::getItemValue()
{
	return this->itemValue;
}

std::string DcmWidgetElement::toString()
{
	return std::string(this->itemTag.toStdString() + " " + this->itemVM.toStdString() + " "+ this->itemVR.toStdString() + " " + this->itemLength.toStdString() + " " + this->itemDescription.toStdString() + " " + this->itemValue.toStdString());
}

bool DcmWidgetElement::checkIfContains(QString str)
{
	return this->itemTag.contains(str) || this->itemVM.contains(str) || this->itemVR.contains(str) || this->itemLength.contains(str) || this->itemDescription.contains(str) || this->itemValue.contains(str);
}