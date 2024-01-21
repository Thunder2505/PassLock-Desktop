#include "passlock.h"
#include "ui_passlock.h"

PassLock::PassLock(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PassLock)
{
    ui->setupUi(this);
    QCoreApplication::setOrganizationName("SeanPersonalProject");
    QCoreApplication::setApplicationName("PassLock");
    db = QSqlDatabase::addDatabase("QSQLITE");
    connect(ui->actionChange_App_Password, SIGNAL(triggered()), this, SLOT(showPasswordDialog()));

    QSettings settings("PassLock", "SaveLocation");
    //settings.clear();
    QString lastUsedDirectory = settings.value("LastUsedDirectory").toString();
    QString defaultDirectory = (lastUsedDirectory.isEmpty()) ? QDir::homePath() : lastUsedDirectory;
    QString fileName;
    if(lastUsedDirectory.isEmpty()){
        fileName = QFileDialog::getSaveFileName(this, "Open DB File", defaultDirectory, "SQLite Database Files (*.db)");
        settings.setValue("LastUsedDirectory", fileName);
    }
    else{
        fileName=lastUsedDirectory;
    }

    if (!fileName.isEmpty()) {
        db.setDatabaseName(fileName);
        db.open();
    }
    else{
        QMessageBox msg;
        msg.setText("DB path is invalid!");
        msg.setIcon(QMessageBox::Critical);
        msg.setWindowTitle("Fatal Error");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        exit(EXIT_FAILURE);
    }

    QSqlQuery * query = new QSqlQuery(db);
    query->setForwardOnly(true);
    if (!query->exec("SELECT name FROM sqlite_master WHERE type='table' AND name='Login'") || !query->next()) { //Checks if DB is valid, and recreates it if invalid
        query->exec("CREATE TABLE \"Login\" (\"Password\"	TEXT)");
        query->exec("CREATE TABLE \"Passwords\" (\"Account\"	TEXT,\"Email\"	TEXT,\"UserName\"	TEXT,\"Password\"	TEXT,\"Fav\"	BOOLEAN DEFAULT 0,\"Access_Count\"	INTEGER DEFAULT 0,\"Last_Accessed\"	BOOLEAN DEFAULT 0,\"RC\"	TEXT)");
        query->exec("CREATE TABLE sqlite_sequence(name,seq)");
    }
    query->exec("SELECT Password FROM Login");
    if(!query->first()){
        showPasswordDialog();
    }
    bLog = false;
    while (!bLog) {
        Login();
    }
}

void PassLock::ResizeTabs(){

    ui->twPassLock->tabBar()->setDocumentMode(true);
    ui->twPassLock->tabBar()->setExpanding(true);
}


void PassLock::Login(){
    QDialog dialog;
    dialog.setWindowTitle("Login");
    dialog.setStyleSheet("QWidget { background-color: #222222; color: #eeeeee; font-family: \"Roboto\", sans-serif;}"
                         "QApplication { background-color: #222222; color: #eeeeee; font-family: \"Roboto\", sans-serif;}"
                         "QMainWindow { background-color: black;}"
                         "QScrollBar::vertical { background-color: #333333;width: 10px;border-radius: 5px;}"
                         "QScrollBar::handle:vertical { background-color: #666666; border-radius: 5px;}"
                         "QPushButton { background-color: #2d2d2d; border: 1px solid #555555; border-radius: 5px; color: #ffffff; padding: 5px 10px;}"
                         "QPushButton:hover { background-color: #3a3a3a;}"
                         "QTabBar { background-color: #333333; color: #ffffff; border: 1px solid black;}"
                         "QTabBar::tab { background-color: #444444; border: 1px solid #555555; border-bottom-left-radius: 5px; border-bottom-right-radius: 5px; padding: 5px 10px;}"
                         "QTabBar::tab:selected { background-color: #555555;}"
                         "QScrollArea { border: 1px solid #555555; background-color: #333333;}"
                         "QLineEdit { background-color: #444444; border: 1px solid #555555; border-radius: 5px; color: #ffffff; padding: 5px;}"
                         "QLineEdit:focus { border: 1px solid #777777;}"
                         "QLabel { color: #ffffff;}"
                         "QFrame{ border: 1px solid black;}"
                         "QLabel { border: 0px;}"
    );

    QFormLayout* layout = new QFormLayout(&dialog);

    QLineEdit* edtPass = new QLineEdit(&dialog);
    edtPass->setPlaceholderText("Enter Password");
    edtPass->setEchoMode(QLineEdit::Password);

    layout->addRow("Enter Password:", edtPass);

    QPushButton *submitButton = new QPushButton("Submit", &dialog);
    connect(submitButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(submitButton);

    QPushButton *cancelButton = new QPushButton("Cancel", &dialog);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    layout->addWidget(cancelButton);
    QPushButton *btnChangeDbDir = new QPushButton("Change DB", &dialog);
    connect(btnChangeDbDir, &QPushButton::clicked, this, &PassLock::ChangeDbDir);
    layout->addWidget(btnChangeDbDir);

    if (dialog.exec() == QDialog::Accepted) {
        QString password = edtPass->text();
        encryptionKey = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
        QSqlQuery * query = new QSqlQuery(db);
        query->setForwardOnly(true);
        query->exec("SELECT Password FROM Login");
        query->first();
        QByteArray CorPass = query->value(0).toByteArray();
        QByteArray checkPass = Decrypt(CorPass, encryptionKey);
        if(password==checkPass){
            bLog=true;
        }
        else{
            bLog=false;
            QMessageBox msg;
            msg.setText("Incorrect Password");
            msg.setIcon(QMessageBox::Warning);
            msg.setWindowTitle("Incorrect Password!");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.exec();
        }
    }
    else{
        exit(EXIT_FAILURE);
    }
}

void PassLock::ChangeDbDir(){
    QSettings settings("PassLock", "SaveLocation");
    QString lastUsedDirectory = settings.value("LastUsedDirectory").toString();
    QString defaultDirectory = (lastUsedDirectory.isEmpty()) ? QDir::homePath() : lastUsedDirectory;
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, "Open DB File", defaultDirectory, "SQLite Database Files (*.db)");
    settings.setValue("LastUsedDirectory", fileName);
    if (!fileName.isEmpty()) {
        db.setDatabaseName(fileName);
        db.open();
    }
    else{
        QMessageBox msg;
        msg.setText("DB path is invalid!");
        msg.setIcon(QMessageBox::Critical);
        msg.setWindowTitle("Fatal Error");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        exit(EXIT_FAILURE);
    }

    QSqlQuery * query = new QSqlQuery(db);
    query->setForwardOnly(true);
    if (!query->exec("SELECT name FROM sqlite_master WHERE type='table' AND name='Login'") || !query->next()) { //Checks if DB is valid, and recreates it if invalid
        query->exec("CREATE TABLE \"Login\" (\"Password\"	TEXT)");
        query->exec("CREATE TABLE \"Passwords\" (\"Account\"	TEXT,\"Email\"	TEXT,\"UserName\"	TEXT,\"Password\"	TEXT,\"Fav\"	BOOLEAN DEFAULT 0,\"Access_Count\"	INTEGER DEFAULT 0,\"Last_Accessed\"	BOOLEAN DEFAULT 0,\"RC\"	TEXT)");
        query->exec("CREATE TABLE sqlite_sequence(name,seq)");
    }
    query->exec("SELECT Password FROM Login");
    if(!query->first()){
        showPasswordDialog();
    }
}

