#pragma once

#include <QtWidgets>
#include <dcmtk/dcmdata/dcdeftag.h>

class DcmWidgetElement
{
	private:
		QString itemTag;
		QString itemVR;
		QString itemVM;
		QString itemLength;
		QString itemDescription;
		QString itemValue;
		int depth = -1;

	public:
		DcmWidgetElement() { }
		DcmWidgetElement(const QString &itemTag, const QString &itemVR, const QString &itemVM, const QString &itemLength, const QString &itemDescription, const QString &itemValue);
		~DcmWidgetElement() = default;

		QString getItemTag();
		QString getItemVR();
		QString getItemVM();
		QString getItemLength();
		QString getItemDescription();
		QString getItemValue();
		std::string toString();
		int getDepth();
		void setDepth(const int& depth);
		void incrementDepth();
		void setItemTag(const QString& final);
		bool checkIfContains(QString str);
		DcmTagKey extractTagKey();

		static int hexToDecimal(const char* hex);

		bool operator==(DcmWidgetElement &element);
};