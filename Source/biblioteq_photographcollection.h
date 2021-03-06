#ifndef _BIBLIOTEQ_PHOTOGRAPHCOLLECTION_H_
#define _BIBLIOTEQ_PHOTOGRAPHCOLLECTION_H_

/*
** -- Qt Includes --
*/

#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QStringList>

/*
** -- Local Includes --
*/

#include "biblioteq_item.h"
#include "biblioteq_misc_functions.h"
#include "ui_biblioteq_photograph.h"
#include "ui_biblioteq_photographinfo.h"

class biblioteq_bgraphicsscene;
class biblioteq_borrowers_editor;
class biblioteq_copy_editor;
class biblioteq_graphicsitempixmap;

class biblioteq_photographcollection: public QMainWindow, public biblioteq_item
{
  Q_OBJECT

 public:
  biblioteq_photographcollection(QMainWindow *parentArg,
				 const QString &oidArg,
				 const int rowArg);
  ~biblioteq_photographcollection();
  static int photographsPerPage(void);
  void duplicate(const QString &p_oid, const int state);
  void insert(void);
  void modify(const int state, const QString &behavior = "");
  void search(const QString &field = "", const QString &value = "");

  void setPublicationDateFormat(const QString &dateFormat)
  {
    pc.publication_date->setDisplayFormat(dateFormat);
  }

  void updateWindow(const int state);

 private:
  QDialog *m_photo_diag;
  QString m_engWindowTitle;
  QString m_itemOid;
  Ui_pcDialog pc;
  Ui_photographDialog photo;
  biblioteq_bgraphicsscene *m_scene;
  bool verifyItemFields(void);
  void changeEvent(QEvent *event);
  void closeEvent(QCloseEvent *event);
  void loadPhotographFromItem(QGraphicsScene *scene,
			      QGraphicsPixmapItem *item,
			      const int percent);
  void loadPhotographFromItemInNewWindow(QGraphicsPixmapItem *item);
  void showPhotographs(const int page);
  void storeData(void);
  void updateTablePhotographCount(const int count);

 private slots:
  void slotAddItem(void);
  void slotCancel(void);
  void slotClosePhoto(void);
  void slotDeleteItem(void);
  void slotExportItem(void);
  void slotExportPhotographs(void);
  void slotGo(void);
  void slotImageViewSizeChanged(const QString &text);
  void slotImportItems(void);
  void slotInsertItem(void);
  void slotModifyItem(void);
  void slotPageChanged(const QString &text);
  void slotPrint(void);
  void slotQuery(void);
  void slotReset(void);
  void slotSceneSelectionChanged(void);
  void slotSelectImage(void);
  void slotUpdateItem(void);
  void slotViewContextMenu(const QPoint &pos);
  void slotViewNextPhotograph(void);
  void slotViewPhotograph(void);
  void slotViewPreviousPhotograph(void);
};

#endif
