#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings("Pixyl", "PixylPush");

    settingsDialog = new SettingsDialog(this);

    settings.sync();
    connect(settingsDialog, SIGNAL(settingsSaved()), this, SLOT(syncSettings()));
//    connect(this, SIGNAL(destroyed(QObject *)), this, SLOT(saveLog(QObject *)));

    queueHeader = QStringList({
        "Filename",
        "Album",
        "Status",
        "Date Added",
        "Date Modified",
        "Path",
    });
    queueModel = new QStandardItemModel();
    queueModel->setHorizontalHeaderLabels(queueHeader);
    ui->queueTableView->setModel(queueModel);
    ui->queueTableView->resizeColumnsToContents();

    watchHeader = QStringList({
        "Folder",
        "Status",
        "No. Files",
        "Last Scanned",
        "Last Modified",
        "Path",
    });
    watchModel = new QStandardItemModel();
    watchModel->setHorizontalHeaderLabels(watchHeader);
    ui->watchTableView->setModel(watchModel);

    ui->watchTableView->resizeColumnsToContents();
    connect(ui->addQueueButton, SIGNAL(clicked()), this, SLOT(addQueue()));
    connect(ui->removeQueueButton, SIGNAL(clicked()), this, SLOT(removeQueues()));
    connect(ui->clearQueueButton, SIGNAL(clicked()), this, SLOT(clearQueue()));

    connect(ui->actionSetting, SIGNAL(triggered()), this->settingsDialog, SLOT(show()));
    connect(ui->addFolderButton, SIGNAL(clicked()), this, SLOT(addFolder()));
    connect(ui->removeFolderButton, SIGNAL(clicked()), this, SLOT(removeFolders()));
    connect(ui->clearWatchlistButton, SIGNAL(clicked()), this, SLOT(clearWatchlist()));

    connect(ui->actionCreateAlbum, SIGNAL(triggered()), this, SLOT(showCreateAlbumDialog()));
    connect(ui->actionResume,SIGNAL(triggered()),this,SLOT(resumeQueue()));
    connect(ui->actionStop,SIGNAL(triggered()),this,SLOT(stopQueue()));
    connect(ui->actionLogIn,SIGNAL(triggered()),this,SLOT(googleLogIn()));
    connect(ui->actionLogOut,SIGNAL(triggered()),this,SLOT(googleLogOut()));


    /* Initialize the scan timers */
    queueTimerInit();
    folderTimerInit();
//    saveTimerInit();
//    logInit();
//    emailInit();
    /* Start scan on start up option */
    if(settings.value("startScanningStartup").toBool()){
        resumeQueue();
    }

    connect(ui->actionEmail, SIGNAL(triggered()), this, SLOT(showEmailTemplate()));
    connect(ui->actionSMS, SIGNAL(triggered()), this, SLOT(showSMSTemplate()));

}

void MainWindow::syncSettings() {
//    qDebug() << "scanning internal" <<settings->value("scanningInterval", "10").toInt(); //done
//    qDebug() << "on error retries" <<  settings->value("onErrorRetries","10").toInt();
//    qDebug() << "on error interval" << settings->value("onErrorAttemptInterval","10").toInt();
//    qDebug() << "play chime" << settings->value("playChimeUploadFinish").toBool();
//    qDebug() << "save queue exit"<< settings->value("saveQueueExit").toBool();
//    qDebug() << "show preview upload"<< settings->value("showPreviewUpload").toBool();
//    qDebug() << "start minimize"<< settings->value("startMinimizedInTray").toBool();
//    qDebug() << "start scan start up"<< settings->value("startScanningStartup").toBool(); //done


   /* If timer is not initilize yet, do nothing. The time will sync automatically by the Setting object
    * Otherwise, restart the active timer to update the interval
    * If setting is triggerd before timer is initilize, you will crash */
    if(queueTimer != nullptr && folderTimer != nullptr){
        if(settings->value("scanningInterval").toInt() != queueTimer->interval()){
            qDebug() << "Scan interval changed.";
            stopQueue();
            resumeQueue();
        }else{
            qDebug() << "Scan interval NOT changed.";
        }
    }else{
        qDebug() << "Unable to change scan interval. Timer is not initialized.";
    }

    /* Save queue on exit */
    if(settings->value("saveQueueExit").toBool()){
//        connect(ui->MainWindow,&QWidget::destroyed,this,MainWindow::saveLog()));
    }

}

