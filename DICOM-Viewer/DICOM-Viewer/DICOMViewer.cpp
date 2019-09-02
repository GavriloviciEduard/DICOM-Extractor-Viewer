#include "DICOMViewer.h"
#include <fstream>

#define SPACE "  "

DICOMViewer::DICOMViewer(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableWidget->verticalHeader()->setDefaultSectionSize(15);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.buttonDelete->setEnabled(false);
	ui.buttonEdit->setEnabled(false);
	ui.buttonInsert->setEnabled(false);
	ui.tableWidget->horizontalHeader()->setStyleSheet("QHeaderView { font-weight: 2000; }");
	ui.tableWidget->horizontalHeader()->setHighlightSections(false);
	QHeaderView *verticalHeader = ui.tableWidget->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(10);
}

//========================================================================================================================
void replace(std::string& str, const std::string& from, const std::string& to)
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
void DICOMViewer::fileTriggered(QAction* qaction)
{
	const QString option = qaction->text();

	if (option == "Open")
	{
		const QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"), tr(""), tr("DICOM File (*.dcm)"));

		if (!fileName.isEmpty())
		{
			if (file.loadFile(fileName.toStdString().c_str()).good())
			{
				ui.tableWidget->scrollToTop();
				this->clearTable();
				this->extractData(file);
				ui.tableWidget->resizeColumnsToContents();
				this->setWindowTitle("PixelData DICOM Editor - " + fileName);
				ui.buttonInsert->setEnabled(true);
				ui.buttonClose->setEnabled(true);
				std::string nr = std::to_string(getFileSize(fileName.toStdString()));
				precision(nr, 2);
				ui.label->setText("Size: " + QString::fromStdString(nr) + " MB");
			}
			else
			{
				alertFailed("Failed to open file!");
			}
		}
	}

	else if(option == "Close")
	{
		this->clearTable();
		
	}

	else if (option == "Compare")
	{
		this->dialog = new CompareDialog(nullptr);
		dialog->show();
	}

	else if (option == "Save as")
	{
		const QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),tr(""), tr("DICOM File (*.dcm)"));
		if (!fileName.isEmpty())
		{
			if (!file.saveFile(fileName.toStdString().c_str()).good())
			{
				alertFailed("Failed to save!");
			}
		}
	}
}

//========================================================================================================================
void DICOMViewer::extractData(DcmFileFormat file)
{
	DcmMetaInfo* metaInfo = file.getMetaInfo();
	DcmDataset* dataSet = file.getDataset();

	for (unsigned long i = 0; i < metaInfo->card(); i++)
	{
		this->insertInTable(metaInfo->getElement(i));
	}

	for (unsigned long i = 0; i < dataSet->card(); i++)
	{
		this->insertInTable(dataSet->getElement(i));
	}

}

//========================================================================================================================
void DICOMViewer::insertInTable(DcmElement* element)
{
	this->depthRE = 0;
	this->getNestedSequences(DcmTagKey(
		OFstatic_cast(Uint16, element->getGTag()),
		OFstatic_cast(Uint16, element->getETag())),nullptr);

	if (!this->nestedElements.empty())
	{
		for (auto widget_element : this->nestedElements)
		{
			indent(widget_element, widget_element.getDepth());
			this->insert(widget_element, globalIndex);
			DcmWidgetElement copyElement = DcmWidgetElement(widget_element);
			copyElement.setTableIndex(globalIndex);
			this->elements.push_back(copyElement);
			this->globalIndex++;

		}

		this->nestedElements.clear();
	}
	
	else
	{
		DcmWidgetElement widgetElement = createElement(element,nullptr,nullptr);
		widgetElement.setItemTag(widgetElement.getItemTag().toUpper());
		this->insert(widgetElement, globalIndex);
		DcmWidgetElement copyElement = DcmWidgetElement(widgetElement);
		copyElement.setTableIndex(globalIndex);
		this->elements.push_back(copyElement);
		this->globalIndex++;
	}
}

