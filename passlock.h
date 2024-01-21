#ifndef PASSLOCK_H
#define PASSLOCK_H

#include <QMainWindow>
#include <QSqlDatabase> //For db connections
#include <QSqlQuery>
#include <QSqlQueryModel> //For db sql manip
#include <QInputDialog> //For input windows
#include <QMessageBox>  //For Error Messages
#include <QFormLayout>  //For form layout
#include <QDialogButtonBox>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>//Encryption and Decryption
#include <QByteArray>
#include <QSettings>
#include <QFileDialog>
#include <QUrl>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QSysInfo>
#include <QClipboard>
#include <QTimer>
#include <QLabel>
#include <QScreen>
#include <QApplication>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
QT_BEGIN_NAMESPACE
namespace Ui {
class PassLock;
}
QT_END_NAMESPACE

class PassLock : public QMainWindow
{
    Q_OBJECT

public:
    QSqlDatabase db;
    PassLock(QWidget *parent = nullptr);
    ~PassLock();
    void ResizeTabs();
    void Login();
    QByteArray Encrypt(const QByteArray& data, const QByteArray& key);
    QByteArray Decrypt(const QByteArray& encryptedData, const QByteArray& key);
    QByteArray deriveKeyFromPassword(const QString& password);
    QString getContentUriFilePath(const QUrl& contentUri);
    void ShowPasswords();
    void QuickView();


private slots:
    void on_btnAddSave_clicked();

    void on_edtViewSearch_returnPressed();

    void on_btnViewSearch_clicked();

    void PasswordWin();

    void SetFav();

    void on_twPassLock_currentChanged(int index);

    void on_btnQVFavEmail_clicked();

    void on_btnQVFavUname_clicked();

    void on_btnQVFavPassword_clicked();

    void on_btnQVFreqEmail_clicked();

    void on_btnQVFreqUname_clicked();

    void on_btnQVFreqPassword_clicked();

    void on_btnQVRecEmail_clicked();

    void on_btnQVRecUname_clicked();

    void on_btnQVRecPassword_clicked();

    void on_btnAddRC_clicked();

    void on_btnRCDone_clicked();

    void on_btnRCAdd_clicked();

    void on_tabwRCList_cellClicked(int row);

    void on_btnRCRem_clicked();

    void deleteAcc();

    void copyFromTable(int row, int column);

    void editPassword();

    void saveChanges();

    void showPasswordDialog();

    void ChangeDbDir();

private:
    QString BtnTextManip(QString input);
    Ui::PassLock *ui;
    void copySelectedContents();
    QByteArray encryptionKey;
    bool bLog;
    QString sSelectedAcc;
    //QWidget *containerWidget = nullptr;
    QVBoxLayout *containerLayout = nullptr;
    QString sEmail,sUName,sPW;
    int RemRow;
    QTableWidget *tableWidget = nullptr;
    QFrame *frameRC = nullptr;
    QFrame *frameEdt = nullptr;
    QPushButton *btnRCRem = nullptr;
    QTextEdit *tedtRCAddText = nullptr;
    QString sEditAccName, sEditAccEmail, sEditAccUser, sEditAccPw, sEditAccRC;
};
#endif // PASSLOCK_H
