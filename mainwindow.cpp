#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "searchdialog.h"
#include "replacedialog.h"
#include "codeeditor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileInfo>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 状态栏初始化
    statusLabel.setMaximumWidth(150);
    statusLabel.setText("length: 0  lines: 1");
    ui->statusbar->addPermanentWidget(&statusLabel);

    statusCursorLabel.setMaximumWidth(150);
    statusCursorLabel.setText("Ln: 1  Col: 1");
    ui->statusbar->addPermanentWidget(&statusCursorLabel);

    QLabel *author = new QLabel(ui->statusbar);
    author->setText(tr("黎嘉耀"));
    ui->statusbar->addPermanentWidget(author);

    // 2. 文件操作绑定
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionLing, &QAction::triggered, this, &MainWindow::saveAsFile);
    connect(ui->actionQuan, &QAction::triggered, ui->TextEdit, &QPlainTextEdit::selectAll);
    connect(ui->actionTui, &QAction::triggered, this, &QWidget::close);

    // 3. 格式设置绑定
    connect(ui->actionZi, &QAction::triggered, this, &MainWindow::setFontSize);
    connect(ui->actionYan, &QAction::triggered, this, &MainWindow::setFontColor);
    connect(ui->actionBei, &QAction::triggered, this, &MainWindow::setFontBgColor);
    connect(ui->actionSe, &QAction::triggered, this, &MainWindow::setEditorBgColor);
    connect(ui->actionHuan, &QAction::toggled, this, &MainWindow::toggleAutoWrap);
    connect(ui->actionHang, &QAction::toggled, this, &MainWindow::toggleLineNumber);

    // 4. 查找与替换对话框初始化
    findDialog = new searchDialog(this);
    replaceDialog = new ReplaceDialog(this);
    connect(ui->actionCha, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionTi, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // 查找逻辑（加入了空内容判断，解决 QTextCursor 越界警告）
    connect(findDialog, &searchDialog::findRequested, this, [=](const QString &text, bool forward, bool caseSensitive) {
        if (text.isEmpty() || ui->TextEdit->document()->isEmpty()) return;

        QTextDocument::FindFlags flags;
        if (!forward) flags |= QTextDocument::FindBackward;
        if (caseSensitive) flags |= QTextDocument::FindCaseSensitively;

        if (!ui->TextEdit->find(text, flags)) {
            QMessageBox::information(this, tr("查找"), tr("未找到指定内容。"));
        }
    });

    // 替换对话框的查找逻辑（同理）
    connect(replaceDialog, &ReplaceDialog::findRequested, this, [=](const QString &text, bool forward, bool caseSensitive) {
        if (text.isEmpty() || ui->TextEdit->document()->isEmpty()) return;

        QTextDocument::FindFlags flags;
        if (!forward) flags |= QTextDocument::FindBackward;
        if (caseSensitive) flags |= QTextDocument::FindCaseSensitively;

        if (!ui->TextEdit->find(text, flags)) {
            QMessageBox::information(this, tr("查找"), tr("未找到指定内容。"));
        }
    });

    // 替换逻辑
    connect(replaceDialog, &ReplaceDialog::replaceRequested, this, [=](const QString &target, const QString &replacement, bool all, bool caseSensitive, bool forward) {
        if (target.isEmpty()) return;

        if (all) {
            QString content = ui->TextEdit->toPlainText();
            if (caseSensitive) {
                content.replace(target, replacement);
            } else {
                QRegularExpression re(target, QRegularExpression::CaseInsensitiveOption);
                content.replace(re, replacement);
            }
            ui->TextEdit->setPlainText(content);
        } else {
            QTextCursor cursor = ui->TextEdit->textCursor();
            if (cursor.hasSelection()) {
                QString selectedText = cursor.selectedText();
                if (selectedText.compare(target, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0) {
                    cursor.insertText(replacement);
                    return;
                }
            }
            QTextDocument::FindFlags flags;
            if (!forward) flags |= QTextDocument::FindBackward;
            if (caseSensitive) flags |= QTextDocument::FindCaseSensitively;
            if (ui->TextEdit->find(target, flags)) {
                ui->TextEdit->textCursor().insertText(replacement);
            } else {
                QMessageBox::information(this, tr("替换"), tr("未找到指定内容。"));
            }
        }
    });

    // 5. 工具栏、状态栏显隐
    connect(ui->actionGong, &QAction::toggled, ui->toolBar, &QToolBar::setVisible);
    connect(ui->actionZhuang, &QAction::toggled, ui->statusbar, &QStatusBar::setVisible);

    // 6. 编辑操作及 Action 使能状态
    connect(ui->TextEdit, &QPlainTextEdit::copyAvailable, ui->actionJian, &QAction::setEnabled);
    connect(ui->TextEdit, &QPlainTextEdit::copyAvailable, ui->actionFu, &QAction::setEnabled);
    connect(ui->TextEdit, &QPlainTextEdit::undoAvailable, ui->actionChe, &QAction::setEnabled);
    connect(ui->TextEdit, &QPlainTextEdit::redoAvailable, ui->actionHui, &QAction::setEnabled);

    ui->actionJian->setEnabled(false);
    ui->actionFu->setEnabled(false);
    ui->actionChe->setEnabled(false);
    ui->actionHui->setEnabled(false);

    connect(ui->actionChe, &QAction::triggered, ui->TextEdit, &QPlainTextEdit::undo);
    connect(ui->actionHui, &QAction::triggered, ui->TextEdit, &QPlainTextEdit::redo);
    connect(ui->actionJian, &QAction::triggered, ui->TextEdit, &QPlainTextEdit::cut);
    connect(ui->actionFu, &QAction::triggered, ui->TextEdit, &QPlainTextEdit::copy);
    connect(ui->actionZhan, &QAction::triggered, ui->TextEdit, &QPlainTextEdit::paste);

    // 7. 状态栏与标题实时更新
    connect(ui->TextEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(ui->TextEdit->document(), &QTextDocument::modificationChanged, this, &MainWindow::updateWindowTitle);

    // 8. 自动保存定时器
    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    autoSaveTimer->start(60000); // 每 60 秒触发

    currentFileName = "";
    ui->TextEdit->document()->setModified(false);
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionGuan_triggered()
{
    AboutDialog dlg(this);
    dlg.exec();
}

// ---------------- 文件操作实现 ----------------

void MainWindow::newFile()
{
    if (ui->TextEdit->document()->isModified()) {
        QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("提示"),
            tr("文档已被修改，是否保存？"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save && !saveFile()) return;
        if (ret == QMessageBox::Cancel) return;
    }
    ui->TextEdit->clear();
    currentFileName.clear();
    ui->TextEdit->document()->setModified(false);
    updateWindowTitle();
}

void MainWindow::openFile()
{
    if (ui->TextEdit->document()->isModified()) {
        QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("提示"),
            tr("文档已被修改，是否保存？"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save && !saveFile()) return;
        if (ret == QMessageBox::Cancel) return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("文本文件 (*.txt);;所有文件 (*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件：%1").arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    ui->TextEdit->setPlainText(in.readAll());
    file.close();

    currentFileName = fileName;
    ui->TextEdit->document()->setModified(false);
    updateWindowTitle();
}

bool MainWindow::saveFile()
{
    if (currentFileName.isEmpty()) return saveAsFile();
    QFile file(currentFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法保存文件：%1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << ui->TextEdit->toPlainText();
    file.close();

    ui->TextEdit->document()->setModified(false);
    updateWindowTitle();
    return true;
}

bool MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"), "", tr("文本文件 (*.txt);;所有文件 (*)"));
    if (fileName.isEmpty()) return false;
    currentFileName = fileName;
    return saveFile();
}

void MainWindow::updateWindowTitle()
{
    QString title = currentFileName.isEmpty() ? tr("新建文本文件") : QFileInfo(currentFileName).fileName();
    setWindowTitle(title + "[*] - 编辑器");
    setWindowModified(ui->TextEdit->document()->isModified());
}

// ---------------- 格式设置实现 ----------------

void MainWindow::setFontSize()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, ui->TextEdit->font(), this, tr("选择字体"));
    if (ok) ui->TextEdit->setFont(font);
}

void MainWindow::setFontColor()
{
    QColor color = QColorDialog::getColor(ui->TextEdit->textColor(), this, tr("选择字体颜色"));
    if (color.isValid()) ui->TextEdit->setTextColor(color);
}

void MainWindow::setFontBgColor()
{
    QColor color = QColorDialog::getColor(ui->TextEdit->palette().color(QPalette::Base), this, tr("选择字体背景色"));
    if (color.isValid()) {
        QPalette p = ui->TextEdit->palette();
        p.setColor(QPalette::Highlight, color); // 字体背景色通常指选中背景色
        ui->TextEdit->setPalette(p);
    }
}

void MainWindow::setEditorBgColor()
{
    QColor color = QColorDialog::getColor(ui->TextEdit->palette().color(QPalette::Base), this, tr("选择编辑器背景色"));
    if (color.isValid()) {
        QPalette p = ui->TextEdit->palette();
        p.setColor(QPalette::Base, color);
        ui->TextEdit->setPalette(p);
    }
}

void MainWindow::toggleLineNumber(bool checked)
{
    if (CodeEditor *editor = qobject_cast<CodeEditor*>(ui->TextEdit)) {
        editor->setLineNumberVisible(checked);
    }
}

void MainWindow::toggleAutoWrap(bool checked)
{
    ui->TextEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

// ---------------- 状态栏与对话框 ----------------

void MainWindow::updateStatusBar()
{
    QTextCursor cursor = ui->TextEdit->textCursor();
    int length = ui->TextEdit->toPlainText().length();
    int lines = ui->TextEdit->document()->blockCount();
    int ln = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;

    statusLabel.setText("length:" + QString::number(length) + "  lines:" + QString::number(lines));
    statusCursorLabel.setText("Ln:" + QString::number(ln) + "  Col:" + QString::number(col));
}

void MainWindow::showFindDialog()
{
    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    replaceDialog->show();
    replaceDialog->raise();
    replaceDialog->activateWindow();
}

// ---------------- 关闭与自动保存 ----------------

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (ui->TextEdit->document()->isModified()) {
        QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("退出"),
            tr("文档已被修改，是否保存？"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            if (!saveFile()) {
                event->ignore();
                return;
            }
        } else if (ret == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::autoSave()
{
    if (ui->TextEdit->document()->isModified() && !currentFileName.isEmpty()) {
        QFile file(currentFileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ui->TextEdit->toPlainText();
            file.close();
            statusBar()->showMessage(tr("已自动保存"), 2000);
        }
    }
}
