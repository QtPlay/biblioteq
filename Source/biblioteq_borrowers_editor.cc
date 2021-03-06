#include "biblioteq_borrowers_editor.h"

extern biblioteq *qmain;

/*
** -- biblioteq_borrowers_editor() --
*/

biblioteq_borrowers_editor::biblioteq_borrowers_editor
(QWidget *parent,
 biblioteq_item *bitemArg,
 const int quantityArg,
 const QString &ioidArg,
 const QString &uniqueidArg,
 const QFont &font,
 const QString &itemTypeArg,
 const int stateArg):QDialog(parent)
{
  setWindowModality(Qt::WindowModal);
  m_bd.setupUi(this);
#ifdef Q_OS_MAC
#if QT_VERSION < 0x050000
  setAttribute(Qt::WA_MacMetalStyle, BIBLIOTEQ_WA_MACMETALSTYLE);
#endif
#endif
  m_ioid = ioidArg;
  m_quantity = quantityArg;
  m_bitem = bitemArg;
  m_state = stateArg;
  m_itemType = itemTypeArg;

  /*
  ** Override the state, if necessary.
  */

  if(qmain->getRoles().contains("administrator") ||
     qmain->getRoles().contains("circulation"))
    m_state = biblioteq::EDITABLE;
  else if(qmain->getRoles().contains("librarian"))
    m_state = biblioteq::VIEW_ONLY;

  if(m_state == biblioteq::EDITABLE)
    {
      connect(m_bd.saveButton, SIGNAL(clicked(void)), this,
	      SLOT(slotSave(void)));
      connect(m_bd.eraseButton, SIGNAL(clicked(void)), this,
	      SLOT(slotEraseBorrower(void)));
    }
  else
    {
      m_bd.saveButton->setVisible(false);
      m_bd.eraseButton->setVisible(false);
    }

  connect(m_bd.cancelButton, SIGNAL(clicked(void)), this,
	  SLOT(slotCloseCurrentBorrowers(void)));
#if QT_VERSION >= 0x050000
  m_bd.table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
  m_bd.table->verticalHeader()->setResizeMode(QHeaderView::Fixed);
#endif
  setGlobalFonts(font);

  if(!uniqueidArg.isEmpty())
    setWindowTitle
      (QString(tr("BiblioteQ: Item Reservation Status (")) + uniqueidArg +
       QString(tr(")")));
  else
    setWindowTitle(tr("BiblioteQ: Item Reservation Status"));
}

/*
** -- ~biblioteq_borrowers_editor() --
*/

biblioteq_borrowers_editor::~biblioteq_borrowers_editor()
{
}

/*
** -- showUsers() --
*/

