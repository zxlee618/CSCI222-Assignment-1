/*
 * File:   RetrieveForm.cpp
 * Author: parallels
 *
 * Created on September 2, 2015, 9:08 PM
 */

#include "RetrieveForm.h"

RetrieveForm::RetrieveForm() {
    widget.setupUi(this);

    connect(widget.selectFileButton,SIGNAL(clicked()),this, SLOT(selectionDir()));
    connect(widget.cancelButton,SIGNAL(clicked()),this, SLOT(cancelFunc()));
    connect(widget.okButton,SIGNAL(clicked()),this, SLOT(okFunc()));
}

RetrieveForm::~RetrieveForm() {
}

void RetrieveForm::selectionDir()
{
    QWidget *parent=0;
    QString pathDir = QFileDialog::getExistingDirectory(parent, "Open Directory",
             "/home",QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(!pathDir.isEmpty()){
        widget.directoryField->setText(pathDir);
    }
    directoryPath = pathDir;
}

void RetrieveForm::cancelFunc()
{
    fileName="";
    directoryPath="";
    QDialog::reject();
}

void RetrieveForm::okFunc()
{
    fileName = widget.fileNameField->text().trimmed();
    QDialog::accept();
}

QString RetrieveForm::getDirectoryPath()
{
    return directoryPath;
}

QString RetrieveForm::getFilename()
{
    return fileName;
}
