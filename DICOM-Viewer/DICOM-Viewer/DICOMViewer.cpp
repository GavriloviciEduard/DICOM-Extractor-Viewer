#include "DICOMViewer.h"

DICOMViewer::DICOMViewer(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
}

void DICOMViewer::closeButtonClicked()
{
	this->close();
}

void DICOMViewer::fileTriggered(QAction* qaction)
{
	QString option = qaction->text();
	if (option == "Open")
	{
		QString fileName = QFileDialog::getOpenFileName(this);
		if (file.loadFile(fileName.toStdString().c_str()).good())
		{
			extractData(file);
			ui.tableWidget->resizeColumnsToContents();
		}
		else
		{
			QMessageBox* messageBox = new QMessageBox();
			messageBox->setText("Failed to open DICOM file!\n");
			messageBox->exec();
		}
	}
	else
	{
		
	}
}

void DICOMViewer::extractData(DcmFileFormat file)
{
	DcmMetaInfo* metaInfo = file.getMetaInfo();
	DcmDataset* dataSet = file.getDataset();

	for (int i = 0; i < dataSet->card(); i++)
	{
		insertInTable(dataSet->getElement(i),i);
	}
	for (int i = 0; i < metaInfo->card(); i++)
	{
		insertInTable(metaInfo->getElement(i), i);
	}
}

void DICOMViewer::insertInTable(DcmElement* element, int index)
{
	DcmTagKey tagKey = DcmTagKey(OFstatic_cast(Uint16, element->getGTag()), OFstatic_cast(Uint16, element->getETag()));
	DcmTag tagName = DcmTag(tagKey);
	DcmVR* vr = new DcmVR(element->getVR());
	
	OFString value;
	element->getOFString(value, 0, true);

	QTableWidgetItem* itemTag = new QTableWidgetItem();
	QTableWidgetItem* itemVR = new QTableWidgetItem();
	QTableWidgetItem* itemVM = new QTableWidgetItem();
	QTableWidgetItem* itemLength = new QTableWidgetItem();
	QTableWidgetItem* itemDescription = new QTableWidgetItem();
	QTableWidgetItem* itemValue = new QTableWidgetItem();

	itemDescription->setText(tagName.getTagName());
	itemTag->setText(QString::fromStdString(tagKey.toString().c_str()));
	itemVR->setText(QString::fromStdString(vr->getVRName()));
	itemVM->setText(QString::fromStdString(std::to_string(element->getVM())));
	itemLength->setText(QString::fromStdString(std::to_string(element->getLength())));
	itemValue->setText(QString::fromStdString(value.c_str()));

	ui.tableWidget->insertRow(index);
	ui.tableWidget->setItem(index, 0, itemTag);
	ui.tableWidget->setItem(index, 1, itemVR);
	ui.tableWidget->setItem(index, 2, itemVM);
	ui.tableWidget->setItem(index, 3, itemLength);
	ui.tableWidget->setItem(index, 4, itemDescription);
	ui.tableWidget->setItem(index, 5, itemValue);
}