QByteArray PassLock::Encrypt(const QByteArray& data, const QByteArray& key){
    QByteArray encryptedData(data);
    for (int i = 0; i < data.size(); ++i) {
        encryptedData[i] = data.at(i) ^ key.at(i % key.size());
    }
    return encryptedData.toBase64();
}

QByteArray PassLock::Decrypt(const QByteArray& encryptedData, const QByteArray& key){
    QByteArray decodedData = QByteArray::fromBase64(encryptedData);
    QByteArray decryptedData(decodedData);
    for (int i = 0; i < decodedData.size(); ++i) {
        decryptedData[i] = decodedData.at(i) ^ key.at(i % key.size());
    }
    return decryptedData;
}


PassLock::~PassLock()
{
    delete ui;
}

void PassLock::on_btnAddSave_clicked()
{
    if (ui->edtAddAccName->text()==""){
        QMessageBox msg;
        msg.setText("Please enter an account name!");
        msg.setIcon(QMessageBox::Warning);
        msg.setWindowTitle("No account name");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        return;
    }
    if (ui->edtAddEmail->text()=="" && ui->edtAddUserName->text()==""){
        QMessageBox msg;
        msg.setText("Please enter an email address/Username!");
        msg.setIcon(QMessageBox::Warning);
        msg.setWindowTitle("No email/username");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        return;
    }
    if (ui->edtAddPassword->text()==""){
        QMessageBox msg;
        msg.setText("Please enter a password!");
        msg.setIcon(QMessageBox::Warning);
        msg.setWindowTitle("No password");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        return;
    }
    QSqlQuery * query = new QSqlQuery(db);
    query->setForwardOnly(true);
    query->exec("SELECT * from Passwords WHERE Account = \"" + ui->edtAddAccName->text()+"\"");
    if(query->first()){
        QMessageBox msg;
        msg.setText("This account already exists");
        msg.setIcon(QMessageBox::Warning);
        msg.setWindowTitle("Account Exists!");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
    else{
        QString concatenatedString ="";
        for (int row = 0; row < ui->tabwRCList->rowCount(); ++row) {
            QTableWidgetItem *item = ui->tabwRCList->item(row, 0);
            if (item) {
                concatenatedString += item->text();
            }
            if (row < ui->tabwRCList->rowCount() - 1) {
                concatenatedString += ",";
            }
        }
        QString EncPass = Encrypt(ui->edtAddPassword->text().toUtf8(),encryptionKey);
        query->exec("INSERT INTO Passwords ('Account','Email','UserName','Password','Fav','Access_Count','Last_Accessed','RC') VALUES (\""+ui->edtAddAccName->text()+"\",\""+ui->edtAddEmail->text()+"\",\""+ui->edtAddUserName->text()+"\",\""+EncPass+"\",0,0,0,\""+concatenatedString+"\")");
        ui->edtAddPassword->clear();
        ui->edtAddUserName->clear();
        ui->edtAddAccName->clear();
        ui->edtAddEmail->clear();
        while(ui->tabwRCList->rowCount()!=0){
            ui->tabwRCList->removeRow(0);
        }
        ui->tedtRCAddText->clear();
    }

}

void PassLock::ShowPasswords(){
    ui->btnViewSearch->click();
}


void PassLock::on_edtViewSearch_returnPressed()
{
    ShowPasswords();
}


void PassLock::on_btnViewSearch_clicked()
{
    QMessageBox msg;
    if(containerLayout!=nullptr){
        while (QLayoutItem *item = containerLayout->takeAt(0)) {
            QWidget *widget = item->widget();
            containerLayout->removeWidget(widget);
            delete widget;
        }
    }
    QSqlQuery * query = new QSqlQuery(db);
    query->setForwardOnly(true);
    query->exec("SELECT * FROM Passwords");
    if (!query->first()){
        ui->twPassLock->setCurrentIndex(2);
    }
    query->exec("SELECT * FROM Passwords WHERE Account LIKE '%" + ui->edtViewSearch->text() + "%' ORDER BY Account asc");

    if(containerLayout==nullptr){
        containerLayout = qobject_cast<QVBoxLayout*>(ui->scrViewAll->widget()->layout());
    }
    QSpacerItem *spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    containerLayout->addItem(spacerItem);
    while (query->next()) {
        QPushButton *newButton = new QPushButton(BtnTextManip(query->value(0).toString()), ui->scrViewAll);
        newButton->setObjectName("btn"+query->value(0).toString());
        connect(newButton, &QPushButton::clicked, this, &PassLock::PasswordWin);
        containerLayout->insertWidget(containerLayout->count() - 1, newButton);
    }
}

void PassLock::PasswordWin() {
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    QSqlQuery * query = new QSqlQuery(db);
    query->setForwardOnly(true);
    query->exec("SELECT * FROM Passwords WHERE Account = '" + senderButton->text() + "'");
    sSelectedAcc=senderButton->text();
    query->first();
    if(containerLayout!=nullptr){
        while (QLayoutItem *item = containerLayout->takeAt(0)) {
            QWidget *widget = item->widget();
            containerLayout->removeWidget(widget);
            delete widget;
        }
    }
    QSpacerItem *vSpacePwViewUpper = new QSpacerItem(20, 30, QSizePolicy::Fixed, QSizePolicy::Fixed);
    containerLayout->addItem(new QSpacerItem(*vSpacePwViewUpper));

    // Create the View Password frame
    QFrame *frmViewPassword = new QFrame;
    frmViewPassword->setFrameShape(QFrame::StyledPanel);
    frmViewPassword->setFrameShadow(QFrame::Raised);
    QVBoxLayout *verticalLayout_11 = new QVBoxLayout(frmViewPassword);

    // Create and add QLabel to View Password frame
    QLabel *lblViewPasswordAcc = new QLabel(query->value(0).toString());
    lblViewPasswordAcc->setAlignment(Qt::AlignCenter);
    verticalLayout_11->addWidget(lblViewPasswordAcc);

    // Create and add QFrame to View Password frame
    QFrame *frmViewPasswordDetails = new QFrame;
    frmViewPasswordDetails->setFrameShape(QFrame::StyledPanel);
    frmViewPasswordDetails->setFrameShadow(QFrame::Raised);
    QVBoxLayout *verticalLayout_10 = new QVBoxLayout(frmViewPasswordDetails);

    // Create and add QLineEdit with QLabel for Email
    if(query->value(1).toString()!=""){
        QLabel *lblViewPasswordEmail = new QLabel("Email:");
        lblViewPasswordEmail->setMinimumWidth(65);
        QPushButton *btnViewPasswordEmail = new QPushButton(BtnTextManip(query->value(1).toString()));
        sEmail=query->value(1).toString();
        btnViewPasswordEmail->setStyleSheet("QPushButton {background-color: #444444;border: 1px solid #555555; /* Border color */border-radius: 5px; /* Rounded corners */color: #ffffff; /* Text color */padding: 5px; /* Padding */}"
                                            "QPushButton:hover {background-color: #777777; /* Darker background color on hover */}"
                                            );
        connect(btnViewPasswordEmail, &QPushButton::clicked, [&]() {
            QApplication::clipboard()->setText(sEmail);
        });
        btnViewPasswordEmail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QHBoxLayout *hLayViewPasswordEmail = new QHBoxLayout;
        hLayViewPasswordEmail->addWidget(lblViewPasswordEmail);
        hLayViewPasswordEmail->addWidget(btnViewPasswordEmail);
        verticalLayout_10->addLayout(hLayViewPasswordEmail);
    }
    // Create and add QLineEdit with QLabel for UserName
    if(query->value(2).toString()!=""){
        QLabel *lblViewPasswordUName = new QLabel("UserName:");
        lblViewPasswordUName->setMinimumWidth(65);
        QPushButton *btnViewPasswordUName = new QPushButton(BtnTextManip(query->value(2).toString()));
        sUName=query->value(2).toString();
        btnViewPasswordUName->setStyleSheet("QPushButton {background-color: #444444;border: 1px solid #555555; /* Border color */border-radius: 5px; /* Rounded corners */color: #ffffff; /* Text color */padding: 5px; /* Padding */}"
                                            "QPushButton:hover {background-color: #777777; /* Darker background color on hover */}"
                                            );
        connect(btnViewPasswordUName, &QPushButton::clicked, [&]() {
            QApplication::clipboard()->setText(sUName);
        });
        btnViewPasswordUName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QHBoxLayout *hLayViewPasswordUName = new QHBoxLayout;
        hLayViewPasswordUName->addWidget(lblViewPasswordUName);
        hLayViewPasswordUName->addWidget(btnViewPasswordUName);
        verticalLayout_10->addLayout(hLayViewPasswordUName);
    }
    // Create and add QLineEdit with QLabel for Password
    QLabel *lblViewPasswordPw = new QLabel("Password:");
    lblViewPasswordPw->setMinimumWidth(65);
    QPushButton *btnViewPasswordPw = new QPushButton(BtnTextManip(Decrypt(query->value(3).toByteArray(),encryptionKey)));
    sPW=query->value(3).toString();
    btnViewPasswordPw->setStyleSheet("QPushButton {background-color: #444444;border: 1px solid #555555; /* Border color */border-radius: 5px; /* Rounded corners */color: #ffffff; /* Text color */padding: 5px; /* Padding */}"
                                        "QPushButton:hover {background-color: #777777; /* Darker background color on hover */}"
                                        );
    connect(btnViewPasswordPw, &QPushButton::clicked, [&]() {
        QApplication::clipboard()->setText(Decrypt(sPW.toUtf8(),encryptionKey));
    });

    btnViewPasswordPw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QHBoxLayout *hLayViewPasswordUPw = new QHBoxLayout;
    hLayViewPasswordUPw->addWidget(lblViewPasswordPw);
    hLayViewPasswordUPw->addWidget(btnViewPasswordPw);
    verticalLayout_10->addLayout(hLayViewPasswordUPw);



    // Create and add QTable for "View RC Codes"
    if(query->value(7).toString()!=""){
        QHBoxLayout *hLayViewPasswordRC = new QHBoxLayout;
        if(tableWidget != nullptr){
            tableWidget = nullptr;
        }
        tableWidget = new QTableWidget;
        tableWidget->setObjectName("tabwViewRC");
        tableWidget->setMinimumSize(220, 200);

        tableWidget->setColumnCount(1);
        tableWidget->setHorizontalHeaderLabels(QStringList() << "RC:");
        QString RC = query->value(7).toString();

        static QRegularExpression splitRegex("[,\\n]");
        QStringList values = RC.split(splitRegex, Qt::SkipEmptyParts);

        tableWidget->setColumnCount(1);
        tableWidget->verticalHeader()->hide();

        for (int i = 0; i < values.size(); ++i) {
            tableWidget->insertRow(tableWidget->rowCount());
            QString text=values.at(i);
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignHCenter);
            tableWidget->setItem(tableWidget->rowCount()-1, 0, item);
        }
        tableWidget->setColumnWidth(0,220);
        tableWidget->setFixedWidth(220);
        connect(tableWidget, &QTableWidget::cellDoubleClicked, this, &PassLock::copyFromTable);
        //tableWidget->setEnabled(false);
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        hLayViewPasswordRC->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        hLayViewPasswordRC->addWidget(tableWidget);
        hLayViewPasswordRC->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        verticalLayout_10->addLayout(hLayViewPasswordRC);
    }

    // Create and add QPushButton for "Set Favourite"
    QPushButton *btnViewPasswordFav = new QPushButton("Set Favourite");
    connect(btnViewPasswordFav, &QPushButton::clicked, this, &PassLock::SetFav);
    QHBoxLayout *hLayViewPasswordFav = new QHBoxLayout;
    hLayViewPasswordFav->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayViewPasswordFav->addWidget(btnViewPasswordFav);
    hLayViewPasswordFav->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    verticalLayout_10->addLayout(hLayViewPasswordFav);

    // Create and add Modify Components
    QPushButton *btnDeleteAccount = new QPushButton("Delete");
    connect(btnDeleteAccount, &QPushButton::clicked, this, &PassLock::deleteAcc);
    btnDeleteAccount->setMinimumSize(60, 0);
    QPushButton *btnEditAccount = new QPushButton("Edit");
    connect(btnEditAccount, &QPushButton::clicked, this, &PassLock::editPassword);
    btnEditAccount->setMinimumSize(60, 0);
    QHBoxLayout *hLayViewModify = new QHBoxLayout;
    hLayViewModify->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayViewModify->addWidget(btnDeleteAccount);
    hLayViewModify->addWidget(btnEditAccount);
    hLayViewModify->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    verticalLayout_10->addLayout(hLayViewModify);

    QPushButton *btnViewPasswordDone = new QPushButton("Done");
    connect(btnViewPasswordDone, &QPushButton::clicked, this, &PassLock::ShowPasswords);
    QHBoxLayout *hLayViewPasswordDone = new QHBoxLayout;
    hLayViewPasswordDone->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayViewPasswordDone->addWidget(btnViewPasswordDone);
    hLayViewPasswordDone->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    // Add frmViewPasswordDetails to frmViewPassword
    verticalLayout_11->addWidget(frmViewPasswordDetails);
    verticalLayout_11->addLayout(hLayViewPasswordDone);

    // Add frmViewPassword to verticalLayout_13
    containerLayout->addWidget(frmViewPassword);

    // Add spacer (vSpacePwViewLower)
    QSpacerItem *vSpacePwViewLower = new QSpacerItem(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);
    containerLayout->addItem(new QSpacerItem(*vSpacePwViewLower));

    QSqlQuery * query2 = new QSqlQuery(db);
    query2->setForwardOnly(true);
    query2->exec("UPDATE Passwords SET Last_Accessed = 0 WHERE Account <> '"+sSelectedAcc+"'");
    query2->exec("UPDATE Passwords SET Last_Accessed = 1 WHERE Account = '"+sSelectedAcc+"'");
    query2->exec("UPDATE Passwords SET Access_Count = "+QString::number(query->value(5).toInt()+1)+" WHERE Account = '"+sSelectedAcc+"'");
}

