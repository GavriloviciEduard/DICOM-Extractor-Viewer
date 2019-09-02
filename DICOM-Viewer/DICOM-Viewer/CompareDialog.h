#pragma once

#include <QObject>
#include "ui_CompareDialog.h"
#include <QtWidgets/qtablewidget.h>
#include "DcmWidgetElement.h"
#include "DcmWidgetElement.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dctagkey.h"

class CompareDialog final : public QWidget
{
	Q_OBJECT;

	public:
		explicit CompareDialog(QWidget* parent);
		~CompareDialog() = default;

private:
		Ui::dialogCompare ui{};
		DcmFileFormat file1;
		DcmFileFormat file2;
		std::vector<DcmWidgetElement> elements1;
		std::vector<DcmWidgetElement> elements2;
		std::vector<DcmWidgetElement> nestedElements;
		std::vector<std::tuple<DcmWidgetElement, DcmWidgetElement>> tableElements;
		int depthRE = 0;
		int globalIndex = 0;
		bool firstFile = false;
		int loaded = 0;

		void loadFile(DcmFileFormat* file, const QString& fileName, bool first);
		static void alertFailed(const std::string& message);
		void extractData(DcmFileFormat* file);
		void insertInTable(DcmElement* element, DcmFileFormat* file);
		void getNestedSequences(const DcmTagKey& tag, DcmSequenceOfItems* sequence, DcmFileFormat* file);
		static void indent(DcmWidgetElement& element, int depth);
		void iterateItem(DcmItem *item, int& depth, DcmFileFormat* file);
		void insert(DcmWidgetElement element, int &index) const;
		void insertRight(DcmWidgetElement element, int &index) const;
		DcmWidgetElement createElement(DcmElement* element = nullptr, DcmSequenceOfItems* sequence = nullptr, DcmItem* item = nullptr) const;
		void insertBoth(DcmWidgetElement el1, DcmWidgetElement el2, int &index, int status) const;
		void insertSequence(int status, int& index1, int& index2);
		void merge();
		void clearTable() const;
		static bool isDelimitation(DcmWidgetElement& el);
		void populateTableElementsVector();
		static void precision(std::string& nr, const int& precision);
		static double getFileSize(const std::string& fileName);
		static void replace(std::string& str, const std::string& from, const std::string& to);


	private slots:
		void loadFile1();
		void loadFile2();
		void findText();
};