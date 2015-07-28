#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->toolButton, &QToolButton::clicked, this, &MainWindow::onBrowseForInputFilename);
    connect(ui->toolButton_2, &QToolButton::clicked, this, &MainWindow::onBrowseForOutputDirectory);
    connect(ui->inputFilename, &QLineEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(ui->outputDirectory, &QLineEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onStartExtract);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onBrowseForInputFilename()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QDir::homePath(),
                                                    tr("All files (*)"));

    if (!fileName.isEmpty())
    {
        ui->inputFilename->clear();
        ui->inputFilename->setText(fileName);
    }
}

void MainWindow::onBrowseForOutputDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        ui->outputDirectory->clear();
        ui->outputDirectory->setText(dir);
    }
}

void MainWindow::onTextChanged(const QString &text)
{
    Q_UNUSED(text)

    QFileInfo inputFile(ui->inputFilename->text());
    QFileInfo outputDir(ui->outputDirectory->text());

    ui->pushButton->setEnabled(inputFile.exists() && outputDir.exists());
}

void MainWindow::onStartExtract()
{
    QFile file(ui->inputFilename->text());

    if (file.open(QIODevice::ReadOnly))
    {
        uchar *raw = file.map(0, file.size());
        QByteArray data((const char*)raw, file.size());

        QByteArray startMatch("\x89PNG");
        QByteArray endMatch("IEND""\xae""B""\x60");
        int start = 0, end = 0;
        int index = 0;

        ui->progressBar->setRange(0, data.length());

        while (start != -1)
        {
            start = data.indexOf(startMatch, start);
            end = data.indexOf(endMatch, start);

            if (start == -1  || end == -1)
                break;

            end += endMatch.length();

            QFile output(ui->outputDirectory->text() + "/img" + QString::number(index) + ".png");
            if (output.open(QIODevice::WriteOnly))
            {
                output.write(data.constData() + start, end - start);
                output.close();
            }

            start = end;
            index++;

            ui->progressBar->setValue(start);
        }

        ui->progressBar->setValue(0);

        file.unmap(raw);
        file.close();
    }
}
