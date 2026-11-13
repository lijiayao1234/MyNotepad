#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class searchDialog;
class ReplaceDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 保留关于对话框
    void on_actionGuan_triggered();

    // 新功能槽函数
    void newFile();
    void openFile();
    bool saveFile();
    bool saveAsFile();
    void updateWindowTitle();
    void setFontSize();
    void setFontColor();
    void setFontBgColor();
    void setEditorBgColor();
    void toggleLineNumber(bool checked);
    void toggleAutoWrap(bool checked);
    void updateStatusBar();
    void showFindDialog();
    void showReplaceDialog();
    void autoSave();

private:
    Ui::MainWindow *ui;

    QLabel statusCursorLabel;
    QLabel statusLabel;

    QString currentFileName;
    QTimer *autoSaveTimer;
    searchDialog *findDialog;
    ReplaceDialog *replaceDialog;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
