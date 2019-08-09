#include "CompareDialog.h"

#define SPACE "  "

CompareDialog::CompareDialog(QDialog * parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui.tableWidget1->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableWidget1->verticalHeader()->setDefaultSectionSize(12);
	ui.tableWidget1->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget1->setSelectionMode(QAbstractItemView::NoSelection);
	ui.tableWidget1->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

//========================================================================================================================
void CompareDialog::loadFile(DcmFileFormat* file, QString fileName, bool first)
{
	if (file->loadFile(fileName.toStdString().c_str()).good())
	{
		extractData(file);
	}
	else
	{
		this->alertFailed("Failed to open file!");
	}

	if (first)
	{
		ui.buttonLoad1->setText(fileName);
	}
	else
	{
		ui.butonLoad2->setText(fileName);
	}
}

//========================================================================================================================
void CompareDialog::extractData(DcmFileFormat* file)
{
	DcmMetaInfo* metaInfo = file->getMetaInfo();
	DcmDataset* dataSet = file->getDataset();

	for (int i = 0; i < metaInfo->card(); i++)
	{
		insertInTable(metaInfo->getElement(i),file);
	}

	for (int i = 0; i < dataSet->card(); i++)
	{
		insertInTable(dataSet->getElement(i),file);
	}

	if (loaded >= 2)
	{
		merge();
	}

	globalIndex = 0;
}

//========================================================================================================================
void CompareDialog::insertInTable(DcmElement* element, DcmFileFormat* file)
{
	this->depthRE = 0;
	this->getNestedSequences(element->getTag().getBaseTag(),nullptr,file);

	if (this->nestedElements.size())
	{
		for (auto element : this->nestedElements)
		{
			this->indent(element, element.getDepth());
			DcmWidgetElement copyElement = DcmWidgetElement(element);
			copyElement.setTableIndex(globalIndex);
			copyElement.calculateDepthFromTag();

			if (firstFile == true)
			{
				this->elements1.push_back(copyElement);
			}

			else
			{
				this->elements2.push_back(copyElement);
			}

			this->globalIndex++;
		}

		this->nestedElements.clear();
	}

	else
	{
		DcmWidgetElement widgetElement = createElement(element, nullptr, nullptr);
		widgetElement.setItemTag(widgetElement.getItemTag().toUpper());
		DcmWidgetElement copyElement = DcmWidgetElement(widgetElement);
		copyElement.setTableIndex(globalIndex);
		copyElement.calculateDepthFromTag();

		if (firstFile == true)
		{
			this->elements1.push_back(copyElement);
		}

		else
		{
			this->elements2.push_back(copyElement);
		}

		this->globalIndex++;
	}
}

//========================================================================================================================
void CompareDialog::getNestedSequences(DcmTagKey tag, DcmSequenceOfItems* sequence, DcmFileFormat* file)
{
	OFCondition cond = OFCondition(EC_CorruptedData);

	if (tag.hasValidGroup())
		cond = file->getDataset()->findAndGetSequence(tag, sequence, true);

	if (cond.good() || (sequence != nullptr &&  sequence->card()))
	{
		DcmWidgetElement widgetElement1 = createElement(nullptr, sequence, nullptr);
		widgetElement1.setDepth(this->depthRE);
		widgetElement1.setItemTag(widgetElement1.getItemTag().toUpper());
		this->nestedElements.push_back(widgetElement1);
		this->depthRE++;

		for (int i = 0; i < sequence->card(); i++)
		{
			DcmWidgetElement widgetElement2 = createElement(nullptr, nullptr, sequence->getItem(i));
			widgetElement2.setDepth(this->depthRE);
			widgetElement2.setItemTag(widgetElement2.getItemTag().toUpper());
			this->nestedElements.push_back(widgetElement2);
			this->iterateItem(sequence->getItem(i), this->depthRE, file);
			DcmWidgetElement widgetElementDelim = DcmWidgetElement(
				QString("(FFFE,E00D)"),
				QString("??"), QString("0"),
				QString("0"),
				QString("ItemDelimitationItem"),
				QString(""));
			this->depthRE--;
			widgetElementDelim.setDepth(this->depthRE);
			this->nestedElements.push_back(widgetElementDelim);
		}

		DcmWidgetElement widgetElementDelim = DcmWidgetElement(
			QString("(FFFE,E0DD)"),
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

//========================================================================================================================
void CompareDialog::indent(DcmWidgetElement & element, int depth)
{
	QString str = element.getItemTag();

	while (depth--)
	{
		str.insert(0, SPACE);
	}

	element.setItemTag(str);
}

//========================================================================================================================
void CompareDialog::iterateItem(DcmItem * item, int & depth, DcmFileFormat* file)
{
	depth++;

	for (int i = 0; i < item->getNumberOfValues(); i++)
	{
		DcmWidgetElement widgetElement = createElement(item->getElement(i), nullptr, nullptr);
		widgetElement.setItemTag(widgetElement.getItemTag().toUpper());

		if (widgetElement.getItemVR() != "SQ")
		{
			widgetElement.setDepth(depth);
			this->nestedElements.push_back(widgetElement);
		}

		DcmTagKey NULLkey;
		DcmSequenceOfItems* sequence;
		item->getElement(i)->getParentItem()->findAndGetSequence(item->getElement(i)->getTag().getBaseTag(), sequence, true);
		this->getNestedSequences(NULLkey, sequence,file);
	}
}

//========================================================================================================================
void CompareDialog::insertRight(DcmWidgetElement element, int & index)
{
	if (index >= ui.tableWidget1->rowCount())
	{
		ui.tableWidget1->insertRow(index);
	}

	ui.tableWidget1->setItem(index, 3, new QTableWidgetItem(element.getItemTag()));
	ui.tableWidget1->setItem(index, 4, new QTableWidgetItem(element.getItemLength()));
	ui.tableWidget1->setItem(index, 5, new QTableWidgetItem(element.getItemValue()));
}

//========================================================================================================================
void CompareDialog::insert(DcmWidgetElement element, int & index)
{
	if (index >= ui.tableWidget1->rowCount())
	{
		ui.tableWidget1->insertRow(index);
	}

	ui.tableWidget1->setItem(index, 0, new QTableWidgetItem(element.getItemTag()));
	ui.tableWidget1->setItem(index, 1, new QTableWidgetItem(element.getItemLength()));
	ui.tableWidget1->setItem(index, 2, new QTableWidgetItem(element.getItemValue()));
}

//========================================================================================================================
DcmWidgetElement CompareDialog::createElement(DcmElement * element, DcmSequenceOfItems * sequence, DcmItem * item)
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

	return DcmWidgetElement();
}

//========================================================================================================================
void CompareDialog::insertBoth(DcmWidgetElement el1, DcmWidgetElement el2, int &index, int status)
{

	ui.tableWidget1->insertRow(index);

	if (status == 1)
	{
		ui.tableWidget1->setItem(index, 0, new QTableWidgetItem(el1.getItemTag()));
		ui.tableWidget1->setItem(index, 1, new QTableWidgetItem(el1.getItemLength()));
		ui.tableWidget1->setItem(index, 2, new QTableWidgetItem(el1.getItemValue()));
		ui.tableWidget1->setItem(index, 3, new QTableWidgetItem(el2.getItemLength()));
		ui.tableWidget1->setItem(index, 4, new QTableWidgetItem(el2.getItemValue()));

		if (!(el1 == el2))
		{
			ui.tableWidget1->item(index, 0)->setBackgroundColor(QColor(220, 220, 220));
			ui.tableWidget1->item(index, 1)->setBackgroundColor(QColor(250, 128, 114));
			ui.tableWidget1->item(index, 2)->setBackgroundColor(QColor(250, 128, 114));
			ui.tableWidget1->item(index, 3)->setBackgroundColor(QColor(250, 128, 114));
			ui.tableWidget1->item(index, 4)->setBackgroundColor(QColor(250, 128, 114));
		}

		else
		{
			ui.tableWidget1->item(index, 0)->setBackgroundColor(QColor(220, 220, 220));
		}
	}

	else if (status == 2)
	{
		ui.tableWidget1->setItem(index, 0, new QTableWidgetItem(el1.getItemTag()));
		ui.tableWidget1->setItem(index, 1, new QTableWidgetItem(el1.getItemLength()));
		ui.tableWidget1->setItem(index, 2, new QTableWidgetItem(el1.getItemValue()));
		ui.tableWidget1->setItem(index, 3, new QTableWidgetItem(el2.getItemLength()));
		ui.tableWidget1->setItem(index, 4, new QTableWidgetItem(el2.getItemValue()));
		ui.tableWidget1->item(index, 0)->setBackgroundColor(QColor(220, 220, 220));
		ui.tableWidget1->item(index, 3)->setBackgroundColor(QColor(104, 223, 240));
		ui.tableWidget1->item(index, 4)->setBackgroundColor(QColor(104, 223, 240));
	}

	else
	{
		ui.tableWidget1->setItem(index, 0, new QTableWidgetItem(el1.getItemTag()));
		ui.tableWidget1->setItem(index, 1, new QTableWidgetItem(el1.getItemLength()));
		ui.tableWidget1->setItem(index, 2, new QTableWidgetItem(el1.getItemValue()));
		ui.tableWidget1->setItem(index, 3, new QTableWidgetItem(el2.getItemLength()));
		ui.tableWidget1->setItem(index, 4, new QTableWidgetItem(el2.getItemValue()));
		ui.tableWidget1->item(index, 0)->setBackgroundColor(QColor(220, 220, 220));
		ui.tableWidget1->item(index, 1)->setBackgroundColor(QColor(0, 250, 154));
		ui.tableWidget1->item(index, 2)->setBackgroundColor(QColor(0, 250, 154));
	}
}

//========================================================================================================================
void CompareDialog::insertSequence(int status, int& index1, int& index2)
{
	if (status == 1)
	{
		insertBoth(this->elements1[index1], this->elements2[index2] , globalIndex, status);
		index1++;
		index2++;
		globalIndex++;

		while (this->elements1[index1].getItemDescription() == "Item" && this->elements2[index2].getItemDescription() == "Item")
		{
			int item1Depth = this->elements1[index1].getDepth();
			int item2Depth = this->elements2[index2].getDepth();
			insertBoth(this->elements1[index1], this->elements2[index2], globalIndex, status);
			index1++;
			index2++;
			globalIndex++;

			while (this->elements1[index1].getDepth() == item1Depth && this->elements2[index2].getDepth() == item2Depth)
			{
				if (this->elements1[index1].getItemVR() == "SQ" && this->elements2[index2].getItemVR() == "SQ")
				{
					insertSequence(1, index1, index2);
				}

				else if (this->elements1[index1].getItemVR() == "SQ" && this->elements2[index2].getItemVR() != "SQ")
				{
					insertSequence(2, index1, index2);
				}

				else if (this->elements1[index1].getItemVR() != "SQ" && this->elements2[index2].getItemVR() == "SQ")
				{
					insertSequence(3, index1, index2);
				}

				insertBoth(this->elements1[index1], this->elements2[index2], globalIndex, status);
				index1++;
				index2++;
				globalIndex++;
			}

			while (this->elements1[index1].getDepth() == item1Depth && this->elements2[index2].getDepth() != item2Depth)
			{
				if (this->elements1[index1].getItemVR() == "SQ")
				{
					insertSequence(status, index1, index2);
				}

				insertBoth(this->elements1[index1], DcmWidgetElement(this->elements1[index1].getItemTag(), "", "", "", "", ""), globalIndex, status);
				index1++;
				globalIndex++;
			}

			while (this->elements1[index1].getDepth() == item1Depth && this->elements2[index2].getDepth() != item2Depth)
			{
				if (this->elements2[index2].getItemVR() == "SQ")
				{
					insertSequence(status, index1, index2);
				}

				insertBoth(DcmWidgetElement(this->elements2[index2].getItemTag(), "", "", "", "", ""), this->elements2[index2], globalIndex, status);
				index2++;
				globalIndex++;
			}
				insertBoth(this->elements1[index1], this->elements2[index2], globalIndex, status);
				index1++;
				index2++;
				globalIndex++;
		}


			insertBoth(this->elements1[index1], this->elements2[index2], globalIndex, status);
			index1++;
			index2++;
			globalIndex++;
	}

	else if (status == 2)
	{
		insertBoth(this->elements1[index1], DcmWidgetElement(this->elements1[index1].getItemTag(), "", "", "", "", ""), globalIndex, 3);
		index1++;
		globalIndex++;

		while (this->elements1[index1].getItemDescription() == "Item")
		{
			int itemDepth = this->elements1[index1].getDepth();
			insertBoth(this->elements1[index1], DcmWidgetElement(this->elements1[index1].getItemTag(), "", "", "", "", ""), globalIndex, 3);
			index1++;
			globalIndex++;

			while (this->elements1[index1].getDepth() >= itemDepth +1 )
			{
				if (this->elements1[index1].getItemVR() == "SQ")
				{
					insertBoth(this->elements1[index1], DcmWidgetElement(this->elements1[index1].getItemTag(), "", "", "", "", ""), globalIndex, 3);
					index1++;
					globalIndex++;

					insertSequence(status, index1, index2);

				}

				insertBoth(this->elements1[index1], DcmWidgetElement(this->elements1[index1].getItemTag(), "", "", "", "", ""), globalIndex, 3);
				index1++;
				globalIndex++;
				
			}

			insertBoth(this->elements1[index1], DcmWidgetElement(this->elements1[index1].getItemTag(), "", "", "", "", ""), globalIndex, 3);
			index1++;
			globalIndex++;

			
		}

		if (index1 < this->elements1.size() && this->isDelimitation(this->elements1[index1]))
		{
			insertBoth(this->elements1[index1], DcmWidgetElement(this->elements1[index1].getItemTag(), "", "", "", "", ""), globalIndex, 3);
			index1++;
			globalIndex++;
		}
	}

	else
	{
		insertBoth(DcmWidgetElement(this->elements2[index2].getItemTag(), "", "", "", "", ""), this->elements2[index2], globalIndex, 2);
		index2++;
		globalIndex++;

		while (this->elements2[index2].getItemDescription() == "Item")
		{
			int itemDepth = this->elements2[index2].getDepth();
			insertBoth(DcmWidgetElement(this->elements2[index2].getItemTag(), "", "", "", "", ""), this->elements2[index2], globalIndex, 2);
			index2++;
			globalIndex++;

			while (this->elements2[index2].getDepth() >= itemDepth + 1 )
			{
				if (this->elements2[index2].getItemVR() == "SQ")
				{

					insertBoth(DcmWidgetElement(this->elements2[index2].getItemTag(), "", "", "", "", ""), this->elements2[index2], globalIndex, 2);
					index2++;
					globalIndex++;

					insertSequence(status, index1, index2);
				}

				insertBoth(DcmWidgetElement(this->elements2[index2].getItemTag(), "", "", "", "", ""), this->elements2[index2], globalIndex, 2);
				index2++;
				globalIndex++;
			}
			
			insertBoth(DcmWidgetElement(this->elements2[index2].getItemTag(), "", "", "", "", ""), this->elements2[index2], globalIndex, 2);
			index2++;
			globalIndex++;
		}

	
		if (this->isDelimitation(this->elements2[index2]))
		{
			insertBoth(DcmWidgetElement(this->elements2[index2].getItemTag(), "", "", "", "", ""), this->elements2[index2], globalIndex, 2);
			index2++;
			globalIndex++;
		}
	}
}

//========================================================================================================================
void CompareDialog::merge()
{
	if (!this->elements1.empty() && !this->elements2.empty())
	{
		globalIndex = 0;
		int index1 = 0;
		int index2 = 0;


		while (index1 < elements1.size() - 1 && index2 < elements2.size() - 1)
		{

			if (elements1[index1].getItemVR() == "SQ" && elements2[index2].getItemVR() == "SQ")
			{
				if (elements1[index1].compareTagKey(elements2[index2]) == 3)
				{
					insertSequence(2, index1, index2);
				}

				else if (elements1[index1].compareTagKey(elements2[index2]) == 2)
				{
					insertSequence(3, index1, index2);
				}

				else
				{
					insertSequence(1, index1, index2);
				}
			}

			else if (elements1[index1].getItemVR() == "SQ" && elements2[index2].getItemVR() != "SQ" && elements1[index1].compareTagKey(elements2[index2]) == 3)
			{
				insertSequence(2, index1, index2);
			}

			else if(elements1[index1].getItemVR() != "SQ" && elements2[index2].getItemVR() == "SQ" && elements1[index1].compareTagKey(elements2[index2]) == 2)
			{
				insertSequence(3, index1, index2);
			}

			else if (elements1[index1].compareTagKey(elements2[index2]) == 1 )
			{
				insertBoth(elements1[index1], elements2[index2],globalIndex, 1);
				index1++;
				index2++;
				globalIndex++;
			}

			else if(elements1[index1].compareTagKey(elements2[index2]) == 2 )
			{
				if (elements2[index2].getItemVR() == "SQ")
				{
					insertSequence(2, index1, index2);
				}

				else
				{
					DcmWidgetElement empty = DcmWidgetElement(elements2[index2].getItemTag(), "", "", "", "", "");
					insertBoth(empty, elements2[index2], globalIndex, 2);
					index2++;
					globalIndex++;
				}
			}

			else
			{
				if (elements1[index1].getItemVR() == "SQ")
				{
					insertSequence(3, index1, index2);
				}

				else
				{
					DcmWidgetElement empty = DcmWidgetElement(elements1[index1].getItemTag(), "", "", "", "", "");
					insertBoth(elements1[index1], empty, globalIndex, 3);
					index1++;
					globalIndex++;
				}
			}
		}

		while (index1 < elements1.size() - 1)
		{
			if (elements1[index1].getItemVR() == "SQ")
			{
				insertSequence(2, index1, index2);
				index1++;
			}

			else
			{
				DcmWidgetElement empty = DcmWidgetElement(elements1[index1].getItemTag(), "", "", "", "", "");
				insertBoth(elements1[index1],empty, globalIndex, 3);
				index1++;
				globalIndex++;
			}
		}

		while (index2 < elements2.size() -1)
		{
			if (elements2[index2].getItemVR() == "SQ")
			{
				insertSequence(3, index1, index2);
				index2++;
			}

			else
			{
				DcmWidgetElement empty = DcmWidgetElement(elements2[index2].getItemTag(), "", "", "", "", "");
				insertBoth(empty, elements2[index2], globalIndex, 2);
				index2++;
				globalIndex++;
			}
		}

		insertBoth(elements1[elements1.size() - 1], elements2[elements2.size() - 1],globalIndex, 1);
	}
}

//========================================================================================================================
void CompareDialog::clearTable()
{
	for (int i = ui.tableWidget1->rowCount(); i >= 0; i--)
	{
		ui.tableWidget1->removeRow(i);
	}
}

//========================================================================================================================
bool CompareDialog::isDelimitation(DcmWidgetElement & el)
{
	if(el.getItemDescription()=="ItemDelimitationItem" || el.getItemDescription() == "SequenceDelimitationItem")
		return true;

	return false;
}

//========================================================================================================================
void CompareDialog::loadFile1()
{
	QString fileName = QFileDialog::getOpenFileName(this);
	if (!fileName.isEmpty())
	{
		clearTable();
		firstFile = true;
		elements1.clear();
		loaded++;
		this->loadFile(&file1, fileName, true);
	}
}

//========================================================================================================================
void CompareDialog::loadFile2()
{
	QString fileName = QFileDialog::getOpenFileName(this);
	if (!fileName.isEmpty())
	{
		clearTable();
		firstFile = false;
		elements2.clear();
		loaded++;
		this->loadFile(&file2, fileName, false);
	}
}

//========================================================================================================================
void CompareDialog::alertFailed(std::string message)
{
	QMessageBox* messageBox = new QMessageBox();
	messageBox->setIcon(QMessageBox::Warning);
	messageBox->setText(QString::fromStdString(message));
	messageBox->exec();
	delete messageBox;
}