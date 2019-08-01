#include "DICOMViewer.h"
#include <fstream>

std::ofstream fout("file.out");



DICOMViewer::DICOMViewer(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableWidget->verticalHeader()->setDefaultSectionSize(12);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
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
				ui.tableWidget->scrollToTop();
				clearTable();
				extractData(file);
				ui.tableWidget->resizeColumnsToContents();
				this->setWindowTitle("Viewer - " + fileName);
			}
			else
			{
				alertFailed("Failed to open file!");
			}
		}
	}

	else if(option == "Close")
	{
		clearTable();
	}
}

void DICOMViewer::extractData(DcmFileFormat file)
{
	DcmMetaInfo* metaInfo = file.getMetaInfo();
	DcmDataset* dataSet = file.getDataset();

	for (int i = 0; i < metaInfo->card(); i++)
	{
		insertInTable(metaInfo->getElement(i));	
	}

	for (int i = 0; i < dataSet->card(); i++)
	{
		insertInTable(dataSet->getElement(i));
	}
}

void DICOMViewer::insertInTable(DcmElement* element)
{
	DcmTagKey tagKey = DcmTagKey(OFstatic_cast(Uint16, element->getGTag()), OFstatic_cast(Uint16, element->getETag()));
	DcmTag tagName = DcmTag(tagKey);
	DcmVR vr = DcmVR(element->getVR());
	std::string str;

	for (int i = 0; i < 10; i++)
	{
		OFString value;
		element->getOFString(value, i, false);
		str.append(value.c_str());
		str.append(" ");
	}
	
	depthRE = 0;
	getNestedSequences(tagKey);

	if (this->nestedElements.size())
	{
		for (auto element : this->nestedElements)
		{
			DcmWidgetElement widgetElement = DcmWidgetElement(
				element.getItemTag(),
				element.getItemVR(),
				element.getItemVM(),
				element.getItemLength(),
				element.getItemDescription(),
				element.getItemValue());


			indent(widgetElement, element.getDepth());

			ui.tableWidget->insertRow(globalIndex);
			ui.tableWidget->setItem(globalIndex, 0, new QTableWidgetItem(widgetElement.getItemTag()));
			ui.tableWidget->setItem(globalIndex, 1, new QTableWidgetItem(widgetElement.getItemVR()));
			ui.tableWidget->setItem(globalIndex, 2, new QTableWidgetItem(widgetElement.getItemVM()));
			ui.tableWidget->setItem(globalIndex, 3, new QTableWidgetItem(widgetElement.getItemLength()));
			ui.tableWidget->setItem(globalIndex, 4, new QTableWidgetItem(widgetElement.getItemDescription()));
			ui.tableWidget->setItem(globalIndex, 5, new QTableWidgetItem(widgetElement.getItemValue()));

			DcmWidgetElement copyElement = DcmWidgetElement(widgetElement);
			elements.push_back(copyElement);
			globalIndex++;

			fout <<widgetElement.toString() << " " << "Depth: " << element.getDepth() << '\n';
		}

		fout << '\n' << '\n';
		this->nestedElements.clear();
	}
	
	else
	{
		DcmWidgetElement widgetElement = DcmWidgetElement(
			QString::fromStdString(tagKey.toString().c_str()),
			QString::fromStdString(vr.getVRName()),
			QString::fromStdString(std::to_string(element->getVM())),
			QString::fromStdString(std::to_string(element->getLength())),
			tagName.getTagName(),
			QString::fromStdString(str));

		ui.tableWidget->insertRow(globalIndex);
		ui.tableWidget->setItem(globalIndex, 0, new QTableWidgetItem(widgetElement.getItemTag()));
		ui.tableWidget->setItem(globalIndex, 1, new QTableWidgetItem(widgetElement.getItemVR()));
		ui.tableWidget->setItem(globalIndex, 2, new QTableWidgetItem(widgetElement.getItemVM()));
		ui.tableWidget->setItem(globalIndex, 3, new QTableWidgetItem(widgetElement.getItemLength()));
		ui.tableWidget->setItem(globalIndex, 4, new QTableWidgetItem(widgetElement.getItemDescription()));
		ui.tableWidget->setItem(globalIndex, 5, new QTableWidgetItem(widgetElement.getItemValue()));

		DcmWidgetElement copyElement = DcmWidgetElement(widgetElement);
		elements.push_back(copyElement);

		globalIndex++;
	}

	
}

void DICOMViewer::repopulate(std::vector<DcmWidgetElement> source)
{
	ui.tableWidget->clearContents();
	for (int i = ui.tableWidget->rowCount(); i >= 0; i--)
	{
		ui.tableWidget->removeRow(i);
	}

	for (int i = 0; i < source.size(); i++)
	{
		DcmWidgetElement element = DcmWidgetElement(source[i]);

		ui.tableWidget->insertRow(i);
		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(element.getItemTag()));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(element.getItemVR()));
		ui.tableWidget->setItem(i, 2, new QTableWidgetItem(element.getItemVM()));
		ui.tableWidget->setItem(i, 3, new QTableWidgetItem(element.getItemLength()));
		ui.tableWidget->setItem(i, 4, new QTableWidgetItem(element.getItemDescription()));
		ui.tableWidget->setItem(i, 5, new QTableWidgetItem(element.getItemValue()));
	}
}

