#include "CompareDialog.h"

CompareDialog::CompareDialog(QDialog * parent)
{
	ui.setupUi(this);
}

CompareDialog::~CompareDialog()
{

}

void CompareDialog::loadFile(DcmFileFormat file, QTableWidget* table)
{
	QString fileName = QFileDialog::getOpenFileName(this);
	if (!fileName.isEmpty())
	{
		extractData(file, table);
	}
	else
	{
		alertFailed("Failed to open file!");
	}
}

void CompareDialog::extractData(DcmFileFormat file, QTableWidget* table)
{
	DcmMetaInfo* metaInfo = file.getMetaInfo();
	DcmDataset* dataSet = file.getDataset();

	for (int i = 0; i < metaInfo->card(); i++)
	{
		insertInTable(table,metaInfo->getElement(i));
	}
	for (int i = 0; i < dataSet->card(); i++)
	{
		insertInTable(table,dataSet->getElement(i));
	}
}

void CompareDialog::insertInTable(QTableWidget* table, DcmElement* element)
{

}

void CompareDialog::getNestedSequences(DcmTagKey tag)
{

}

void CompareDialog::iterateItem(DcmItem* item, int &depth)
{
	/*depth++;
	for (int i = 0; i < item->getNumberOfValues(); i++)
	{
		DcmWidgetElement widgetElement = createElement(item->getElement(i), nullptr, nullptr);
		if (widgetElement.getItemVR() != "SQ")
		{
			widgetElement.setDepth(depth);
			this->nestedElements.push_back(widgetElement);
		}

		this->getNestedSequences(item->getElement(i)->getTag().getBaseTag());
	}*/
}

void CompareDialog::loadFile1()
{
	this->loadFile(file1,ui.tableWidget1);
}

void CompareDialog::loadFile2()
{
	this->loadFile(file2,ui.tableWidget2);
}

void CompareDialog::alertFailed(std::string message)
{
	QMessageBox* messageBox = new QMessageBox();
	messageBox->setIcon(QMessageBox::Warning);
	messageBox->setText(QString::fromStdString(message));
	messageBox->exec();
	delete messageBox;
}