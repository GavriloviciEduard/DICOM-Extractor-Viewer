#pragma once

#include <QtWidgets>

class DcmWidgetElement
{
private:
	QString itemTag;
	QString itemVR;
	QString itemVM;
	QString itemLength;
	QString itemDescription;
	QString itemValue;

public:
	DcmWidgetElement();
	DcmWidgetElement(const QString &itemTag, const QString &itemVR, const QString &itemVM, const QString &itemLength, const QString &itemDescription, const QString &itemValue);
	~DcmWidgetElement();

	QString getItemTag();
	QString getItemVR();
	QString getItemVM();
	QString getItemLength();
	QString getItemDescription();
	QString getItemValue();

	bool checkIfContains(QString str);
};