void PassLock::SetFav(){
    QSqlQuery * query = new QSqlQuery(db);
    query->setForwardOnly(true);
    query->exec("UPDATE Passwords SET Fav = 0 WHERE Account <> '"+sSelectedAcc+"'");
    query->exec("UPDATE Passwords SET Fav = 1 WHERE Account = '"+sSelectedAcc+"'");
}

void PassLock::QuickView(){
    QSqlQuery * query = new QSqlQuery(db);
    query->setForwardOnly(true);
    query->exec("SELECT * FROM Passwords");
    if(query->first()){
        query->exec("SELECT * FROM Passwords WHERE Fav = 1");
        if(query->first()){
            ui->lblQVFavAccName->setText(query->value(0).toString());

            if(query->value(1).toString()!="")
            {
                ui->lblQVFavEmail->setVisible(true);
                ui->btnQVFavEmail->setVisible(true);
                ui->btnQVFavEmail->setText(BtnTextManip(query->value(1).toString()));
            }
            else{
                ui->lblQVFavEmail->setVisible(false);
                ui->btnQVFavEmail->setVisible(false);
            }
            if(query->value(2).toString()!="")
            {
                ui->lblQVFavUname->setVisible(true);
                ui->btnQVFavUname->setVisible(true);
                ui->btnQVFavUname->setText(BtnTextManip(query->value(2).toString()));
            }
            else{
                ui->btnQVFavUname->setVisible(false);
                ui->lblQVFavUname->setVisible(false);
            }

            ui->btnQVFavPassword->setText(BtnTextManip(Decrypt(query->value(3).toByteArray(),encryptionKey)));
            ui->frmQVFav->setVisible(true);
        }
        else{
            ui->frmQVFav->setVisible(false);
        }
        query->exec("SELECT * FROM Passwords WHERE Last_Accessed = 1");
        if(query->first()){
            ui->lblQVRecAccName->setText(query->value(0).toString());

            if(query->value(1).toString()!="")
            {
                ui->lblQVRecEmail->setVisible(true);
                ui->btnQVRecEmail->setVisible(true);
                ui->btnQVRecEmail->setText(BtnTextManip(query->value(1).toString()));
            }
            else{
                ui->lblQVRecEmail->setVisible(false);
                ui->btnQVRecEmail->setVisible(false);
            }
            if(query->value(2).toString()!="")
            {
                ui->lblQVRecUname->setVisible(true);
                ui->btnQVRecUname->setVisible(true);
                ui->btnQVRecUname->setText(BtnTextManip(query->value(2).toString()));
            }
            else{
                ui->btnQVRecUname->setVisible(false);
                ui->lblQVRecUname->setVisible(false);
            }

            ui->btnQVRecPassword->setText(BtnTextManip(Decrypt(query->value(3).toByteArray(),encryptionKey)));
            ui->frmQVRec->setVisible(true);
        }
        else{
            ui->frmQVRec->setVisible(false);
        }
        query->exec("SELECT * FROM Passwords ORDER BY Access_Count desc");
        if(query->next()){
            ui->lblQVFreqAccName->setText(query->value(0).toString());
            if(query->value(1).toString()!="")
            {
                ui->lblQVFreqEmail->setVisible(true);
                ui->btnQVFreqEmail->setVisible(true);
                ui->btnQVFreqEmail->setText(BtnTextManip(query->value(1).toString()));
            }
            else{
                ui->lblQVFreqEmail->setVisible(false);
                ui->btnQVFreqEmail->setVisible(false);
            }
            if(query->value(2).toString()!="")
            {
                ui->lblQVFreqUname->setVisible(true);
                ui->btnQVFreqUname->setVisible(true);
                ui->btnQVFreqUname->setText(BtnTextManip(query->value(2).toString()));
            }
            else{
                ui->btnQVFreqUname->setVisible(false);
                ui->lblQVFreqUname->setVisible(false);
            }
            ui->btnQVFreqPassword->setText(BtnTextManip(Decrypt(query->value(3).toByteArray(),encryptionKey)));
            ui->frmQVFreq->setVisible(true);
        }
        else{
            ui->frmQVFreq->setVisible(false);
        }
    }
    else{
        ui->twPassLock->setCurrentIndex(2);
    }
}