void biblioteq_borrowers_editor::showUsers(void)
{
  QDateEdit *dateEdit = 0;
  QProgressDialog progress1(this);
  QProgressDialog progress2(this);
  QSqlQuery query(qmain->getDB());
  QString str = "";
  QString tmpStr = "";
  QStringList list;
  QTableWidgetItem *item = 0;
  bool terminate = false;
  int i = 0;
  int j = 0;
  int row = 0;

#ifdef Q_OS_MAC
#if QT_VERSION < 0x050000
  progress1.setAttribute(Qt::WA_MacMetalStyle, BIBLIOTEQ_WA_MACMETALSTYLE);
  progress2.setAttribute(Qt::WA_MacMetalStyle, BIBLIOTEQ_WA_MACMETALSTYLE);
#endif
#endif
  m_bd.table->setCurrentItem(0);
  m_bd.table->setColumnCount(0);
  m_bd.table->setRowCount(0);
  m_columnHeaderIndexes.clear();

  if(m_state == biblioteq::EDITABLE)
    {
      list.append(tr("Copy Number"));
      list.append(tr("Barcode"));
      list.append(tr("Member ID"));
      list.append(tr("First Name"));
      list.append(tr("Last Name"));
      list.append(tr("Reservation Date"));
      list.append(tr("Copy Due Date"));
      list.append(tr("Lender"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Copy Number");
      m_columnHeaderIndexes.append("Barcode");
      m_columnHeaderIndexes.append("Member ID");
      m_columnHeaderIndexes.append("First Name");
      m_columnHeaderIndexes.append("Last Name");
      m_columnHeaderIndexes.append("Reservation Date");
      m_columnHeaderIndexes.append("Copy Due Date");
      m_columnHeaderIndexes.append("Lender");
      m_columnHeaderIndexes.append("MYOID");
    }
  else
    {
      list.append(tr("Copy Number"));
      list.append(tr("Barcode"));
      list.append(tr("Reservation Date"));
      list.append(tr("Copy Due Date"));
      m_columnHeaderIndexes.append("Copy Number");
      m_columnHeaderIndexes.append("Barcode");
      m_columnHeaderIndexes.append("Reservation Date");
      m_columnHeaderIndexes.append("Copy Due Date");
    }

  m_bd.table->setColumnCount(list.size());
  m_bd.table->setHorizontalHeaderLabels(list);
  list.clear();
  m_bd.table->setRowCount(m_quantity);
  m_bd.table->scrollToTop();
  m_bd.table->horizontalScrollBar()->setValue(0);

  /*
  ** Hide the OID column.
  */

  if(m_state == biblioteq::EDITABLE)
    m_bd.table->setColumnHidden(m_bd.table->columnCount() - 1, true);

  if(!isVisible())
    updateGeometry();

  show();
  progress1.setModal(true);
  progress1.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress1.setLabelText(tr("Constructing objects..."));
  progress1.setMaximum(m_quantity);
  progress1.setMinimum(0);
  progress1.show();
  progress1.update();

  for(i = 0; i < m_quantity && !progress1.wasCanceled(); i++)
    {
      for(j = 0; j < m_bd.table->columnCount(); j++)
	if(j == 6 && m_state == biblioteq::EDITABLE)
	  {
	    if((dateEdit = new(std::nothrow) QDateEdit()) != 0)
	      {
		m_bd.table->setCellWidget(i, j, dateEdit);
		dateEdit->setDate
		  (QDate::fromString("01/01/2000", "MM/dd/yyyy"));
		dateEdit->setCalendarPopup(true);
		dateEdit->setEnabled(false);
	      }
	    else
	      qmain->addError(QString(tr("Memory Error")),
			      QString(tr("Unable to allocate memory for the "
					 "\"dateEdit\" object. "
					 "This is a serious problem!")),
			      QString(""), __FILE__, __LINE__);
	  }
	else if((item = new(std::nothrow) QTableWidgetItem()) != 0)
	  {
	    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	    if(j == 0)
	      {
		tmpStr.setNum(i + 1);
		item->setText(tmpStr);
	      }

	    m_bd.table->setItem(i, j, item);
	  }
	else
	  qmain->addError
	    (QString(tr("Memory Error")),
	     QString(tr("Unable to allocate memory for the \"item\" "
			"object. "
			"This is a serious problem!")),
	     QString(""), __FILE__, __LINE__);

      if(i + 1 <= progress1.maximum())
	progress1.setValue(i + 1);

#ifndef Q_OS_MAC
      progress1.repaint();
      QApplication::processEvents();
#endif
    }

  progress1.close();
  m_bd.table->setRowCount(i); // Support cancellation.
  query.setForwardOnly(true);

  if(m_state == biblioteq::EDITABLE)
    {
      query.prepare("SELECT borrowers.copy_number, "
		    "borrowers.copyid, "
		    "member.memberid, "
		    "member.first_name, "
		    "member.last_name, "
		    "borrowers.reserved_date, "
		    "borrowers.duedate, "
		    "borrowers.reserved_by, "
		    "borrowers.myoid "
		    "FROM member member, "
		    "item_borrower borrowers "
		    "WHERE borrowers.type = ? AND "
		    "borrowers.item_oid = ? AND "
		    "borrowers.memberid = member.memberid "
		    "ORDER BY borrowers.copy_number");
      query.bindValue(0, m_itemType);
      query.bindValue(1, m_ioid);
    }
  else
    {
      query.prepare("SELECT borrowers.copy_number, "
		    "borrowers.copyid, "
		    "borrowers.reserved_date, "
		    "borrowers.duedate "
		    "FROM "
		    "item_borrower_vw borrowers "
		    "WHERE borrowers.type = ? AND borrowers.item_oid = ? "
		    "ORDER BY borrowers.copy_number");
      query.bindValue(0, m_itemType);
      query.bindValue(1, m_ioid);
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    qmain->addError(QString(tr("Database Error")),
		    QString(tr("Unable to retrieve borrower data.")),
		    query.lastError().text(), __FILE__, __LINE__);

  QApplication::restoreOverrideCursor();
  progress2.setModal(true);
  progress2.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress2.setLabelText(tr("Retrieving borrower data..."));
  progress2.setMinimum(0);

  /*
  ** SQLite does not support query.size().
  */

  if(qmain->getDB().driverName() == "QSQLITE")
    {
      if(query.lastError().isValid())
	progress2.setMaximum(0);
      else
	progress2.setMaximum
	  (biblioteq_misc_functions::sqliteQuerySize(query.lastQuery(),
						     query.boundValues(),
						     qmain->getDB(),
						     __FILE__,
						     __LINE__));
    }
  else
    progress2.setMaximum(query.size());

  progress2.show();
  progress2.update();
  i = -1;

  QDate date;
  QDate tomorrow(QDate::currentDate());

  tomorrow = tomorrow.addDays(1);

  while(i++, !progress2.wasCanceled() && query.next())
    {
      if(query.isValid())
	for(j = 0; j < m_bd.table->columnCount(); j++)
	  {
	    if(query.value(0).isNull())
	      row = i;
	    else
	      row = query.value(0).toInt() - 1;

	    if(j == 0 && query.value(0).isNull())
	      str = QString::number(i + 1);
	    else if(j == 5 && m_state == biblioteq::EDITABLE)
	      {
		date = QDate::fromString(query.value(j).toString(),
					 "MM/dd/yyyy");
		str = date.toString(Qt::ISODate);
	      }
	    else if((j == 2 || j == 3) && m_state != biblioteq::EDITABLE)
	      {
		date = QDate::fromString(query.value(j).toString(),
					 "MM/dd/yyyy");
		str = date.toString(Qt::ISODate);
	      }
	    else
	      str = query.value(j).toString();

	    if(j == 6 && m_state == biblioteq::EDITABLE)
	      {
		QDateEdit *de =
		  qobject_cast<QDateEdit *> (m_bd.table->cellWidget(row, j));

		if(de)
		  {
		    de->setDate(QDate::fromString(str, "MM/dd/yyyy"));
		    de->setEnabled(true);
		    de->setMinimumDate(tomorrow);
		  }
	      }
	    else if(m_bd.table->item(row, j) != 0)
	      m_bd.table->item(row, j)->setText(str);
	    else
	      terminate = true;
	  }

      if(i + 1 <= progress2.maximum())
	progress2.setValue(i + 1);

#ifndef Q_OS_MAC
      progress2.repaint();
      QApplication::processEvents();
#endif

      if(terminate)
	break; // Out of resources?
    }

  progress2.close();

  for(int i = 0; i < m_bd.table->columnCount() - 1; i++)
    m_bd.table->resizeColumnToContents(i);
}

/*
** -- slotEraseBorrower() --
*/

void biblioteq_borrowers_editor::slotEraseBorrower(void)
{
  QSqlQuery query(qmain->getDB());
  QString availability = "";
  QString copyid = "";
  QString errorstr = "";
  QString memberid = "";
  QString oid = "";
  QString returnedDate = QDateTime::currentDateTime().toString("MM/dd/yyyy");
  int row = m_bd.table->currentRow();

  if(row < 0)
    {
      QMessageBox::critical
	(this, tr("BiblioteQ: User Error"),
	 tr("Please select the copy that has been returned."));
      return;
    }

  oid = biblioteq_misc_functions::getColumnString
    (m_bd.table, row, m_columnHeaderIndexes.indexOf("MYOID"));

  if(oid.isEmpty())
    {
      QMessageBox::critical(this, tr("BiblioteQ: User Error"),
			    tr("It appears that the selected "
			       "item has not been reserved."));
      return;
    }

  if(QMessageBox::question(this, tr("BiblioteQ: Question"),
			   tr("Are you sure that the copy has been returned?"),
			   QMessageBox::Yes | QMessageBox::No,
			   QMessageBox::No) == QMessageBox::No)
    return;

  query.prepare("DELETE FROM item_borrower WHERE "
		"myoid = ? AND type = ?");
  query.bindValue(0, oid);
  query.bindValue(1, m_itemType);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(QString(tr("Database Error")),
		      QString(tr("Unable to modify the reservation status of "
				 "the selected copy.")),
		      query.lastError().text(), __FILE__, __LINE__);
      QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
			    tr("Unable to modify the reservation status of "
			       "the selected copy."));
      return;
    }
  else
    {
      /*
      ** Record the return in the history table.
      */

      copyid = biblioteq_misc_functions::getColumnString
	(m_bd.table, row,
	 m_columnHeaderIndexes.indexOf("Barcode"));
      memberid = biblioteq_misc_functions::getColumnString
	(m_bd.table, row,
	 m_columnHeaderIndexes.indexOf("Member ID"));
      query.prepare("UPDATE member_history SET returned_date = ? "
		    "WHERE item_oid = ? AND copyid = ? AND "
		    "memberid = ?");
      query.bindValue(0, returnedDate);
      query.bindValue(1, m_ioid);
      query.bindValue(2, copyid);
      query.bindValue(3, memberid);

      if(!query.exec())
	qmain->addError(QString(tr("Database Error")),
			QString(tr("Unable to modify the returned date of "
				   "the selected copy.")),
			query.lastError().text(), __FILE__, __LINE__);

      QApplication::restoreOverrideCursor();
      qmain->updateMembersBrowser(memberid);

      /*
	QApplication::setOverrideCursor(Qt::WaitCursor);
	availability = biblioteq_misc_functions::getAvailability
	(ioid, qmain->getDB(),
	itemType, errorstr);
	QApplication::restoreOverrideCursor();

	if(!errorstr.isEmpty())
	qmain->addError(QString(tr("Database Error")),
	QString(tr("Retrieving availability.")),
	errorstr, __FILE__, __LINE__);

	biblioteq_misc_functions::updateColumn
	(qmain->getUI().table, bitem->getRow(),
	tr("Availability"), availability);
	biblioteq_misc_functions::updateColumn
	(qmain->getUI().table, bitem->getRow(),
	tr("Due Date"), "Returned");
      */

      /*
      ** Update the Reservation History panel, if necessary.
      */

      qmain->updateReservationHistoryBrowser(memberid, m_ioid, copyid,
					     m_itemType, returnedDate);
      showUsers();

      /*
      ** Update the main window's summary panel, if necessary.
      */

      if(m_bitem &&
	 m_ioid ==
	 biblioteq_misc_functions::getColumnString(qmain->getUI().table,
						   m_bitem->getRow(),
						   "MYOID") &&
	 m_itemType ==
	 biblioteq_misc_functions::getColumnString(qmain->getUI().table,
						   m_bitem->getRow(),
						   qmain->getUI().table->
						   columnNumber("Type")))
	qmain->slotDisplaySummary();
    }
}

/*
** -- slotCloseCurrentBorrowers() --
*/

void biblioteq_borrowers_editor::slotCloseCurrentBorrowers(void)
{
  deleteLater();
}

/*
** -- slotSave() --
*/

void biblioteq_borrowers_editor::slotSave(void)
{
  QDate now = QDate::currentDate();
  QDateEdit *dueDate = 0;
  QProgressDialog progress(this);
  QSqlQuery query(qmain->getDB());
  QString oid = "";
  bool error = false;
  int i = 0;

#ifdef Q_OS_MAC
#if QT_VERSION < 0x050000
  progress.setAttribute(Qt::WA_MacMetalStyle, BIBLIOTEQ_WA_MACMETALSTYLE);
#endif
#endif
  progress.setCancelButton(0);
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.setLabelText(tr("Updating the due date(s)..."));
  progress.setMaximum(m_bd.table->rowCount());
  progress.setMinimum(0);
  progress.show();
#ifndef Q_OS_MAC
  progress.repaint();
  QApplication::processEvents();
#endif

  for(i = 0; i < m_bd.table->rowCount(); i++)
    {
      oid = biblioteq_misc_functions::getColumnString
	(m_bd.table, i, m_columnHeaderIndexes.indexOf("MYOID"));

      if(!oid.isEmpty())
	{
	  dueDate = qobject_cast<QDateEdit *> (m_bd.table->cellWidget(i, 6));

	  if(!dueDate || dueDate->date() <= now)
	    error = true;
	  else
	    {
	      query.prepare("UPDATE item_borrower "
			    "SET duedate = ? "
			    "WHERE myoid = ? AND "
			    "type = ?");
	      query.bindValue(0, dueDate->date().toString("MM/dd/yyyy"));
	      query.bindValue(1, oid);
	      query.bindValue(2, m_itemType);

	      if(!query.exec())
		qmain->addError(QString(tr("Database Error")),
				QString(tr("Unable to update the due date.")),
				query.lastError().text(), __FILE__,
				__LINE__);
	      else
		{
		  /*
		    biblioteq_misc_functions::updateColumn
		    (qmain->getUI().table,
		    bitem->getRow(), tr("Due Date"),
		    dueDate->date().toString(Qt::ISODate));
		  */
		}
	    }
	}

      if(i + 1 <= progress.maximum())
	progress.setValue(i + 1);

#ifndef Q_OS_MAC
      progress.repaint();
      QApplication::processEvents();
#endif
    }

  progress.close();

  if(error)
    QMessageBox::critical(this, tr("BiblioteQ: User Error"),
			  tr("Some or all of the Due Dates were not updated "
			     "because of invalid dates."));

  if(query.lastError().isValid())
    QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
			  tr("Some or all of the Due Dates were not updated "
			     "because of database errors."));
}

/*
** -- keyPressEvent() --
*/

void biblioteq_borrowers_editor::keyPressEvent(QKeyEvent *event)
{
  if(event && event->key() == Qt::Key_Escape)
    slotCloseCurrentBorrowers();

  QDialog::keyPressEvent(event);
}

/*
** -- closeEvent() --
*/

void biblioteq_borrowers_editor::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event);
  slotCloseCurrentBorrowers();
}

/*
** -- setGlobalFonts() --
*/

void biblioteq_borrowers_editor::setGlobalFonts(const QFont &font)
{
  setFont(font);

  foreach(QWidget *widget, findChildren<QWidget *> ())
    {
      widget->setFont(font);
      widget->update();
    }

  update();
}

/*
** -- changeEvent() --
*/

void biblioteq_borrowers_editor::changeEvent(QEvent *event)
{
  if(event)
    switch(event->type())
      {
      case QEvent::LanguageChange:
	{
	  m_bd.retranslateUi(this);
	  break;
	}
      default:
	break;
      }

  QDialog::changeEvent(event);
}