//========================================================================================================================
void DICOMViewer::repopulate(std::vector<DcmWidgetElement> source) const
{
	ui.tableWidget->clearContents();
	for (int i = ui.tableWidget->rowCount(); i >= 0; i--)
	{
		ui.tableWidget->removeRow(i);
	}

	for (unsigned long i = 0; i < source.size(); i++)
	{
		const DcmWidgetElement element = DcmWidgetElement(source[i]);
		
		this->insert(element, i);
	}
	
}

//========================================================================================================================
void DICOMViewer::getNestedSequences(const DcmTagKey& tag, DcmSequenceOfItems* sequence)
{
	OFCondition cond = OFCondition(EC_CorruptedData);
	
	if (tag.hasValidGroup() && tag.getGroup() != 0x7FE0)
	{
		cond = file.getDataset()->findAndGetSequence(tag, sequence, true);
	}

	else
	{
		DcmElement *elem;

		if (file.getDataset()->findAndGetElement(DCM_PixelData, elem, OFTrue).good())
		{
			DcmPixelData *dpix = OFstatic_cast(DcmPixelData*, elem);
			E_TransferSyntax xfer = EXS_Unknown;
			const DcmRepresentationParameter *param = nullptr;
			dpix->getOriginalRepresentationKey(xfer, param);
			DcmPixelSequence *pixSeq = nullptr;

			if (dpix->getEncapsulatedRepresentation(xfer, param, pixSeq).good() && (pixSeq != nullptr))
			{
				DcmWidgetElement widgetElement = createElement(elem, nullptr, nullptr);
				widgetElement.setItemTag(widgetElement.getItemTag().toUpper());
				this->nestedElements.push_back(widgetElement);

				for (unsigned long i = 0; i < pixSeq->card(); ++i)
				{
					DcmPixelItem* item;
					pixSeq->getItem(item, i);
					auto* newItem = reinterpret_cast<DcmItem*>(item);
					DcmWidgetElement widgetElemen = createElement(nullptr, nullptr, newItem);
					widgetElemen.setItemTag(widgetElemen.getItemTag().toUpper());
					widgetElemen.setDepth(1);
					widgetElemen.setVR(widgetElement.getItemVR().toUpper());
					widgetElemen.setValue("Not Loaded");
					this->nestedElements.push_back(widgetElemen);

				}

				DcmWidgetElement widgetElementDelim = DcmWidgetElement(
					QString("(FFFE,E00D)"),
					QString(""), QString("0"),
					QString("0"),
					QString("SequenceDelimitationItem"),
					QString(""));
				widgetElementDelim.setDepth(0);
				this->nestedElements.push_back(widgetElementDelim);

				return;
			}
		}
	}

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
			this->iterateItem(sequence->getItem(i), this->depthRE);
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
void DICOMViewer::iterateItem(DcmItem * item,int& depth)
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

		const DcmTagKey NULLkey;
		DcmSequenceOfItems* sequence;
		item->getElement(i)->getParentItem()->findAndGetSequence(item->getElement(i)->getTag().getBaseTag(), sequence, true);
		this->getNestedSequences(NULLkey, sequence);
	}
}

//========================================================================================================================
void DICOMViewer::clearTable()
{
	if (ui.tableWidget->rowCount())
	{
		this->nestedElements.clear();
		this->elements.clear();
		this->globalIndex = 0;


		QList<QTableWidgetItem *> list = ui.tableWidget->selectedItems();
		
		if (!list.empty())
		{
			QTableWidgetItem *foundItem = list[0];
			this->scrollPosition = ui.tableWidget->model()->index(foundItem->row(), foundItem->column());
		}


		for (int i = ui.tableWidget->rowCount(); i >= 0; i--)
		{
			ui.tableWidget->removeRow(i);
		}

		ui.lineEdit->clear();
		this->setWindowTitle("PixelData DICOM Editor");
	}
}

