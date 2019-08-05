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

int DcmWidgetElement::getDepth()
{
	return this->depth;
}

void DcmWidgetElement::setDepth(const int & depth)
{
	this->depth = depth;
}

void DcmWidgetElement::incrementDepth()
{
	this->depth++;
}

void DcmWidgetElement::setItemTag(const QString & final)
{
	this->itemTag = final;
}


bool DcmWidgetElement::checkIfContains(QString str)
{
	return this->itemTag.toUpper().contains(str.toUpper()) || this->itemVM.toUpper().contains(str.toUpper()) || this->itemVR.toUpper().contains(str.toUpper()) || this->itemLength.toUpper().contains(str.toUpper()) || this->itemDescription.toUpper().contains(str.toUpper()) || this->itemValue.toUpper().contains(str.toUpper());
}

DcmTagKey DcmWidgetElement::extractTagKey()
{
	QString str = this->getItemTag().toUpper();
	str.replace(" ", "");
	str.replace("(", "");
	str.replace(")", "");
	QStringList list = str.split(",");
	return DcmTagKey(hexToDecimal(list[0].toStdString().c_str()), hexToDecimal(list[1].toStdString().c_str()));
}

bool DcmWidgetElement::isInSequence()
{
	QString str = this->getItemTag().replace(" ", "");
	if (str != this->getItemTag())
	{
		return true;
	}
	return false;
}

int DcmWidgetElement::hexToDecimal(const char * hex)
{
	int length = strlen(hex);
	int base = 1;
	int result = 0;
	for (int i = length - 1; i >= 0; i--)
	{
		if (hex[i] >= '0' && hex[i] <= '9')
		{
			result += (hex[i] - 48) * base;
			base *= 16;
		}
		else if (hex[i] >= 'A' && hex[i] <= 'F')
		{
			result += (hex[i] - 55) * base;
			base *= 16;
		}
	}
	return result;
}

bool DcmWidgetElement::operator==(DcmWidgetElement & element)
{
	return this->getItemTag() == element.getItemTag() &&
		this->getItemVM() == element.getItemVM() &&
		this->getItemVR() == element.getItemVR() &&
		this->getItemLength() == element.getItemLength() &&
		this->getItemValue() == element.getItemValue();
}