void PassLock::on_twPassLock_currentChanged(int index)
{
    if(index==0){
        ShowPasswords();
    }
    else if(index==1){
        QuickView();
    }
    else if(index==2){
        ui->frmAddAcc->setVisible(true);
        ui->frmAddRc->setVisible(false);
    }
}


void PassLock::on_btnQVFavEmail_clicked()
{
    QApplication::clipboard()->setText(ui->btnQVFavEmail->text());
}


void PassLock::on_btnQVFavUname_clicked()
{
    QApplication::clipboard()->setText(ui->btnQVFavUname->text());
}


void PassLock::on_btnQVFavPassword_clicked()
{
    QApplication::clipboard()->setText(ui->btnQVFavPassword->text());
}


void PassLock::on_btnQVFreqEmail_clicked()
{
    QApplication::clipboard()->setText(ui->btnQVFreqEmail->text());
}


void PassLock::on_btnQVFreqUname_clicked()
{
    QApplication::clipboard()->setText(ui->btnQVFreqUname->text());
}


void PassLock::on_btnQVFreqPassword_clicked()
{
    QApplication::clipboard()->setText(ui->btnQVFreqPassword->text());
}


void PassLock::on_btnQVRecEmail_clicked()
{
    QApplication::clipboard()->setText(Decrypt(ui->btnQVRecEmail->text().toUtf8(),encryptionKey));
}