void MainWindow::saveTimerInit(){
    saveTimer = new QTimer(this);
    saveTimer->start(50000);
    connect(saveTimer,SIGNAL(timeout()),this,SLOT(saveLog()));
}
void MainWindow::queueTimerStart(){
    qDebug() << "queue timer start";
    queueTimer->start(settings->value("scanningInterval", "10").toInt() * 1000);

}

void MainWindow::queueTimerStop(){
    qDebug() << "queue timer stop";
    queueTimer->stop();
}

void MainWindow::queueTimerInit(){
    queueTimer = new QTimer(this);
    connect(queueTimer,SIGNAL(timeout()),this,SLOT(queueUpload()));
}

void MainWindow::folderTimerStart(){
    qDebug() << "folder timer start";
    folderTimer->start(settings->value("scanningInterval", "10").toInt() * 1000); // convert to ms
}

void MainWindow::folderTimerStop(){
    qDebug() << "folder timer stop";
    folderTimer->stop();
}

void MainWindow::folderTimerInit(){
    folderTimer = new QTimer(this);
    connect(folderTimer,SIGNAL(timeout()),this,SLOT(folderScan()));
}

void MainWindow::resumeQueue(){
    if(queueTimer != nullptr && folderTimer != nullptr){
            queueTimerStart();
            folderTimerStart();
            ui->statusBar->showMessage("Scanning resumed");
        }else{
            qDebug() << "Unable to resume scan. Timer is not initialized";
        }
}

void MainWindow::stopQueue(){
    if(queueTimer != nullptr && folderTimer != nullptr){
            queueTimerStop();
            folderTimerStop();
            ui->statusBar->showMessage("Scanning stopped");

    }else{
        qDebug() << "Unable to stop scan. Timer is not initialized";
    }
}

