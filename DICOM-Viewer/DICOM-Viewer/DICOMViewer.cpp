#include "DICOMViewer.h"
#include <fstream>

#define SPACE "  "

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
				this->clearTable();
				this->extractData(file);
				ui.tableWidget->resizeColumnsToContents();
				this->setWindowTitle("Viewer - " + fileName);
				ui.label->setText("Size: " + QString::fromStdString(std::to_string(getFileSize(fileName.toStdString())) + " KB"));
			}
			else
			{
				this->alertFailed("Failed to open file!");
			}
		}
	}

	else if(option == "Close")
	{
		this->clearTable();
	}
	else if (option == "Compare")
	{

	}
}

void DICOMViewer::extractData(DcmFileFormat file)
{
	DcmMetaInfo* metaInfo = file.getMetaInfo();
	DcmDataset* dataSet = file.getDataset();

	for (int i = 0; i < metaInfo->card(); i++)
	{
		this->insertInTable(metaInfo->getElement(i));
	}

	for (int i = 0; i < dataSet->card(); i++)
	{
		this->insertInTable(dataSet->getElement(i));
	}
}

void DICOMViewer::insertInTable(DcmElement* element)
{
		
	this->depthRE = 0;
	this->getNestedSequences(DcmTagKey(
		OFstatic_cast(Uint16, element->getGTag()), 
		OFstatic_cast(Uint16, element->getETag())));

	if (this->nestedElements.size())
	{
		for (auto element : this->nestedElements)
		{
			this->indent(element, element.getDepth());

			this->insert(element, globalIndex);

			DcmWidgetElement copyElement = DcmWidgetElement(element);
			this->elements.push_back(copyElement);
			this->globalIndex++;

		}

		this->nestedElements.clear();
	}
	
	else
	{
		DcmWidgetElement widgetElement = createElement(element,nullptr,nullptr);

		this->insert(widgetElement, globalIndex);

		DcmWidgetElement copyElement = DcmWidgetElement(widgetElement);
		this->elements.push_back(copyElement);

		this->globalIndex++;
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

		this->insert(element, i);
	}
}

void DICOMViewer::getNestedSequences(DcmTagKey tag)
{
	DcmSequenceOfItems *sequence = NULL;
	OFCondition cond = file.getDataset()->findAndGetSequence(tag, sequence, true);

	if (cond.good())
	{
		DcmWidgetElement widgetElement1 = createElement(nullptr,sequence,nullptr);

		widgetElement1.setDepth(this->depthRE);
		
		this->nestedElements.push_back(widgetElement1);

		this->depthRE++;

		for (int i = 0; i < sequence->card(); i++)
		{
			DcmWidgetElement widgetElement2 = createElement(nullptr,nullptr, sequence->getItem(i));

			widgetElement2.setDepth(this->depthRE);

			this->nestedElements.push_back(widgetElement2);
			

			this->iterateItem(sequence->getItem(i), this->depthRE);

			DcmWidgetElement widgetElementDelim = DcmWidgetElement(
				QString("(fffe,e00d)"), 
				QString("??"), QString("0"),
				QString("0"), 
				QString("ItemDelimitationItem"), 
				QString(""));
			this->depthRE--;
			widgetElementDelim.setDepth(this->depthRE);
			this->nestedElements.push_back(widgetElementDelim);
		}

		DcmWidgetElement widgetElementDelim = DcmWidgetElement(
			QString("(fffe,e0dd)"), 
			QString("??"), 
			QString("0"), 
			QString("0"), 
			QString("SequenceDelimitationItem"), 
			QString(""));
		this->depthRE--;
		widgetElementDelim.setDepth(this->depthRE);

		this->nestedElements.push_back(widgetElementDelim);
	}
}

void DICOMViewer::iterateItem(DcmItem * item,int& depth)
{
	depth++;
	for (int i = 0; i < item->getNumberOfValues(); i++)
	{
		DcmWidgetElement widgetElement = createElement(item->getElement(i),nullptr,nullptr);
		if (widgetElement.getItemVR() != "SQ")
		{
			widgetElement.setDepth(depth);
			this->nestedElements.push_back(widgetElement);
		}

		this->getNestedSequences(DcmTagKey(
			OFstatic_cast(Uint16, item->getElement(i)->getGTag()), 
			OFstatic_cast(Uint16, item->getElement(i)->getETag())));
	}
}