void PassLock::on_btnQVRecUname_clicked()
{
    QApplication::clipboard()->setText(Decrypt(ui->btnQVRecUname->text().toUtf8(),encryptionKey));
}


void PassLock::on_btnQVRecPassword_clicked()
{
    QApplication::clipboard()->setText(Decrypt(ui->btnQVRecPassword->text().toUtf8(),encryptionKey));
}


void PassLock::on_btnAddRC_clicked()
{
    ui->frmAddRc->setVisible(true);
    ui->frmAddAcc->setVisible(false);
    ui->tabwRCList->setColumnWidth(0,ui->tabwRCList->width());
    ui->btnRCRem->setVisible(false);
}


void PassLock::on_btnRCDone_clicked()
{
    ui->frmAddAcc->setVisible(true);
    ui->frmAddRc->setVisible(false);
    ui->btnRCRem->setVisible(false);
}


void PassLock::on_btnRCAdd_clicked()
{
    QString inputText = ui->tedtRCAddText->toPlainText();

    static QRegularExpression spaceRemovalRegex("[\\s&&[^\\n]]");
    inputText.remove(spaceRemovalRegex);

    static QRegularExpression splitRegex("[,\\n]");
    QStringList values = inputText.split(splitRegex, Qt::SkipEmptyParts);

    ui->tabwRCList->setColumnCount(1);
    ui->tabwRCList->verticalHeader()->hide();

    for (int i = 0; i < values.size(); ++i) {
        ui->tabwRCList->insertRow(ui->tabwRCList->rowCount());
        QTableWidgetItem *item = new QTableWidgetItem(values.at(i));
        item->setTextAlignment(Qt::AlignHCenter);
        ui->tabwRCList->setItem(ui->tabwRCList->rowCount()-1, 0, item);
    }
    ui->tedtRCAddText->clear();
}


void PassLock::on_tabwRCList_cellClicked(int row)
{
    RemRow =row;
    ui->btnRCRem->setVisible(true);
}


void PassLock::on_btnRCRem_clicked()
{
    ui->tabwRCList->removeRow(RemRow);
    ui->btnRCRem->setVisible(false);
}

void PassLock::deleteAcc(){
    QMessageBox msg;
    msg.setText("Are you sure you want to delete "+sSelectedAcc);
    msg.setIcon(QMessageBox::Warning);
    msg.setWindowTitle("Deleting account");
    msg.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel);
    int result = msg.exec();
    if (result == QMessageBox::Yes) {
        msg.setText("These changes cannot be reverted! ");
        msg.setIcon(QMessageBox::Warning);
        msg.setWindowTitle("Deleting account");
        msg.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
        result = msg.exec();
        if (result == QMessageBox::Ok) {
            QSqlQuery * query = new QSqlQuery(db);
            query->exec("DELETE FROM Passwords WHERE Account='"+sSelectedAcc+"'");
            ShowPasswords();
            query->exec("Select * from Passwords");
            if(!query->first()){
                ui->twPassLock->setCurrentIndex(2);
                ui->tabHome->setVisible(false);
                ui->tabView->setVisible(false);
            }
        }
        else{
            return;
        }
    }
    else {
        return;
    }
}

