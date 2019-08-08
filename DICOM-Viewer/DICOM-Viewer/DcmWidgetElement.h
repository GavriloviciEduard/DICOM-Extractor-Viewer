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
		int tableIndex = -1;

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
		int getTableIndex();
		int getDepth();
		void calculateTableIndex(const int& current,const std::vector<DcmWidgetElement>& elements);

		std::string toString();
		void setDepth(const int& depth);
		void setTableIndex(const int& index);
		void incrementDepth();
		void setItemTag(const QString& final);
		bool checkIfContains(QString str);
		DcmTagKey extractTagKey();
		void calculateDepthFromTag();
		static int hexToDecimal(const char* hex);
		bool operator==(DcmWidgetElement &element);
		bool operator>(DcmWidgetElement &element);
		bool operator<(DcmWidgetElement &element);

		int compareTagKey(DcmWidgetElement &element);
};