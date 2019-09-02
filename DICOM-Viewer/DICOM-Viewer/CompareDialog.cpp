#include "CompareDialog.h"

#define SPACE "  "

CompareDialog::CompareDialog(QWidget * parent)
{
	ui.setupUi(this);
	ui.tableWidget1->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableWidget1->verticalHeader()->setDefaultSectionSize(14);
	ui.tableWidget1->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget1->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget1->setColumnWidth(2, ui.tableWidget1->columnWidth(2) * 3);
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	QHeaderView *verticalHeader = ui.tableWidget1->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(10);
}

//========================================================================================================================
void CompareDialog::replace(std::string & str, const std::string & from, const std::string & to)
{
	if (from.empty())
		return;

	size_t start_pos = 0;

	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

//========================================================================================================================
void CompareDialog::loadFile(DcmFileFormat* file, const QString& fileName, bool first)
{
	if (file->loadFile(fileName.toStdString().c_str()).good())
	{
		ui.tableWidget1->scrollToTop();
		extractData(file);
	}
	
	else
	{
		alertFailed("Failed to open file!");
	}
}

//========================================================================================================================
void CompareDialog::extractData(DcmFileFormat* file)
{
	DcmMetaInfo* metaInfo = file->getMetaInfo();
	DcmDataset* dataSet = file->getDataset();

	for (unsigned long i = 0; i < metaInfo->card(); i++)
	{
		insertInTable(metaInfo->getElement(i),file);
	}

	for (unsigned long i = 0; i < dataSet->card(); i++)
	{
		insertInTable(dataSet->getElement(i),file);
	}

	if (loaded >= 2)
	{
		merge();
		populateTableElementsVector();
	}

	globalIndex = 0;
}

//========================================================================================================================
void CompareDialog::insertInTable(DcmElement* element, DcmFileFormat* file)
{
	this->depthRE = 0;
	this->getNestedSequences(element->getTag().getBaseTag(),nullptr,file);

	if (!this->nestedElements.empty())
	{
		for (auto widget_element : this->nestedElements)
		{
			indent(widget_element, widget_element.getDepth());
			DcmWidgetElement copyElement = DcmWidgetElement(widget_element);
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
void CompareDialog::getNestedSequences(const DcmTagKey& tag, DcmSequenceOfItems* sequence, DcmFileFormat* file)
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

		for (unsigned long i = 0; i < sequence->card(); i++)
		{
			DcmWidgetElement widgetElement2 = createElement(nullptr, nullptr, sequence->getItem(i));
			widgetElement2.setDepth(this->depthRE);
			widgetElement2.setItemTag(widgetElement2.getItemTag().toUpper());
			this->nestedElements.push_back(widgetElement2);
			this->iterateItem(sequence->getItem(i), this->depthRE, file);
			DcmWidgetElement widgetElementDelim = DcmWidgetElement(
				QString("(FFFE,E00D)"),
				QString(""), QString("0"),
				QString("0"),
				QString("ItemDelimitationItem"),
				QString(""));
			this->depthRE--;
			widgetElementDelim.setDepth(this->depthRE);
			this->nestedElements.push_back(widgetElementDelim);
		}

		DcmWidgetElement widgetElementDelim = DcmWidgetElement(
			QString("(FFFE,E0DD)"),
			QString(""),
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
	if(depth ==-1)
		return;
	
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

	for (unsigned long i = 0; i < item->getNumberOfValues(); i++)
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
void CompareDialog::insertRight(DcmWidgetElement element, int & index) const
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
void CompareDialog::insert(DcmWidgetElement element, int & index) const
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
DcmWidgetElement CompareDialog::createElement(DcmElement * element, DcmSequenceOfItems * sequence, DcmItem * item) const
{
	if (element)
	{
		DcmTagKey tagKey = DcmTagKey(
			OFstatic_cast(Uint16, element->getGTag()),
			OFstatic_cast(Uint16, element->getETag()));
		DcmTag tagName = DcmTag(tagKey);
		DcmVR vr = DcmVR(element->getVR());
		std::string finalString;

		if (tagKey != DCM_PixelData && element->getLengthField() <= 50)
		{
			OFString value;
			element->getOFStringArray(value, true);
			finalString = value.c_str();
			replace(finalString, "\\", " ");
		}

		else
		{
			for (int i = 0; i < 10; i++)
			{
				OFString value;
				element->getOFString(value, i, false);
				finalString.append(value.c_str());
				finalString.append(" ");
			}
		}


		if (strcmp(vr.getVRName(), "??") == 0)
		{
			DcmWidgetElement widgetElement = DcmWidgetElement(
				QString::fromStdString(tagKey.toString().c_str()),
				QString::fromStdString(""),
				QString::fromStdString(std::to_string(element->getVM())),
				QString::fromStdString(std::to_string(element->getLength())),
				"",
				QString::fromStdString(finalString));

			return widgetElement;
		}

		DcmWidgetElement widgetElement = DcmWidgetElement(
			QString::fromStdString(tagKey.toString().c_str()),
			QString::fromStdString(vr.getVRName()),
			QString::fromStdString(std::to_string(element->getVM())),
			QString::fromStdString(std::to_string(element->getLength())),
			tagName.getTagName(),
			QString::fromStdString(finalString));


		return widgetElement;
	}

	if (sequence)
	{
		DcmTagKey tagKey = DcmTagKey(
			OFstatic_cast(Uint16, sequence->getGTag()),
			OFstatic_cast(Uint16, sequence->getETag()));
		DcmTag tagName = DcmTag(tagKey);
		DcmVR vr = DcmVR(sequence->getVR());
		std::string finalString;

		if (tagKey != DCM_PixelData && sequence->getLengthField() <= 50)
		{
			OFString value;
			sequence->getOFStringArray(value, true);
			finalString = value.c_str();
			replace(finalString, "\\", " ");
		}


		else
		{
			for (int i = 0; i < 10; i++)
			{
				OFString value;
				sequence->getOFString(value, i, false);
				finalString.append(value.c_str());
				finalString.append(" ");
			}

		}

		if (strcmp(vr.getVRName(), "??") == 0)
		{
			DcmWidgetElement widgetElement = DcmWidgetElement(
				QString::fromStdString(tagKey.toString().c_str()),
				QString::fromStdString(""),
				QString::fromStdString(std::to_string(sequence->getVM())),
				QString::fromStdString(std::to_string(sequence->getLength())),
				"",
				QString::fromStdString(finalString));

			return widgetElement;
		}

		DcmWidgetElement widgetElement = DcmWidgetElement(
			QString::fromStdString(tagKey.toString().c_str()),
			QString::fromStdString(vr.getVRName()),
			QString::fromStdString(std::to_string(sequence->getVM())),
			QString::fromStdString(std::to_string(sequence->getLength())),
			tagName.getTagName(),
			QString::fromStdString(finalString));


		return widgetElement;
	}

	if (item)
	{
		DcmTagKey tagKey = DcmTagKey(
			OFstatic_cast(Uint16, item->getGTag()),
			OFstatic_cast(Uint16, item->getETag()));
		DcmTag tagName = DcmTag(tagKey);
		DcmVR vr = DcmVR(item->getVR());


		if (strcmp(vr.getVRName(), "??") == 0)
		{
			DcmWidgetElement widgetElement = DcmWidgetElement(
				QString::fromStdString(tagKey.toString().c_str()),
				QString::fromStdString(""),
				QString::fromStdString(std::to_string(item->getVM())),
				QString::fromStdString(std::to_string(item->getLength())),
				"",
				QString::fromStdString(""));

			return widgetElement;
		}

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
void CompareDialog::insertBoth(DcmWidgetElement el1, DcmWidgetElement el2, int &index, const int status) const
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
void CompareDialog::insertSequence(const int status, int& index1, int& index2)
{
	if (status == 1)
	{
		insertBoth(this->elements1[index1], this->elements2[index2] , globalIndex, status);
		index1++;
		index2++;
		globalIndex++;

		while (this->elements1[index1].getItemDescription() == "Item" && this->elements2[index2].getItemDescription() == "Item")
		{
			const int item1Depth = this->elements1[index1].getDepth();
			const int item2Depth = this->elements2[index2].getDepth();
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
			const int itemDepth = this->elements1[index1].getDepth();
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

		if (index1 < static_cast<int>(this->elements1.size()) && isDelimitation(this->elements1[index1]))
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
			const int itemDepth = this->elements2[index2].getDepth();
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

	
		if (isDelimitation(this->elements2[index2]))
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


		while (index1 < static_cast<int>(elements1.size()) - 1 && index2 < static_cast<int>( elements2.size()) - 1)
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
					const DcmWidgetElement empty = DcmWidgetElement(elements2[index2].getItemTag(), "", "", "", "", "");
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
					const DcmWidgetElement empty = DcmWidgetElement(elements1[index1].getItemTag(), "", "", "", "", "");
					insertBoth(elements1[index1], empty, globalIndex, 3);
					index1++;
					globalIndex++;
				}
			}
		}

		while (index1 < static_cast<int>(elements1.size()) - 1)
		{
			if (elements1[index1].getItemVR() == "SQ")
			{
				insertSequence(2, index1, index2);
				index1++;
			}

			else
			{
				const DcmWidgetElement empty = DcmWidgetElement(elements1[index1].getItemTag(), "", "", "", "", "");
				insertBoth(elements1[index1],empty, globalIndex, 3);
				index1++;
				globalIndex++;
			}
		}

		while (index2 < static_cast<int>(elements2.size()) -1)
		{
			if (elements2[index2].getItemVR() == "SQ")
			{
				insertSequence(3, index1, index2);
				index2++;
			}

			else
			{
				const DcmWidgetElement empty = DcmWidgetElement(elements2[index2].getItemTag(), "", "", "", "", "");
				insertBoth(empty, elements2[index2], globalIndex, 2);
				index2++;
				globalIndex++;
			}
		}

		insertBoth(elements1[elements1.size() - 1], elements2[elements2.size() - 1],globalIndex, 1);
	}
}

//========================================================================================================================
void CompareDialog::clearTable() const
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
	const QString fileName = QFileDialog::getOpenFileName(this);

	if (!fileName.isEmpty())
	{
		clearTable();
		firstFile = true;
		elements1.clear();
		loaded++;
		this->loadFile(&file1, fileName, true);
		std::string nr = std::to_string(getFileSize(fileName.toStdString()));
		precision(nr, 2);
		ui.labelSize1->setText("Size File 1: " + QString::fromStdString(nr) + " MB");
		ui.labelPath1->setText("Path File 1: " + fileName);
	}
}

//========================================================================================================================
void CompareDialog::loadFile2()
{
	const QString fileName = QFileDialog::getOpenFileName(this);

	if (!fileName.isEmpty())
	{
		clearTable();
		firstFile = false;
		elements2.clear();
		loaded++;
		this->loadFile(&file2, fileName, false);
		std::string nr = std::to_string(getFileSize(fileName.toStdString()));
		precision(nr, 2);
		ui.labelSize2->setText("Size File 2: " + QString::fromStdString(nr) + " MB");
		ui.labelPath2->setText("Path File 2: " + fileName);
	}
}

//========================================================================================================================
void CompareDialog::alertFailed(const std::string& message)
{
	auto* messageBox = new QMessageBox();
	messageBox->setIcon(QMessageBox::Warning);
	messageBox->setText(QString::fromStdString(message));
	messageBox->exec();
	delete messageBox;
}

//========================================================================================================================
void CompareDialog::populateTableElementsVector()
{
	this->tableElements.clear();
	for (int i = 0; i < ui.tableWidget1->rowCount(); i++)
	{
		DcmWidgetElement el1 = DcmWidgetElement(ui.tableWidget1->item(i, 0)->text(), "", "", ui.tableWidget1->item(i, 1)->text(), "", ui.tableWidget1->item(i, 2)->text());
		DcmWidgetElement el2 = DcmWidgetElement(ui.tableWidget1->item(i, 0)->text(), "", "", ui.tableWidget1->item(i, 3)->text(), "", ui.tableWidget1->item(i, 4)->text());
		std::tuple<DcmWidgetElement, DcmWidgetElement> tuple = std::tuple<DcmWidgetElement, DcmWidgetElement>(el1, el2);
		this->tableElements.push_back(tuple);
	}
}

//========================================================================================================================
void CompareDialog::precision(std::string & nr, const int & precision)
{
	int len = 0;
	bool ok = false;

	for (auto l : nr)
	{
		len++;
		if (l == '.')
		{
			ok = true;
			len += 2;
		}
		if (ok)
		{
			break;
		}
	}
	nr.resize(len);
}

//========================================================================================================================
double CompareDialog::getFileSize(const std::string& fileName)
{
	std::ifstream in_file(fileName, std::ios::binary | std::ios::ate);
	double size = in_file.tellg() / 1024;
	size /= 1024;
	in_file.close();
	return size;
}

//========================================================================================================================
void CompareDialog::findText()
{
	const QString text = ui.lineSearch->text();
	if (!text.isEmpty())
	{
		if (!this->tableElements.empty())
		{
			std::vector<std::tuple<DcmWidgetElement, DcmWidgetElement>> result;
			for (auto tuple : this->tableElements)
			{
				if (std::get<0>(tuple).checkIfContains(text) || std::get<1>(tuple).checkIfContains(text))
				{
					result.push_back(tuple);
				}
			}

			clearTable();
			for (unsigned long i = 0; i < result.size(); i++)
			{
				ui.tableWidget1->insertRow(i);
				ui.tableWidget1->setItem(i, 0, new QTableWidgetItem(std::get<0>(result[i]).getItemTag()));
				ui.tableWidget1->setItem(i, 1, new QTableWidgetItem(std::get<0>(result[i]).getItemLength()));
				ui.tableWidget1->setItem(i, 2, new QTableWidgetItem(std::get<0>(result[i]).getItemValue()));
				ui.tableWidget1->setItem(i, 3, new QTableWidgetItem(std::get<1>(result[i]).getItemLength()));
				ui.tableWidget1->setItem(i, 4, new QTableWidgetItem(std::get<1>(result[i]).getItemValue()));

				if (std::get<0>(result[i]).getItemValue() == "")
				{
					ui.tableWidget1->item(i, 0)->setBackgroundColor(QColor(220, 220, 220));
					ui.tableWidget1->item(i, 3)->setBackgroundColor(QColor(104, 223, 240));
					ui.tableWidget1->item(i, 4)->setBackgroundColor(QColor(104, 223, 240));
				}
				else if (std::get<1>(result[i]).getItemValue() == "")
				{
					ui.tableWidget1->item(i, 0)->setBackgroundColor(QColor(220, 220, 220));
					ui.tableWidget1->item(i, 1)->setBackgroundColor(QColor(0, 250, 154));
					ui.tableWidget1->item(i, 2)->setBackgroundColor(QColor(0, 250, 154));
				}
				else if (!(std::get<0>(result[i]) == std::get<1>(result[i])))
				{
					ui.tableWidget1->item(i, 0)->setBackgroundColor(QColor(220, 220, 220));
					ui.tableWidget1->item(i, 1)->setBackgroundColor(QColor(250, 128, 114));
					ui.tableWidget1->item(i, 2)->setBackgroundColor(QColor(250, 128, 114));
					ui.tableWidget1->item(i, 3)->setBackgroundColor(QColor(250, 128, 114));
					ui.tableWidget1->item(i, 4)->setBackgroundColor(QColor(250, 128, 114));
				}
			}
		}
	}
	else
	{
		clearTable();
		merge();
	}
}