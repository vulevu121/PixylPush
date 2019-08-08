#ifndef CREATEALBUMDIALOG_H
#define CREATEALBUMDIALOG_H

#include <QDialog>
#include <QDebug>


namespace Ui {
class CreateAlbumDialog;
}

class CreateAlbumDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateAlbumDialog(QWidget *parent = nullptr);
    ~CreateAlbumDialog();
    bool newExistingAlbum;
public slots:
    void emitCreateAlbumSignal();
    void toggleExistingAlbumOption(bool on);
    void toggleNewAlbumOption(bool on);
signals:
    void createAlbumSignal(QString const &name, QString const &desc, QString const &albumId, bool newExistingAlbum);

private:
    Ui::CreateAlbumDialog *ui;
};

#endif // CREATEALBUMDIALOG_H