void PassLock::copyFromTable(int row, int column){
    QApplication::clipboard()->setText(tableWidget->item(row, column)->text());
}

void PassLock::editPassword(){
    QSqlQuery * query = new QSqlQuery(db);
    query->setForwardOnly(true);
    query->exec("SELECT * FROM Passwords WHERE Account = '" + sSelectedAcc + "'");
    query->first();
    if(containerLayout!=nullptr){
        while (QLayoutItem *item = containerLayout->takeAt(0)) {
            QWidget *widget = item->widget();
            containerLayout->removeWidget(widget);
            delete widget;
        }
    }
    frameEdt = nullptr;
    frameEdt = new QFrame;
    frameEdt->setObjectName("frmAddAcc_2");
    frameEdt->setFrameShape(QFrame::StyledPanel);
    frameEdt->setFrameShadow(QFrame::Raised);

    // Create the vertical layout for the frame
    QVBoxLayout *frameLayout = new QVBoxLayout(frameEdt);

    // Add items to the frame layout
    QLabel *labelAccountName = new QLabel("Account Name");
    labelAccountName->setMinimumSize(70, 0);
    labelAccountName->setAlignment(Qt::AlignCenter);

    frameLayout->addWidget(labelAccountName);

    QHBoxLayout *accountNameLayout = new QHBoxLayout;
    QSpacerItem *horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLineEdit *edtAccountName = new QLineEdit;
    edtAccountName->setPlaceholderText(query->value(0).toString());
    sEditAccName= query->value(0).toString();
    connect(edtAccountName, &QLineEdit::textChanged, [&](QString text){
        sEditAccName=text;
    });

    accountNameLayout->addItem(horizontalSpacer1);
    accountNameLayout->addWidget(edtAccountName);
    accountNameLayout->addItem(horizontalSpacer2);

    frameLayout->addLayout(accountNameLayout);

    QSpacerItem *verticalSpacer1 = new QSpacerItem(20, 13, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameLayout->addItem(verticalSpacer1);

    QLabel *labelEmail = new QLabel("Email");
    labelEmail->setMinimumSize(70, 0);
    labelEmail->setAlignment(Qt::AlignCenter);

    frameLayout->addWidget(labelEmail);

    QHBoxLayout *emailLayout = new QHBoxLayout;
    QSpacerItem *horizontalSpacer3 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *horizontalSpacer4 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLineEdit *edtEmail = new QLineEdit;
    edtEmail->setPlaceholderText(query->value(1).toString());
    sEditAccEmail= query->value(1).toString();
    connect(edtEmail, &QLineEdit::textChanged, [&](QString text){
        sEditAccEmail=text;
    });

    emailLayout->addItem(horizontalSpacer3);
    emailLayout->addWidget(edtEmail);
    emailLayout->addItem(horizontalSpacer4);

    frameLayout->addLayout(emailLayout);

    QSpacerItem *verticalSpacer2 = new QSpacerItem(20, 13, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameLayout->addItem(verticalSpacer2);

    QLabel *labelUserName = new QLabel("UserName");
    labelUserName->setMinimumSize(70, 0);
    labelUserName->setAlignment(Qt::AlignCenter);

    frameLayout->addWidget(labelUserName);

    QHBoxLayout *userNameLayout = new QHBoxLayout;
    QSpacerItem *horizontalSpacer5 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *horizontalSpacer6 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLineEdit *edtUserName = new QLineEdit;
    edtUserName->setPlaceholderText(query->value(2).toString());
    sEditAccUser= query->value(2).toString();
    connect(edtUserName, &QLineEdit::textChanged, [&](QString text){
        sEditAccUser=text;
    });


    userNameLayout->addItem(horizontalSpacer5);
    userNameLayout->addWidget(edtUserName);
    userNameLayout->addItem(horizontalSpacer6);

    frameLayout->addLayout(userNameLayout);

    QSpacerItem *verticalSpacer3 = new QSpacerItem(20, 13, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameLayout->addItem(verticalSpacer3);

    QLabel *labelPassword = new QLabel("Password");
    labelPassword->setMinimumSize(70, 0);
    labelPassword->setAlignment(Qt::AlignCenter);

    frameLayout->addWidget(labelPassword);

    QHBoxLayout *passwordLayout = new QHBoxLayout;
    QSpacerItem *horizontalSpacer7 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *horizontalSpacer8 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLineEdit *edtPassword = new QLineEdit;
    edtPassword->setPlaceholderText(Decrypt(query->value(3).toByteArray(),encryptionKey));
    sEditAccPw = Decrypt(query->value(3).toByteArray(),encryptionKey);
    connect(edtPassword, &QLineEdit::textChanged, [&](QString text){
        sEditAccPw=text;
    });

    passwordLayout->addItem(horizontalSpacer7);
    passwordLayout->addWidget(edtPassword);
    passwordLayout->addItem(horizontalSpacer8);

    frameLayout->addLayout(passwordLayout);

    QSpacerItem *verticalSpacer4 = new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameLayout->addItem(verticalSpacer4);

    QHBoxLayout *editRCLayout = new QHBoxLayout;
    QSpacerItem *horizontalSpacer9 = new QSpacerItem(80, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    QPushButton *btnEditRC = new QPushButton("Edit RC");
    connect(btnEditRC, &QPushButton::clicked, [&]() {
        frameRC->setVisible(true);
        frameEdt->setVisible(false);
    });
    btnEditRC->setCheckable(false);
    btnEditRC->setFixedWidth(85);
    QSpacerItem *horizontalSpacer10 = new QSpacerItem(80, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    editRCLayout->addItem(horizontalSpacer9);
    editRCLayout->addWidget(btnEditRC);
    editRCLayout->addItem(horizontalSpacer10);

    frameLayout->addLayout(editRCLayout);

    QSpacerItem *verticalSpacer5 = new QSpacerItem(20, 9, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameLayout->addItem(verticalSpacer5);

    QHBoxLayout *saveLayout = new QHBoxLayout;
    QSpacerItem *horizontalSpacer11 = new QSpacerItem(80, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    QPushButton *btnSave = new QPushButton("Save");
    connect(btnSave, &QPushButton::clicked, this, &PassLock::saveChanges);
    btnSave->setCheckable(false);
    QPushButton *btnCancel = new QPushButton("Cancel");
    connect(btnCancel, &QPushButton::clicked, this, &PassLock::ShowPasswords);
    QSpacerItem *horizontalSpacer12 = new QSpacerItem(80, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    btnSave->setFixedWidth(60);
    btnCancel->setFixedWidth(60);
    saveLayout->addItem(horizontalSpacer11);
    saveLayout->addWidget(btnCancel);
    saveLayout->addWidget(btnSave);
    saveLayout->addItem(horizontalSpacer12);

    frameLayout->addLayout(saveLayout);


    frameRC = nullptr;
    frameRC = new QFrame;
    frameRC->setObjectName("frmAddRc_2");
    frameRC->setFrameShape(QFrame::StyledPanel);
    frameRC->setFrameShadow(QFrame::Raised);

    // Create the vertical layout for the frameRC
    QVBoxLayout *frameRCLayout = new QVBoxLayout(frameRC);

    // Add items to the frameRC layout
    QSpacerItem *vSpaceRC1 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameRCLayout->addItem(new QSpacerItem(*vSpaceRC1));

    QLabel *lblRC = new QLabel("RC:");
    lblRC->setAlignment(Qt::AlignCenter);
    frameRCLayout->addWidget(lblRC);

    QSpacerItem *vSpaceRC2 = new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameRCLayout->addItem(new QSpacerItem(*vSpaceRC2));

    QHBoxLayout *hLayRCList = new QHBoxLayout;
    QSpacerItem *hLayRCList1 = new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Fixed);
    tableWidget=nullptr;
    tableWidget = new QTableWidget;
    tableWidget->setMinimumSize(200, 200);
    tableWidget->setFixedWidth(200);
    tableWidget->setColumnCount(1);
    tableWidget->setHorizontalHeaderLabels(QStringList() << "RC:");
    tableWidget->setColumnWidth(0,200);
    QString RC = query->value(7).toString();
    sEditAccRC = RC;

    static QRegularExpression splitRegex("[,\\n]");
    QStringList values = RC.split(splitRegex, Qt::SkipEmptyParts);

    tableWidget->verticalHeader()->hide();

    for (int i = 0; i < values.size(); ++i) {
        tableWidget->insertRow(tableWidget->rowCount());
        QString text=values.at(i);
        QTableWidgetItem *item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignHCenter);
        tableWidget->setItem(tableWidget->rowCount()-1, 0, item);
    }

    QSpacerItem *hSpaceRCList2 = new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Fixed);

    hLayRCList->addItem(hLayRCList1);
    hLayRCList->addWidget(tableWidget);
    hLayRCList->addItem(hSpaceRCList2);

    frameRCLayout->addLayout(hLayRCList);

    QHBoxLayout *hLayRCRem = new QHBoxLayout;
    QSpacerItem *hSpaceRCRem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnRCRem = nullptr;
    btnRCRem = new QPushButton("Remove");
    btnRCRem->setVisible(false);
    QSpacerItem *hSpaceRCRem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(tableWidget, &QTableWidget::cellClicked, this, [&](int row){
        btnRCRem->setVisible(true);
        RemRow=row;
    });

    connect(btnRCRem, &QPushButton::clicked,this, [&](){
        tableWidget->removeRow(RemRow);
        btnRCRem->setVisible(false);
    });
    hLayRCRem->addItem(hSpaceRCRem1);
    hLayRCRem->addWidget(btnRCRem);
    hLayRCRem->addItem(hSpaceRCRem2);

    frameRCLayout->addLayout(hLayRCRem);

    QSpacerItem *vSpaceRC3 = new QSpacerItem(20, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameRCLayout->addItem(new QSpacerItem(*vSpaceRC3));

    QHBoxLayout *hLayRCAddText = new QHBoxLayout;
    QSpacerItem *hSpaceRCAddText1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    tedtRCAddText = nullptr;
    tedtRCAddText = new QTextEdit;
    tedtRCAddText->setMaximumSize(QSize(16777215, 30));
    QSpacerItem *hSpaceRCAddText2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    hLayRCAddText->addItem(hSpaceRCAddText1);
    hLayRCAddText->addWidget(tedtRCAddText);
    hLayRCAddText->addItem(hSpaceRCAddText2);

    frameRCLayout->addLayout(hLayRCAddText);

    QHBoxLayout *hLayRCAdd = new QHBoxLayout;
    QSpacerItem *hSpaceRCAdd1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    QPushButton *btnRCAdd = new QPushButton("Add");
    QSpacerItem *hSpaceRCAdd2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(btnRCAdd, &QPushButton::clicked, [&](){
        QString inputText = tedtRCAddText->toPlainText();

        static QRegularExpression spaceRemovalRegex("[\\s&&[^\\n]]");
        inputText.remove(spaceRemovalRegex);

        static QRegularExpression splitRegex("[,\\n]");
        QStringList values = inputText.split(splitRegex, Qt::SkipEmptyParts);

        for (int i = 0; i < values.size(); ++i) {
            tableWidget->insertRow(tableWidget->rowCount());
            QTableWidgetItem *item = new QTableWidgetItem(values.at(i));
            item->setTextAlignment(Qt::AlignHCenter);
            tableWidget->setItem(tableWidget->rowCount()-1, 0, item);
        }
        tedtRCAddText->clear();

    });
    hLayRCAdd->addItem(hSpaceRCAdd1);
    hLayRCAdd->addWidget(btnRCAdd);
    hLayRCAdd->addItem(hSpaceRCAdd2);

    frameRCLayout->addLayout(hLayRCAdd);

    QSpacerItem *vSpaceRC4 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameRCLayout->addItem(new QSpacerItem(*vSpaceRC4));

    QHBoxLayout *hLayRCDone = new QHBoxLayout;
    QSpacerItem *hSpaceRCDone1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    QPushButton *btnRCDone = new QPushButton("Done");
    connect(btnRCDone, &QPushButton::clicked, [&]() {
        frameRC->setVisible(false);
        frameEdt->setVisible(true);
    });
    QSpacerItem *hSpaceRCDone2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    hLayRCDone->addItem(hSpaceRCDone1);
    hLayRCDone->addWidget(btnRCDone);
    hLayRCDone->addItem(hSpaceRCDone2);

    frameRCLayout->addLayout(hLayRCDone);

    QSpacerItem *vSpaceRC5 = new QSpacerItem(10, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameRCLayout->addItem(new QSpacerItem(*vSpaceRC5));

    // Set the layout for the frameRC
    frameRC->setLayout(frameRCLayout);


    // Set the layout for the frame
    frameEdt->setLayout(frameLayout);
    containerLayout->addWidget(frameEdt);
    containerLayout->addWidget(frameRC);
    frameRC->setVisible(false);
    QSpacerItem *vSpacePwViewUpper = new QSpacerItem(20, 60, QSizePolicy::Fixed, QSizePolicy::Expanding);
    containerLayout->addItem(new QSpacerItem(*vSpacePwViewUpper));
}

void PassLock::saveChanges(){
    QSqlQuery *query = new QSqlQuery(db);
    query->setForwardOnly(true);
    QString concatenatedString ="";
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = tableWidget->item(row, 0);
        if (item) {
            concatenatedString += item->text();
        }
        if (row < tableWidget->rowCount() - 1) {
            concatenatedString += ",";
        }
    }
    query->exec("UPDATE Passwords SET Account = '"+sEditAccName+"', Email = '"+sEditAccEmail+"', UserName = '"+sEditAccUser+"', Password = '"+Encrypt(sEditAccPw.toUtf8(),encryptionKey)+"', RC = '"+concatenatedString+"' WHERE Account = '"+sSelectedAcc+"'");
    ShowPasswords();
}

void PassLock::showPasswordDialog(){
    QDialog dialog;
    dialog.setWindowTitle("Set App Password");
    dialog.setStyleSheet("QWidget { background-color: #222222; color: #eeeeee; font-family: \"Roboto\", sans-serif;}"
                         "QApplication { background-color: #222222; color: #eeeeee; font-family: \"Roboto\", sans-serif;}"
                         "QMainWindow { background-color: black;}"
                         "QScrollBar::vertical { background-color: #333333;width: 10px;border-radius: 5px;}"
                         "QScrollBar::handle:vertical { background-color: #666666; border-radius: 5px;}"
                         "QPushButton { background-color: #2d2d2d; border: 1px solid #555555; border-radius: 5px; color: #ffffff; padding: 5px 10px;}"
                         "QPushButton:hover { background-color: #3a3a3a;}"
                         "QTabBar { background-color: #333333; color: #ffffff; border: 1px solid black;}"
                         "QTabBar::tab { background-color: #444444; border: 1px solid #555555; border-bottom-left-radius: 5px; border-bottom-right-radius: 5px; padding: 5px 10px;}"
                         "QTabBar::tab:selected { background-color: #555555;}"
                         "QScrollArea { border: 1px solid #555555; background-color: #333333;}"
                         "QLineEdit { background-color: #444444; border: 1px solid #555555; border-radius: 5px; color: #ffffff; padding: 5px;}"
                         "QLineEdit:focus { border: 1px solid #777777;}"
                         "QLabel { color: #ffffff;}"
                         "QFrame{ border: 1px solid black;}"
                         "QLabel { border: 0px;}"
                         );

    QFormLayout* layout = new QFormLayout(&dialog);

    QLineEdit* edtPass = new QLineEdit(&dialog);
    edtPass->setPlaceholderText("Enter a Password");
    edtPass->setEchoMode(QLineEdit::Password);

    QLabel *lblPass =new QLabel(&dialog);
    lblPass->setText("Enter the Password you want to use to access the app");

    layout->addWidget(lblPass);
    layout->addWidget(edtPass);

    QPushButton *submitButton = new QPushButton("Submit", &dialog);
    connect(submitButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(submitButton);

    QPushButton *cancelButton = new QPushButton("Cancel", &dialog);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    layout->addWidget(cancelButton);

    if (dialog.exec() == QDialog::Accepted) {
        QString password = edtPass->text();
        QByteArray newEncKey = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
        QSqlQuery * query = new QSqlQuery(db);
        query->setForwardOnly(true);
        query->exec("SELECT Password FROM Login");
        if(!query->first()){
            query->exec("INSERT INTO Login ('Password') VALUES ('"+Encrypt(password.toUtf8(),newEncKey)+"')");
            encryptionKey = newEncKey;
        }
        else if(bLog){
            QSqlQuery * queryPw = new QSqlQuery(db);
            queryPw->setForwardOnly(true);
            queryPw->exec("SELECT Password FROM Passwords");
            QString PwOld, PwNew;
            while(queryPw->next()){
                PwOld = Decrypt(queryPw->value(0).toByteArray(),encryptionKey);
                PwNew = Encrypt(PwOld.toUtf8(),newEncKey);
                query->exec("UPDATE Passwords SET Password='"+PwNew+"' WHERE Password='"+queryPw->value(0).toString()+"'");
            }
            query->exec("UPDATE Login SET Password='"+Encrypt(password.toUtf8(),newEncKey)+"'");
            encryptionKey = newEncKey;
        }
    }
}

QString PassLock::BtnTextManip(QString input){
    input.replace("&", "&&");
    return input;
}