void MainWindow::addQueue() {
    /* make a list of the files in the queue */
    QStringList queue;
    for(int row = 0; row < queueModel->rowCount();row++){
        queue.append(queueModel->item(row,(queueHeader.indexOf("Path")))->text());
    }
   /* add a file to queue */
    QFileDialog *fileDialog = new QFileDialog(this);

    QString filePath = fileDialog->getOpenFileName(this, tr("Select folder"), "", tr("Images (*.png *.jpg)"));
    QFileInfo fileInfo(filePath);

    if (filePath.length() > 0) {
        if(!queue.contains(filePath)){
            QList<QStandardItem *> queueRow({
                new QStandardItem(fileInfo.fileName()),
                new QStandardItem("MyAlbum"),
                new QStandardItem("Queue"),
                new QStandardItem(QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm AP")),
                new QStandardItem(fileInfo.lastModified().toString()),
                new QStandardItem(filePath)
            });

            this->queueModel->appendRow(queueRow);
            ui->statusBar->showMessage("Queue added");
            ui->queueTableView->resizeColumnsToContents();
        }else {ui->statusBar->showMessage(filePath + QString(" is already added."));}
    }

}

void MainWindow::removeQueues() {
    QModelIndexList selectedRows = ui->queueTableView->selectionModel()->selectedRows();

    foreach (QModelIndex index, selectedRows) {
        this->queueModel->removeRow(index.row());
    }

    ui->statusBar->showMessage("Queue removed");
}

void MainWindow::clearQueue() {
    this->queueModel->removeRows(0, this->queueModel->rowCount());
    ui->statusBar->showMessage("Queue cleared");
}



void MainWindow::addFolder() {
    QFileDialog *fileDialog = new QFileDialog(this);

    QString folderPath = fileDialog->getExistingDirectory(this, tr("Select folder"), "");
    QDir dir(folderPath);
    int num_files = dir.entryInfoList(QStringList() << "*.jpg" << "*.JPG",QDir::Files).length();

    QFileInfo fileInfo(folderPath);
//    qDebug() << dir.count();

    if (folderPath.length() > 0) {
        QList<QStandardItem *> watchRow({
            new QStandardItem(dir.dirName()),
            new QStandardItem("Queue"),
//            new QStandardItem(QString::number(dir.count())),
            new QStandardItem(QString::number(num_files)),
            new QStandardItem(QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm AP")),
            new QStandardItem(fileInfo.lastModified().toString()),
            new QStandardItem(folderPath),
        });

        this->watchModel->appendRow(watchRow);

        ui->statusBar->showMessage("Folder added to watchlist");
        ui->watchTableView->resizeColumnsToContents();
    }


}

void MainWindow::removeFolders() {
    QModelIndexList selectedRows = ui->watchTableView->selectionModel()->selectedRows();

    foreach (QModelIndex index, selectedRows) {
        this->watchModel->removeRow(index.row());
    }

    ui->statusBar->showMessage("Folder(s) removed from watchlist");
}

void MainWindow::clearWatchlist() {
    this->watchModel->removeRows(0, this->watchModel->rowCount());
    ui->statusBar->showMessage("Watchlist cleared");
}

void MainWindow::googleLogIn(){
    gphoto = new GooglePhoto(this);
    email = new GMAIL(this);
    auth = new GoogleOAuth2(this);
    auth->SetScope();
    auth->Authenticate();
    connect(auth,SIGNAL(authenticated(QString const)),gphoto,SLOT(SetAccessToken(QString const)));
    connect(auth,SIGNAL(authenticated(QString const)),email,SLOT(SetAccessToken(QString const)));
    connect(auth,SIGNAL(showMessage(QString const)),ui->statusBar, SLOT(showMessage(QString const)));
//    connect(auth,SIGNAL(authenticateFailed(QString const)),ui->statusBar, SLOT(showMessage(QString const)));
}

void MainWindow::googleLogOut(){
    if(auth != nullptr){
        ui->statusBar->showMessage("Logged out. Queue cleared. Watch folder removed.");
        auth->deleteCookies();
        deleteAllObjects();
        stopQueue();
        clearQueue();
        clearWatchlist();
    }else{
        ui->statusBar->showMessage("You are not currently logged in.");
    }
}

void MainWindow::deleteAllObjects(){
//    qDebug() << "Before delete:" << uploadedList  << uploadedListJson << uploadFailedList;
    uploadedList = QStringList();
    uploadedListJson = QJsonArray();
    uploadFailedList = QMap<QString,int>();
    isReady = true;
//    qDebug() << "After delete:" << uploadedList  << uploadedListJson << uploadFailedList;

}

void MainWindow::createAlbum(QString const &name, QString const &desc, QString const &albumId, bool useExistingAlbum) {
//    gphoto = new GooglePhoto(this);
//    connect(gphoto,SIGNAL(notAuthenticated(QString const)), ui->statusBar, SLOT(showMessage(QString const)));
    if (gphoto->isAuthenticated()){
        if (useExistingAlbum) {
            gphoto->SetTargetAlbumToUpload(albumId);
            qDebug() << gphoto->GetAlbumID();

        } else {
            gphoto->SetAlbumName(name);
            gphoto->SetAlbumDescription(desc);
            gphoto->CreateAlbum();
            connect(gphoto,SIGNAL(albumCreated()),gphoto,SLOT(ShareAlbum()));
        }

        /* Show message */
        connect(gphoto,SIGNAL(showMessage(QString const)), ui->statusBar, SLOT(showMessage(QString const)));
    }
    else{
        QString msg = "Google Photo is not authenticated. Please log in.";
        ui->statusBar->showMessage(msg);
    }
}

void MainWindow::showCreateAlbumDialog() {
    CreateAlbumDialog *dialog = new CreateAlbumDialog(this);
    /* Grab Id from log file */
//    dialog->setExistingAlbumId(getAlbumIdFromFile());
    /* Grab Id from registry */
    dialog->setExistingAlbumId(settings->value("lastUsedAlbumId","").toString());
    dialog->show();
    connect(dialog, SIGNAL(createAlbumSignal(QString const, QString const , QString const , bool )), this, SLOT(createAlbum(QString const, QString const , QString const , bool )));
}


void MainWindow::queueUpload(){
     qDebug() << "Checking upload queue...";
     /* Iterate through the rows in the model, if status is Queue, upload photo,
      * If status is Completed, pass*/
     for(int row = 0; row < queueModel->rowCount();row++){
             QString file = queueModel->item(row,(queueHeader.indexOf("Path")))->text();
             if(queueModel->item(row,(queueHeader.indexOf("Status")))->text() == "Queue"){
                 qDebug() << "+";
                 qDebug() << gphoto->isAlbumReady() << !gphoto->isUploading();

                     if(gphoto->isAlbumReady() && !gphoto->isUploading()){

                            if(!uploadedList.contains(file)){
                                qDebug() << "++";

                                qDebug() << "Uploading" << file;
                                gphoto->UploadPhoto(file);

                                /* Update queue model */
                                queueModel->item(row,(queueHeader.indexOf("Status")))->setText("Uploading");
                                queueModel->item(row,(queueHeader.indexOf("Album")))->setText(gphoto->GetAlbumName());
                                connect(gphoto,SIGNAL(mediaCreated(QString const)),this,SLOT(updateUploadedList(QString const)));
                                connect(gphoto,SIGNAL(mediaCreateFailed(QString const)),this,SLOT(updateFailedList(QString const)));
                            }else{
                                /* In case internet connection broke and resume */
                                QString msg = "File is already uploaded";
                                qDebug() << msg;
                                ui->statusBar->showMessage(msg);
                                queueModel->item(row,(queueHeader.indexOf("Status")))->setText("Completed");
                            }
                       }
             }else if(queueModel->item(row,(queueHeader.indexOf("Status")))->text() == "Uploading"){
                      /* if the status is "Uploading", check if it is complete, if so then change status to completed */
                     qDebug() << "-";

                     if(uploadedList.contains(file)){queueModel->item(row,(queueHeader.indexOf("Status")))->setText("Completed");
                     }else if(uploadFailedList.contains(file)){
                                qDebug() << "--";

                                if( uploadFailedList.value(file) < settings->value("onErrorRetries","10").toInt())
                                        {queueModel->item(row,(queueHeader.indexOf("Status")))->setText("Queue");
                                 }else{queueModel->item(row,(queueHeader.indexOf("Status")))->setText("Failed");
                                    if(!elapsedTime.isActive()){
                                        /* Call function once. Avoid multile singleshot called*/
                                        elapsedTime.singleShot(settings->value("onErrorAttemptInterval","10").toInt(),this,SLOT(resetFailItems()));
                                        }
                                  }
                            }
             }
         }
}

void MainWindow::resetFailItems(){
    for(int row = 0; row < queueModel->rowCount();row++){
            if(queueModel->item(row,(queueHeader.indexOf("Status")))->text() == "Failed"){
                queueModel->item(row,(queueHeader.indexOf("Status")))->setText("Queue");
            }
    }
    elapsedTime.stop();
}



void MainWindow::updateUploadedList(QString const &filename){
    QFileInfo info(filename);
    QJsonObject obj;
    obj["name"] = info.fileName();
    obj["path"] = info.filePath();
    obj["album_name"] = gphoto->GetAlbumName();
    obj["album_url"] = gphoto->GetAlbumURL();
    obj["album_id"] = gphoto->GetAlbumID();
    obj["status"] = "Completed";
    obj["date_added"] = QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm AP");
    obj["url"] = gphoto->GetUploadedPhotoURL();

    uploadedListJson.append(obj);

    if(uploadFailedList.contains(filename)){uploadFailedList.remove(filename);}
    uploadedList.append(filename);
    ui->statusBar->showMessage(filename + " is uploaded");

//    qDebug() << "After Uploaded list:" << uploadedList;
//    qDebug() << "After Uploaded list:" << uploadedListJson;

    isReady = true;
}

void MainWindow::updateFailedList(QString const &filename){
    if(uploadFailedList.contains(filename)){
        int retries = uploadFailedList.value(filename);
        retries += 1;
        uploadFailedList.insert(filename,retries);
    }else{
        uploadFailedList.insert(filename,1);
    }

    ui->statusBar->showMessage(filename + " is NOT uploaded");

//    qDebug() << "Upload failed list:" << uploadFailedList;
//    qDebug() << "Uploaded list:" << uploadedList;
}

void MainWindow::folderScan(){
    /* make a list of the files in the queue */
    QStringList queue;
    for(int row = 0; row < queueModel->rowCount();row++){
        queue.append(queueModel->item(row,(queueHeader.indexOf("Path")))->text());
    }

    /* iterate the files in the folder */
    for(int row = 0; row < watchModel->rowCount();row++){
         if(watchModel->item(row,(watchHeader.indexOf("Status")))->text() == "Queue" ||
                 watchModel->item(row,(watchHeader.indexOf("Status")))->text() == "Scanned"){
            QDir dir(watchModel->item(row,(watchHeader.indexOf("Path")))->text());
            qDebug() << "Scanning foldler" << dir.path();
            /* for each file in folder, if not in queue  and uploadedList, add to queue */
            QFileInfoList images = dir.entryInfoList(QStringList() << "*.jpg" << "*.JPG",QDir::Files);
            foreach(QFileInfo i, images){
                if (images.length() > 0 ) {
                    if( !queue.contains(i.filePath()) && isReady && !uploadedList.contains(i.filePath())){
                        QList<QStandardItem *> queueRow({
                            new QStandardItem(i.fileName()),
                            new QStandardItem("None"),
                            new QStandardItem("Queue"),
                            new QStandardItem(i.birthTime().toString()),
                            new QStandardItem(i.lastModified().toString()),
                            new QStandardItem(i.filePath())
                        });

                        this->queueModel->appendRow(queueRow);
                        ui->statusBar->showMessage("Queue added");
                        ui->queueTableView->resizeColumnsToContents();
                    }
                }
            }
            watchModel->item(row,(watchHeader.indexOf("No. Files")))->setText(QString::number(images.length()));
            watchModel->item(row,(watchHeader.indexOf("Status")))->setText("Scanned");
            connect(gphoto,SIGNAL(mediaCreated(QString)),this,SLOT(updateUploadedList(QString)));
            }
     }
}

void MainWindow::emailInit(){
    if(email != nullptr){
        connect(email,SIGNAL(authenticated()),this,SLOT(emailOut()));
    }

}
void MainWindow::emailOut(){
    email->SetToEmail("khuongnguyensac@gmail.com");
    email->SendEmail();
}

//void MainWindow::emailOut(){
////    qDebug() << "Opening upload log";
//    email->SetFromEmail("info.enchanted.oc@gmail.com");
//    /* Read the uploaded photo log file */
//    QString path1("C:/Users/khuon/Documents/Github/PixylPush/Upload_log.txt");
//    QJsonArray arr1;
//     QFile jsonFile1(path1);
//     if(jsonFile1.exists()){
//         jsonFile1.open(QFile::ReadOnly);
//         QJsonDocument document1 = QJsonDocument().fromJson(jsonFile1.readAll());
//         arr1 = document1.array();
////         qDebug() << arr1;

//         jsonFile1.close();
//    }
//   /* Read the list of item to send */
//   QString path("C:/Users/khuon/Documents/Github/PixylPush/Email.txt");
//    QJsonArray arr;
//    QFile jsonFile(path);
//    if(jsonFile.exists()){
//        jsonFile.open(QFile::ReadOnly);
//        QJsonDocument document = QJsonDocument().fromJson(jsonFile.readAll());
//        arr = document.array();
////        qDebug() << arr;
//        /* For each item, find the photo url, send the email, and add the status */
//        QJsonArray outArr;
//          for(int i = 0; i < arr.count(); i++){
//                QJsonObject jsonItem = arr[i].toObject();

//                for(int j = 0; j < arr1.count(); j++){
//                        QJsonObject jsonPhoto = arr1[i].toObject();
//                        if(jsonPhoto["path"].toString() == jsonItem["PhotoPath"].toString()){
//                            /* assume GMAIL object is already authenticated and ready */
//                            email->SetAlbumURL(jsonPhoto["url"].toString());
//                            email->SetToEmail(jsonItem["Email"].toString());
//                            email->SendEmail();
////                            qDebug() << email->GetToEmail() << email->GetAlbumURL();
//                            break;
//                        }
//                }

//                jsonItem.insert("Status", QJsonValue("Sent"));
//              outArr.push_back(jsonItem);
//            }
////          qDebug() << outArr;


//          jsonFile.close();

//          QFile outFile("C:/Users/khuon/Documents/Github/PixylPush/EmailDone.txt");

//          /* if log file does not exist, create a new one. Otherwise, overwrite */
//          if (outFile.open(QIODevice::WriteOnly)) {
//                  qDebug() << "Saving email done log";

//                  QJsonDocument json_doc(outArr);
//                  QString json_string = json_doc.toJson();

//                  outFile.write(json_string.toLocal8Bit());
//                  outFile.close();
//              }
//              else{
//                  qDebug() << "failed to open save file" << endl;
//              }
//    }else{
//        qDebug() << "No log file";

//    }

//}

QString MainWindow::getAlbumIdFromFile(){
    qDebug() << "Opening upload log";
    QFileDialog *fileDialog = new QFileDialog(this);


    QString filePath = fileDialog->getOpenFileName(this, tr("Select log file"), "", tr("text (*.json, *.txt)"));
    qDebug() << filePath;

    QJsonArray photoArr;
    QFile file(filePath);
     if(file.exists()){
             file.open(QFile::ReadOnly);
             QJsonDocument document = QJsonDocument().fromJson(file.readAll());
             file.close();
             photoArr = document.array();
//             qDebug() << photoArr;
             QString id  = photoArr[0].toObject()["album_id"].toString();
//             qDebug() << id;
             settings->setValue("lastUsedAlbumId",id);
             settings->sync();

             return id;
        }
    else{
            qDebug() << "No log file";
        }
     return "";
}

void MainWindow::logInit(){
    QFileDialog *fileDialog = new QFileDialog(this);

    QString filePath = fileDialog->getOpenFileName(this, tr("Select log file"), "", tr("text (*.json, *.txt)"));
    if (filePath.length() > 0) {
        qDebug() << filePath;
//        logPath = filePath;
        };
}

void MainWindow::saveLog(){
    /* save current album id to registry */
    if(gphoto->isAlbumReady()){
        qDebug() << "Saving album ID to registry";
        settings->setValue("lastUsedAlbumId",gphoto->GetAlbumID());
        settings->sync();
    }
    /* if log file does not exist, create a new one. Otherwise, overwrite */
//    qDebug() << logPath;
    QFile jsonFile (logPath);
    if (jsonFile.open(QIODevice::WriteOnly) && !uploadedListJson.isEmpty()) {
            qDebug() << "Saving log";

            QJsonDocument json_doc(uploadedListJson);
            QString json_string = json_doc.toJson();

            jsonFile.write(json_string.toLocal8Bit());
            jsonFile.close();
        }
        else{
            qDebug() << "failed to open save file" << endl;
            return;
        }
}

void MainWindow::sendNow(QString const &to, QString const &subject, QString const &body){
//    if(email == nullptr){
//        email = new GMAIL();
//    }
     email->SetToEmail(to);
     email->SetToEmail("7143529299@tmomail.net");
     email->SetFromEmail("khuongnguyensac@gmail.com");
     email->SetSubject(subject);
     email->SetBody(body);
//     email->SetAlbumURL(gphoto->GetAlbumURL());
     email->SetAlbumURL("Testing");

     connect(email,SIGNAL(authenticated()),email,SLOT(SendEmail()));
}

void MainWindow::showEmailTemplate() {
    EmailTemplateDialog *emailDialog = new EmailTemplateDialog(this);
    emailDialog->show();
    connect(emailDialog,SIGNAL(sendEmailSignal(QString const,QString const , QString const)),this,SLOT(sendNow(QString const,QString const , QString const)));
}

void MainWindow::showSMSTemplate() {
    SMSTemplateDialog *smsDialog = new SMSTemplateDialog(this);
    smsDialog->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