void DICOMViewer::getNestedSequences(DcmTagKey tag)
{
	DcmSequenceOfItems *sequence = NULL;
	OFCondition cond = file.getDataset()->findAndGetSequence(tag, sequence, true);
	

	if (cond.good())
	{

		DcmTagKey tagKey = DcmTagKey(OFstatic_cast(Uint16, sequence->getGTag()), OFstatic_cast(Uint16, sequence->getETag()));
		DcmTag tagName = DcmTag(tagKey);
		DcmVR vr = DcmVR(sequence->getVR());

		std::string str;
		for (int i = 0; i < 10; i++)
		{
			OFString value;
			sequence->getOFString(value, i, false);
			str.append(value.c_str());
			str.append(" ");
		}

		DcmWidgetElement widgetElement1 = DcmWidgetElement(
			QString::fromStdString(tagKey.toString().c_str()),
			QString::fromStdString(vr.getVRName()),
			QString::fromStdString(std::to_string(sequence->getVM())),
			QString::fromStdString(std::to_string(sequence->getLength())),
			tagName.getTagName(),
			QString::fromStdString(str));

		widgetElement1.setDepth(depthRE);
		
		this->nestedElements.push_back(widgetElement1);

		depthRE++;

		for (int i = 0; i < sequence->card(); i++)
		{
			DcmItem *item = sequence->getItem(i);
			DcmTagKey tagKey = DcmTagKey(OFstatic_cast(Uint16, item->getGTag()), OFstatic_cast(Uint16, item->getETag()));

			DcmTag tagName = DcmTag(tagKey);
			DcmVR vr = DcmVR(item->getVR());

			DcmWidgetElement widgetElement2 = DcmWidgetElement(
				QString::fromStdString(tagKey.toString().c_str()),
				QString::fromStdString(vr.getVRName()),
				QString::fromStdString(std::to_string(item->getVM())),
				QString::fromStdString(std::to_string(item->getLength())),
				tagName.getTagName(),
				QString::fromStdString(""));

			widgetElement2.setDepth(depthRE);

			this->nestedElements.push_back(widgetElement2);
			

			iterateItem(item,depthRE);

			DcmWidgetElement widgetElementDelim = DcmWidgetElement(QString("(fffe,e00d)"), QString("??"), QString("0"), QString("0"), QString("ItemDelimitationItem"), QString(""));
			depthRE--;
			widgetElementDelim.setDepth(depthRE);
			this->nestedElements.push_back(widgetElementDelim);
		}

		DcmWidgetElement widgetElementDelim = DcmWidgetElement(QString("(fffe,e0dd)"), QString("??"), QString("0"), QString("0"), QString("SequenceDelimitationItem"), QString(""));
		depthRE--;
		widgetElementDelim.setDepth(depthRE);

		this->nestedElements.push_back(widgetElementDelim);
	}
}

void DICOMViewer::iterateItem(DcmItem * item,int& depth)
{

	depth++;

	for (int i = 0; i < item->getNumberOfValues(); i++)
	{
		DcmElement* element = item->getElement(i);
		DcmTagKey tagKey = DcmTagKey(OFstatic_cast(Uint16, element->getGTag()), OFstatic_cast(Uint16, element->getETag()));

		DcmTag tagName = DcmTag(tagKey);
		DcmVR vr = DcmVR(element->getVR());

		std::string str;

		for (int i = 0; i < 10; i++)
		{
			OFString value;
			element->getOFString(value, i, false);
			str.append(value.c_str());
			str.append(" ");
		}

		DcmWidgetElement widgetElement = DcmWidgetElement(
			QString::fromStdString(tagKey.toString().c_str()),
			QString::fromStdString(vr.getVRName()),
			QString::fromStdString(std::to_string(element->getVM())),
			QString::fromStdString(std::to_string(element->getLength())),
			tagName.getTagName(),
			QString::fromStdString(str));
	

		if (widgetElement.getItemVR() != "SQ")
		{
			widgetElement.setDepth(depth);
			this->nestedElements.push_back(widgetElement);
		}

		getNestedSequences(tagKey);
	}
}

void DICOMViewer::clearTable()
{
	if (ui.tableWidget->rowCount())
	{
		this->nestedElements.clear();
		this->elements.clear();
		globalIndex = 0;

		for (int i = ui.tableWidget->rowCount(); i >= 0; i--)
		{
			ui.tableWidget->removeRow(i);
		}
		ui.lineEdit->clear();
		this->setWindowTitle("Viewer");
	}
}

void DICOMViewer::alertFailed(std::string message)
{
	QMessageBox* messageBox = new QMessageBox();
	messageBox->setText(QString::fromStdString(message));
	messageBox->exec();
}

void DICOMViewer::indent(DcmWidgetElement & element, int depth)
{
	QString str = element.getItemTag();

	while (depth--)
	{
		str.insert(0, "  ");//2 SPACES DEFAULT
	}

	element.setItemTag(str);

}

void DICOMViewer::findText()
{
	QString text = ui.lineEdit->text();
	if (!text.isEmpty())
	{
		if (elements.size())
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
	}
	else
	{
		repopulate(elements);
	}
}

void DICOMViewer::closeButtonClicked()
{
	close();
}