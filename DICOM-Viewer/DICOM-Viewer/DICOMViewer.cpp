#include "DICOMViewer.h"

DICOMViewer::DICOMViewer(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
		if (!fileName.isEmpty())
		{
			if (file.loadFile(fileName.toStdString().c_str()).good())
			{
				if (ui.tableWidget->rowCount())
				{
					for (int i = ui.tableWidget->rowCount(); i >= 0; i--)
					{
						ui.tableWidget->removeRow(i);
					}
				}
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
	}
	else
	{
		
	}
}

void DICOMViewer::extractData(DcmFileFormat file)
{
	DcmMetaInfo* metaInfo = file.getMetaInfo();
	DcmDataset* dataSet = file.getDataset();
	int index = 0;

	for (int i = 0; i < metaInfo->card(); i++)
	{
		insertInTable(metaInfo->getElement(i), index);
		index++;
	}

	for (int i = 0; i < dataSet->card(); i++)
	{
		insertInTable(dataSet->getElement(i),index);
		index++;
	}
}

void DICOMViewer::insertInTable(DcmElement* element, int index)
{
	DcmTagKey tagKey = DcmTagKey(OFstatic_cast(Uint16, element->getGTag()), OFstatic_cast(Uint16, element->getETag()));
	DcmTag tagName = DcmTag(tagKey);
	DcmVR vr = DcmVR(element->getVR());
	
	OFString value;
	element->getOFStringArray(value, true);
	QString str = QString(value.c_str());
	str.replace("\\" , " ");
	if (str.size() > 35)
	{
		str.resize(35);
		str.append("...");
	}

	DcmWidgetElement widgetElement = DcmWidgetElement(
		QString::fromStdString(tagKey.toString().c_str()),
		QString::fromStdString(vr.getVRName()),
		QString::fromStdString(std::to_string(element->getVM())),
		QString::fromStdString(std::to_string(element->getLength())),
		tagName.getTagName(),
		str);

	ui.tableWidget->insertRow(index);
	ui.tableWidget->setItem(index, 0, new QTableWidgetItem(widgetElement.getItemTag()));
	ui.tableWidget->setItem(index, 1, new QTableWidgetItem(widgetElement.getItemVR()));
	ui.tableWidget->setItem(index, 2, new QTableWidgetItem(widgetElement.getItemVM()));
	ui.tableWidget->setItem(index, 3, new QTableWidgetItem(widgetElement.getItemLength()));
	ui.tableWidget->setItem(index, 4, new QTableWidgetItem(widgetElement.getItemDescription()));
	ui.tableWidget->setItem(index, 5, new QTableWidgetItem(widgetElement.getItemValue()));

	DcmWidgetElement copyElement = DcmWidgetElement(widgetElement);
	elements.push_back(copyElement);
}

void DICOMViewer::repopulate(std::vector<DcmWidgetElement> source)
{
	ui.tableWidget->clear();
	QStringList list;
	list.append("Tag ID");
	list.append("VR");
	list.append("VM");
	list.append("Length");
	list.append("Description");
	list.append("Value");
	ui.tableWidget->setHorizontalHeaderLabels(list);
	for (int i = 0; i < source.size(); i++)
	{
		DcmWidgetElement element = DcmWidgetElement(source[i]);

		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(element.getItemTag()));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(element.getItemVR()));
		ui.tableWidget->setItem(i, 2, new QTableWidgetItem(element.getItemVM()));
		ui.tableWidget->setItem(i, 3, new QTableWidgetItem(element.getItemLength()));
		ui.tableWidget->setItem(i, 4, new QTableWidgetItem(element.getItemDescription()));
		ui.tableWidget->setItem(i, 5, new QTableWidgetItem(element.getItemValue()));
	}
}

void DICOMViewer::findText()
{
	QString text = ui.lineEdit->text();

	if (!text.isEmpty())
	{
		std::vector<DcmWidgetElement> result;
		for (DcmWidgetElement element : elements)
		{
			if (element.checkIfContains(text))
			{
				result.push_back(element);
			}
		}
		repopulate(result);
		ui.tableWidget->scrollToTop();
	}
	else
	{
		repopulate(elements);
	}
}