//========================================================================================================================
void DICOMViewer::alertFailed(const std::string& message)
{
	auto* messageBox = new QMessageBox();
	messageBox->setIcon(QMessageBox::Warning);
	messageBox->setText(QString::fromStdString(message));
	messageBox->exec();
	delete messageBox;
}

//========================================================================================================================
void DICOMViewer::indent(DcmWidgetElement & element, int depth)
{
	if (depth == -1)
		return;
	
	QString str = element.getItemTag();

	while (depth--)
	{
		str.insert(0, SPACE);
	}

	element.setItemTag(str);
}

//========================================================================================================================
DcmWidgetElement DICOMViewer::createElement(DcmElement* element, DcmSequenceOfItems* sequence, DcmItem* item) const
{
	if (element)
	{
		DcmTagKey tagKey = DcmTagKey(
			OFstatic_cast(Uint16, element->getGTag()), 
			OFstatic_cast(Uint16, element->getETag()));
		DcmTag tagName = DcmTag(tagKey);
		DcmVR vr = DcmVR(element->getVR());
		std::string finalString;

		if(tagKey != DCM_PixelData && element->getLengthField()<=50)
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

	
		if (strcmp(vr.getVRName(), "??")==0 )
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

		if (tagKey != DCM_PixelData && sequence->getLengthField()<= 50)
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
void DICOMViewer::insert(DcmWidgetElement element, unsigned long &index) const
{
	ui.tableWidget->insertRow(index);
	ui.tableWidget->setItem(index, 0, new QTableWidgetItem(element.getItemTag()));
	ui.tableWidget->setItem(index, 1, new QTableWidgetItem(element.getItemVR()));
	ui.tableWidget->setItem(index, 2, new QTableWidgetItem(element.getItemVM()));
	ui.tableWidget->setItem(index, 3, new QTableWidgetItem(element.getItemLength()));
	ui.tableWidget->setItem(index, 4, new QTableWidgetItem(element.getItemDescription()));
	ui.tableWidget->setItem(index, 5, new QTableWidgetItem(element.getItemValue()));
}

//========================================================================================================================
double DICOMViewer::getFileSize(const std::string& fileName)
{
	std::ifstream in_file(fileName, std::ios::binary | std::ios::ate);
	double size = in_file.tellg() / 1024;
	size /= 1024;
	in_file.close();
	return size;
}

//========================================================================================================================
void DICOMViewer::getTagKeyOfSequence(const int row, DcmTagKey* returnKey, int* numberInSequence) const
{
	*numberInSequence = 0;

	for (int i = row - 1; i >= 0; i--)
	{
		DcmWidgetElement element = DcmWidgetElement(
			ui.tableWidget->item(i, 0)->text(),
			ui.tableWidget->item(i, 1)->text(),
			ui.tableWidget->item(i, 2)->text(),
			ui.tableWidget->item(i, 3)->text(),
			ui.tableWidget->item(i, 4)->text(),
			ui.tableWidget->item(i, 5)->text());

		if (element.getItemVR() == "na" && element.getItemDescription() == "Item")
		{
			(*numberInSequence)++;
		}

		if (element.getItemVR() == "SQ")
		{
			*returnKey = element.extractTagKey();
			break;
		}
	}
}

//========================================================================================================================
void DICOMViewer::findText()
{
	const QString text = ui.lineEdit->text();

	if (!text.isEmpty())
	{
		if (!this->elements.empty())
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

//========================================================================================================================
void DICOMViewer::tableClicked(int row, int collumn)
{
	QList<QTableWidgetItem*> selectedTags = ui.tableWidget->selectedItems();

	DcmWidgetElement element = DcmWidgetElement(selectedTags[0]->text().trimmed(), selectedTags[1]->text(),
		selectedTags[2]->text(), selectedTags[3]->text(), selectedTags[4]->text(), selectedTags[5]->text());

	if (!selectedTags.empty())
	{
		for (auto elementV : this->elements)
		{
			if ( (element == elementV && !shouldModify(element)) )
			{

				ui.buttonEdit->setEnabled(false);
				ui.buttonDelete->setEnabled(false);
				ui.buttonInsert->setEnabled(false);
				return;
			}

			if (element == elementV && (elementV.getItemVR() == "SQ" || elementV.getItemVR() == "na"))
			{
				ui.buttonEdit->setEnabled(false);
				ui.buttonDelete->setEnabled(true);
				ui.buttonInsert->setEnabled(true);
				return;
			}
		
			if (element == elementV && elementV.getItemVR() != "SQ" && elementV.getItemVR() != "na" && elementV.getDepth() != -1 && elementV.getItemVR() != "OB")
			{
				ui.buttonInsert->setEnabled(false);
				ui.buttonEdit->setEnabled(true);
				ui.buttonDelete->setEnabled(true);
				return;
			}

			if(element == elementV && elementV.getItemVR() == "OB")
			{
				ui.buttonInsert->setEnabled(false);
				ui.buttonEdit->setEnabled(false);
				ui.buttonDelete->setEnabled(false);
				return;
			}

		}
	}

	ui.buttonEdit->setEnabled(true);
	ui.buttonDelete->setEnabled(true);
	ui.buttonInsert->setEnabled(true);
}

//========================================================================================================================
void DICOMViewer::closeButtonClicked()
{
	this->close();
}

//========================================================================================================================
void DICOMViewer::editClicked()
{
	QList<QTableWidgetItem*> items = ui.tableWidget->selectedItems();

	if (!items.empty())
	{
		const DcmWidgetElement elementWidget =  DcmWidgetElement(items[0]->text(), items[1]->text(), items[2]->text(), items[3]->text(), items[4]->text(), items[5]->text());
		this->createSimpleEditDialog(elementWidget);
		ui.tableWidget->scrollTo(this->scrollPosition, QAbstractItemView::PositionAtCenter);
		
		ui.tableWidget->selectRow(this->scrollPosition.row());

	}
}

//========================================================================================================================
bool DICOMViewer::deleteElementFromFile(DcmSequenceOfItems* sequence, DcmWidgetElement element, QList<DcmWidgetElement> list)
{
	int count = -1;
	int i = list.size() - 1;

	while (i >= 0 && list[i].getItemVR() == "na")
	{
		count++;
		i--;
		list.removeLast();
	}

	if (list.empty())
	{
		count--;

		if (!sequence->getParentItem()->findAndDeleteSequenceItem(sequence->getTag().getBaseTag(), count).good())
		{
			alertFailed("Failed!");
			return false;
		}

		return true;
	}

	DcmItem* item = sequence->getItem(count);

	if (list[i] == element)
	{
		if (!item->findAndDeleteElement(element.extractTagKey(), false, false).good())
		{
			alertFailed("Failed!");
			return false;
		}

		else
		{
			return true;
		}
	}

	else
	{
		DcmSequenceOfItems* seq;

		if (item->findAndGetSequence(list[i].extractTagKey(), seq, false, false).good())
		{
			list.removeLast();
			return deleteElementFromFile(seq, element, list);
		}

		else
		{
			alertFailed("Failed!");
			return false;
		}
	}
}

//========================================================================================================================
bool DICOMViewer::modifyValue(DcmSequenceOfItems* sequence, DcmWidgetElement element, QList<DcmWidgetElement> list, const QString& value)
{
	int count = -1;
	int i = list.size() - 1;

	while (list[i].getItemVR() == "na")
	{
		count++;
		i--;
		list.removeLast();
	}

	DcmItem* item = sequence->getItem(count);

	if (list[i] == element)
	{
		DcmElement* el;

		if (item->findAndGetElement(element.extractTagKey(), el, false, false).good())
		{
			if (el->putString(value.toStdString().c_str()).good())
			{
				return true;
			}

			else
			{
				alertFailed("Failed!");
				return false;
			}
		}

		else
		{
			alertFailed("Failed!");
			return false;
		}
	}

	else
	{
		DcmSequenceOfItems* seq;

		if (item->findAndGetSequence(list[i].extractTagKey(), seq, false, false).good())
		{
			list.removeLast();
			return modifyValue(seq, element, list, value);
		}

		else
		{
			alertFailed("Failed!");
			return false;
		}
	}
}

//========================================================================================================================
bool DICOMViewer::insertElement(DcmSequenceOfItems * sequence, DcmWidgetElement element, DcmWidgetElement insertElement, QList<DcmWidgetElement> list) const
{
	int count = -1;
	int i = list.size() - 1;

	while (i >= 0 && list[i].getItemVR() == "na")
	{
		count++;
		i--;
		list.removeLast();
	}

	if (list.empty())
	{
		count--;
		DcmItem* item = sequence->getItem(count);

		if (!item->putAndInsertString(insertElement.extractTagKey(), insertElement.getItemValue().toStdString().c_str(), false).good())
		{
			alertFailed("Failed!");
			return false;
		}

		return true;
	}

	DcmItem* item = sequence->getItem(count);

	if (list[i] == element)
	{
		if (insertElement.getItemVR() == "na")
		{
			DcmItem* it = new DcmItem(DcmTag(insertElement.extractTagKey()));
			if (!item->insertSequenceItem(element.extractTagKey(), it).good())
			{
				alertFailed("Failed!");
				return false;
			}

			return true;
		}

		else
		{
			if (!item->putAndInsertString(insertElement.extractTagKey(), insertElement.getItemValue().toStdString().c_str(), false).good())
			{
				alertFailed("Failed!");
				return false;
			}

			return true;
		}
	}

	else
	{
		DcmSequenceOfItems* seq;

		if (item->findAndGetSequence(list[i].extractTagKey(), seq, false, false).good())
		{
			list.removeLast();
			return this->insertElement(seq, element, insertElement, list);
		}

		else
		{
			alertFailed("Failed");
			return false;
		}
	}
}

//========================================================================================================================
void DICOMViewer::createSimpleEditDialog(DcmWidgetElement element)
{
	element.calculateDepthFromTag();
	auto* editDialog = new EditDialogSimple(nullptr);
	editDialog->setValue(element.getItemValue());
	editDialog->setWindowTitle(element.getItemTag());
	editDialog->setDescription(element.getItemDescription());
	editDialog->exec();

	element.calculateTableIndex(currentRow(element, ui.tableWidget->currentRow()), this->elements);

	QList<DcmWidgetElement> list;
	list.append(element);
	generatePathToRoot(element, element.getTableIndex(), &list);

	const QString result = editDialog->getValue();

	if (!result.isEmpty())
	{
		if (list.size() > 1)
		{
			DcmSequenceOfItems* sequence;

			if(file.getDataset()->findAndGetSequence(list[list.size() - 1].extractTagKey(),sequence,false,false).good())
			{
				list.removeLast();

				if (modifyValue(sequence, element, list,result))
				{
					clearTable();
					extractData(file);
				}
			}
		}

		else
		{
			DcmElement* el;

			if (file.getDataset()->findAndGetElement(element.extractTagKey(), el, false, false).good())
			{
				if (el->putString(result.toStdString().c_str()).good())
				{
					clearTable();
					extractData(file);
				}

				else
				{
					alertFailed("Failed!");
				}
			}

			else
			{
				alertFailed("Failed!");
			}
		}
	}

	delete editDialog;
}

//========================================================================================================================
void DICOMViewer::generatePathToRoot(DcmWidgetElement element, int row, QList<DcmWidgetElement> *elements)
{
	if (element.getDepth() == 0)
	{
		return;
	}

	while (row)
	{
		if (element.getDepth() == 0)
		{
			return;
		}

		DcmWidgetElement el = this->elements[row];
		el.calculateDepthFromTag();

		if (el.getItemVR() == "na" && el.getDepth() == element.getDepth())
		{
			(*elements).append(el);
		}

		if (el.getDepth() == element.getDepth() - 1)
		{
			(*elements).append(el);
			element = el;
		}
		--row;
	}
}

//========================================================================================================================
bool DICOMViewer::shouldModify(DcmWidgetElement element)
{

	bool contains = false;
	
	if (element.getItemDescription() == "PhotometricInterpretation" || element.getItemDescription() =="Rows" || element.getItemDescription()== "Columns" || 
		element.getItemDescription().toUpper().contains("PIXEL") || element.getItemDescription().toUpper().contains("FRAME") || element.getItemDescription().toUpper().contains("BIT") ||
		element.getItemDescription()=="ItemDelimitationItem" || element.getItemDescription()=="SequenceDelimitationItem")
		contains = true;

	const DcmTagKey tagKey = element.extractTagKey();

	if (tagKey.getElement() == 0 || tagKey.getGroup() == 0 || tagKey.getGroup() == 2 || tagKey.getGroup() == 1
		|| tagKey.getGroup() == 5 || tagKey.getGroup() == 3 || tagKey.getGroup() == 7 || tagKey.getGroup() == 0xffff || contains)
		return false;

	return true;
}

//========================================================================================================================
int DICOMViewer::currentRow(DcmWidgetElement element, const int& finalRow) const
{

	int index = 0;

	for (int i = 0; i <= finalRow; i++)
	{
		DcmWidgetElement innerElement = DcmWidgetElement(ui.tableWidget->item(i,0)->text(), ui.tableWidget->item(i, 1)->text(), 
			ui.tableWidget->item(i, 2)->text(), ui.tableWidget->item(i, 3)->text(), ui.tableWidget->item(i, 4)->text(), ui.tableWidget->item(i, 5)->text());

		if (element == innerElement)
			index++;
	}

	return index;

}

//========================================================================================================================
void DICOMViewer::precision(std::string & nr, const int & precision)
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
void DICOMViewer::findIndexInserted( DcmWidgetElement& element)
{
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		DcmWidgetElement elementWidget = DcmWidgetElement(
			ui.tableWidget->item(i, 0)->text(),
			ui.tableWidget->item(i, 1)->text(),
			ui.tableWidget->item(i, 2)->text(),
			ui.tableWidget->item(i, 3)->text(),
			ui.tableWidget->item(i, 4)->text(),
			ui.tableWidget->item(i, 5)->text());

		if (element.getItemTag().toUpper() == elementWidget.getItemTag().toUpper() && element.getItemDescription().toUpper() == elementWidget.getItemDescription().toUpper())
		{
			this->scrollPosition = ui.tableWidget->model()->index(i, 0);
			return;
		}
	}	
}

//========================================================================================================================
void DICOMViewer::deleteClicked()
{
	QList<QTableWidgetItem*> items = ui.tableWidget->selectedItems();
	DcmWidgetElement element = DcmWidgetElement(items[0]->text(), items[1]->text(), items[2]->text(), items[3]->text(), items[4]->text(), items[5]->text());
	element.calculateTableIndex(currentRow(element,ui.tableWidget->currentRow()),this->elements);
	element.calculateDepthFromTag();
	QList<DcmWidgetElement> list;
	list.append(element);
	generatePathToRoot(element, element.getTableIndex(), &list);

	if (list.size() > 1)
	{
		DcmSequenceOfItems* sequence;

		if (file.getDataset()->findAndGetSequence(list[list.size() - 1].extractTagKey(), sequence, false, false).good())
		{
			list.removeLast();

			if (deleteElementFromFile(sequence, element, list))
			{
				clearTable();
				extractData(file);
			}
		}
	}

	else
	{
		if (file.getDataset()->findAndDeleteElement(element.extractTagKey(), false, false).good())
		{
			clearTable();
			extractData(file);
		}
	}

	ui.tableWidget->scrollTo(this->scrollPosition, QAbstractItemView::PositionAtCenter);
	ui.tableWidget->selectRow(this->scrollPosition.row());

}

//========================================================================================================================
void DICOMViewer::insertClicked()
{
	auto* dialog = new TagSelectDialog(nullptr);
	dialog->populate();

	if (dialog->exec() == QDialog::Accepted)
	{
		DcmWidgetElement insertElement = dialog->getElement();
		QList<QTableWidgetItem*> items = ui.tableWidget->selectedItems();

		if (!items.empty())
		{
			DcmWidgetElement selectedElement = DcmWidgetElement(items[0]->text(), items[1]->text(), items[2]->text(), items[3]->text(), items[4]->text(), items[5]->text());
			selectedElement.calculateTableIndex(currentRow(selectedElement, ui.tableWidget->currentRow()), this->elements);
			QList<DcmWidgetElement> list;
			list.append(selectedElement);
			generatePathToRoot(selectedElement, selectedElement.getTableIndex(), &list);

			if (selectedElement.getItemVR() == "SQ" && insertElement.getItemVR() == "na")
			{
				DcmSequenceOfItems* sequence;

				if (file.getDataset()->findAndGetSequence(list[list.size() - 1].extractTagKey(), sequence, false, false).good())
				{
					if (list.size() == 1)
					{
						DcmItem* it = new DcmItem(DcmTag(insertElement.extractTagKey()));

						if (!file.getDataset()->insertSequenceItem(sequence->getTag().getBaseTag(), it).good())
						{
							alertFailed("Failed!");
						}

						else
						{
							clearTable();
							extractData(file);
							ui.tableWidget->scrollTo(this->scrollPosition, QAbstractItemView::PositionAtCenter);
							ui.tableWidget->selectRow(this->scrollPosition.row());
						}
					}

					else
					{
						list.removeLast();

						if (this->insertElement(sequence, selectedElement, insertElement, list))
						{
							clearTable();
							extractData(file);
							ui.tableWidget->scrollTo(this->scrollPosition, QAbstractItemView::PositionAtCenter);
							ui.tableWidget->selectRow(this->scrollPosition.row());
						}
					}
				}

			}

			else if (selectedElement.getItemVR() == "na")
			{
				DcmSequenceOfItems* sequence;

				if (file.getDataset()->findAndGetSequence(list[list.size() - 1].extractTagKey(), sequence, false, false).good())
				{
					list.removeLast();

					if (this->insertElement(sequence, selectedElement, insertElement, list))
					{
						clearTable();
						extractData(file);
						ui.tableWidget->scrollTo(this->scrollPosition, QAbstractItemView::PositionAtCenter);
						ui.tableWidget->selectRow(this->scrollPosition.row());

					}
				}

			}

			else if (selectedElement.getItemVR() == "SQ" && insertElement.getItemVR() != "na")
			{
				alertFailed("Can only insert items in sequence!");
			}

			else
			{
				if (!file.getDataset()->putAndInsertString(insertElement.extractTagKey(), insertElement.getItemValue().toStdString().c_str(), false).good())
				{
					alertFailed("Failed!");
				}

				else
				{
					clearTable();
					extractData(file);
					this->findIndexInserted(insertElement);
					ui.tableWidget->scrollTo(this->scrollPosition, QAbstractItemView::PositionAtCenter);
					ui.tableWidget->selectRow(this->scrollPosition.row());
				}
			}
		}

		else
		{
			if (insertElement.getItemVR() == "SQ")
			{
				if (!file.getDataset()->insertEmptyElement(insertElement.extractTagKey(), false).good())
				{
					alertFailed("Failed!");
				}
				else
				{
					clearTable();
					extractData(file);
				}
			}
			else if (!file.getDataset()->putAndInsertString(insertElement.extractTagKey(),insertElement.getItemValue().toStdString().c_str(),false).good())
			{
				alertFailed("Failed!");
			}
			else
			{
				clearTable();
				extractData(file);
				this->findIndexInserted(insertElement);
				ui.tableWidget->scrollTo(this->scrollPosition, QAbstractItemView::PositionAtCenter);
				ui.tableWidget->selectRow(this->scrollPosition.row());
			}
		}
	}

	delete dialog;
}
