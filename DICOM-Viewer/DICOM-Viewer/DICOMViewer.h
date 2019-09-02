#pragma once

//#ifdef _DEBUG
//#include "vld.h"
//#pragma  comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
//#endif

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include "ui_DICOMViewer.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcvr.h"
#include "dcmtk/dcmdata/dctag.h"
#include "DcmWidgetElement.h"
#include "EditDialogSimple.h"
#include "TagSelectDialog.h"
#include "CompareDialog.h"


class DICOMViewer final : public QMainWindow
{
	Q_OBJECT

	public:
		explicit DICOMViewer(QWidget *parent = Q_NULLPTR);
		~DICOMViewer() = default;

	private:
		Ui::DICOMViewerClass ui{};
		DcmFileFormat file;
		std::vector<DcmWidgetElement> elements;
		std::vector<DcmWidgetElement> nestedElements;
		unsigned long globalIndex = 0;
		int depthRE = 0;
		QModelIndex scrollPosition;
		CompareDialog* dialog{};
		void insertInTable(DcmElement* element);
		void extractData(DcmFileFormat file);
		void repopulate(std::vector<DcmWidgetElement> source) const;
		void getNestedSequences(const DcmTagKey& tag, DcmSequenceOfItems* sequence);
		void iterateItem(DcmItem *item, int& depth);
		void clearTable();
		static void alertFailed(const std::string& message);
		static void indent(DcmWidgetElement& element, int depth);
		DcmWidgetElement createElement(DcmElement* element = nullptr, DcmSequenceOfItems* sequence = nullptr, DcmItem* item = nullptr) const;
		void insert(DcmWidgetElement element, unsigned long &index) const;
		static double getFileSize(const std::string& fileName);
		void  getTagKeyOfSequence(int row, DcmTagKey* returnKey, int* numberInSequence) const;
		static bool deleteElementFromFile(DcmSequenceOfItems* sequence, DcmWidgetElement element, QList<DcmWidgetElement> list);
		static bool modifyValue(DcmSequenceOfItems* sequence, DcmWidgetElement element, QList<DcmWidgetElement> list,const QString& value);
		bool insertElement(DcmSequenceOfItems* sequence, DcmWidgetElement element, DcmWidgetElement insertElement, QList<DcmWidgetElement> list) const;
		void createSimpleEditDialog(DcmWidgetElement element);
		void generatePathToRoot(DcmWidgetElement element, int row, QList<DcmWidgetElement> *elements);
		static bool shouldModify(DcmWidgetElement element);
		int currentRow(DcmWidgetElement element,const int& finalRow) const;
		static void precision(std::string& nr, const int& precision);
		void findIndexInserted(DcmWidgetElement& element);

	private slots:
		void fileTriggered(QAction* qaction);
		void closeButtonClicked();
		void editClicked();
		void deleteClicked();
		void insertClicked();
		void findText();
		void tableClicked(int row, int collumn);
};