void DICOMViewer::clearTable()
{
	if (ui.tableWidget->rowCount())
	{
		this->nestedElements.clear();
		this->elements.clear();
		this->globalIndex = 0;

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
	messageBox->setIcon(QMessageBox::Warning);
	messageBox->setText(QString::fromStdString(message));
	messageBox->exec();
	delete messageBox;
}

void DICOMViewer::indent(DcmWidgetElement & element, int depth)
{
	QString str = element.getItemTag();
	while (depth--)
	{
		str.insert(0, SPACE);
	}

	element.setItemTag(str);
}

DcmWidgetElement DICOMViewer::createElement(
	DcmElement* element, 
	DcmSequenceOfItems* sequence, 
	DcmItem* item)
{
	if (element)
	{
		DcmTagKey tagKey = DcmTagKey(
			OFstatic_cast(Uint16, element->getGTag()), 
			OFstatic_cast(Uint16, element->getETag()));
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

		return widgetElement;
	}
	else if (sequence)
	{
		DcmTagKey tagKey = DcmTagKey(
			OFstatic_cast(Uint16, sequence->getGTag()), 
			OFstatic_cast(Uint16, sequence->getETag()));
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

		DcmWidgetElement widgetElement = DcmWidgetElement(
			QString::fromStdString(tagKey.toString().c_str()),
			QString::fromStdString(vr.getVRName()),
			QString::fromStdString(std::to_string(sequence->getVM())),
			QString::fromStdString(std::to_string(sequence->getLength())),
			tagName.getTagName(),
			QString::fromStdString(str));

		return widgetElement;
	}
	else if (item)
	{
		DcmTagKey tagKey = DcmTagKey(
			OFstatic_cast(Uint16, item->getGTag()),
			OFstatic_cast(Uint16, item->getETag()));
		DcmTag tagName = DcmTag(tagKey);
		DcmVR vr = DcmVR(item->getVR());

		DcmWidgetElement widgetElement = DcmWidgetElement(
			QString::fromStdString(tagKey.toString().c_str()),
			QString::fromStdString(vr.getVRName()),
			QString::fromStdString(std::to_string(item->getVM())),
			QString::fromStdString(std::to_string(item->getLength())),
			tagName.getTagName(),
			QString::fromStdString(""));

		return widgetElement;
	}
}

void DICOMViewer::insert(DcmWidgetElement element, int &index)
{
	ui.tableWidget->insertRow(index);
	ui.tableWidget->setItem(index, 0, new QTableWidgetItem(element.getItemTag()));
	ui.tableWidget->setItem(index, 1, new QTableWidgetItem(element.getItemVR()));
	ui.tableWidget->setItem(index, 2, new QTableWidgetItem(element.getItemVM()));
	ui.tableWidget->setItem(index, 3, new QTableWidgetItem(element.getItemLength()));
	ui.tableWidget->setItem(index, 4, new QTableWidgetItem(element.getItemDescription()));
	ui.tableWidget->setItem(index, 5, new QTableWidgetItem(element.getItemValue()));
}

int DICOMViewer::getFileSize(std::string fileName)
{
	std::ifstream in_file(fileName, std::ios::binary | std::ios::ate);
	int size = in_file.tellg() / 1024;
	in_file.close();
	return size;
}

void DICOMViewer::findText()
{
	QString text = ui.lineEdit->text();
	if (!text.isEmpty())
	{
		if (this->elements.size())
		{
			std::vector<DcmWidgetElement> result;
			for (DcmWidgetElement element : this->elements)
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
		repopulate(this->elements);
	}
}

void DICOMViewer::closeButtonClicked()
{
	this->close();
}

void DICOMViewer::editClicked()
{
}

void DICOMViewer::deleteClicked()
{
}

void DICOMViewer::insertClicked()
{
}
