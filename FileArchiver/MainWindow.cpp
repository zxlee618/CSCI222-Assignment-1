/*
 * File:   MainWindow.cpp
 * Author: parallels
 *
 * Created on August 31, 2015, 10:14 PM
 */

#include "MainWindow.h"
#include "RetrieveForm.h"
#include <string>
MainWindow::MainWindow(std::vector<versionRec>* Data) {
    data=Data;
    parent=0;
    fileVersionSelectedInTable=-1;
    widget.setupUi(this);
    
    tableModel = new TableModel(0);
    tableModel->addTheData(data);
    
    widget.fileView->setModel(tableModel);
    widget.fileView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(widget.selectFileButton,SIGNAL(clicked()),this, SLOT(selectFile()));
    connect(widget.SaveCurrentButton,SIGNAL(clicked()),this, SLOT(saveCurrent()));
    connect(widget.SetReferenceButton,SIGNAL(clicked()),this, SLOT(setAsReference()));
    connect(widget.RetrieveVersionButton,SIGNAL(clicked()),this, SLOT(retrieveVersion()));
    connect(widget.ShowCommentButton,SIGNAL(clicked()),this, SLOT(showComment()));
    connect(widget.fileView, SIGNAL(clicked(const QModelIndex&)),this,
            SLOT(selectionVersionEntryTableDisplay(const QModelIndex&)));
}

MainWindow::~MainWindow() {
    
}
void MainWindow::selectFile()
{   
    QString fileSelection = QFileDialog::getOpenFileName(parent, "Open File",".",
            "Files (*)");
    if(!fileSelection.isEmpty()){
        fileSelect=fileSelection;
        widget.fileField->setText(fileSelection);
        saveFile = false;
        fileVersionSelectedInTable=-1;
        if(file.exists(fileSelect.toStdString())){
            retrieveVersionDataForFile();
            widget.warningFrame->setPlainText("The older version(s) of file been stated as below");
        }
        else
            widget.warningFrame->setPlainText("The file does not been saved into the database yet");
    }
}

void MainWindow::retrieveVersionDataForFile(){
    data->clear();
    std::vector<versionRec>temp = file.getVersionInfo(fileSelect.toStdString());  
    totalEnableForSelection = temp.size();
    for (unsigned int a=0; a<temp.size();a++)
    {
        temp[a].setSymbol(1);
        data->push_back(temp[a]);
    }
    // File have not been saved
    if(!saveFile)
    {
        versionRec ver;
        ver.setVersionNumber(-99);
        ver.setModifyTime(getFileModifyTime(fileSelect.toStdString()));
        ver.setLength(fileSize(fileSelect.toStdString()));
        ver.setSymbol(0);
        ver.setComment("The file have not been saved");
        data->push_back(ver);
    }
    tableModel->resetData(data);
    widget.fileView->resizeColumnsToContents();
}

void MainWindow::saveCurrent()
{
    bool ok;
    QString comment = QInputDialog::getText(this,"GetCommentForm","Comment",
                        QLineEdit::Normal,QDir::home().dirName(), &ok);
   
    if(ok)
    {    
        bool exist = file.exists(fileSelect.toStdString());
        if(exist)
        {
            bool different = file.differs(fileSelect.toStdString());
            if(different)
            {
                bool updateFile = file.update(fileSelect.toStdString(),comment.toStdString());
                if(!updateFile)
                {
                    std::string msg = "The file has NOT been save due to corruption";
                    QMessageBox::information(this,fileSelect,msg.c_str(),
                        QMessageBox::Ok,QMessageBox::Cancel);
                }
                else
                    saveFile = true;
            }
            else
            {
                std::string msg  ="The file has NOT been modified no save required";
                QMessageBox::information(this,fileSelect,msg.c_str(),
                        QMessageBox::Ok,QMessageBox::Cancel);
            }
        }
        else
        {
            file.insertNew(fileSelect.toStdString(), comment.toStdString());
            widget.fileField->clear();
            saveFile = true;
        }
        retrieveVersionDataForFile();
    }
    else
    {
        std::string msg = "The file has NOT no save due 'Cancel' button clicked";
        QMessageBox::information(parent,fileSelect,msg.c_str(),
                QMessageBox::Ok,QMessageBox::Cancel);
    }
}

void MainWindow::selectionVersionEntryTableDisplay(const QModelIndex& index)
{
    fileVersionSelectedInTable = index.row();
}

void MainWindow::showComment()
{
    if(fileVersionSelectedInTable==-1)
    {
        std::string comment = "No file been selected to display the comment";
        QMessageBox::critical(parent,"Error", 
        comment.c_str(),QMessageBox::Ok,QMessageBox::Cancel);
    }
    else
    {
        std::string comment = data->at(fileVersionSelectedInTable).getComment();
        QMessageBox::information(parent,"Comment for selected version", 
        comment.c_str(),QMessageBox::Ok,QMessageBox::Cancel);
    }

}

void MainWindow::retrieveVersion()
{
    
    RetrieveForm *rf = new RetrieveForm;
    rf->exec();

    QString fileName = rf->getFilename();
    QString directoryPath = rf->getDirectoryPath();
    if(!fileName.isEmpty() & !directoryPath.isEmpty())
    {
        directoryPath.append('/');
        directoryPath.append(fileName);
        file.retrieveFile(fileSelect.toStdString(), directoryPath.toStdString(),
                fileVersionSelectedInTable);
    }
    else
    {
        std::string msg = "There are incomplete information needed for 'Retrieve Version' button or Cancel button been clicked";
        QMessageBox::critical(this,"Retrieve Version",msg.c_str(),
            QMessageBox::Ok,QMessageBox::Cancel);
    }
        
}

void MainWindow::setAsReference()
{
    if(fileVersionSelectedInTable==0)
    {
        std::string msg = "The file selected is initial file. The function is aborted";
        QMessageBox::critical(this,fileSelect,msg.c_str(),
            QMessageBox::Ok,QMessageBox::Cancel);
        return;
    }
    if(fileVersionSelectedInTable==-1)
    {
        std::string msg = "No file be selected to set as reference";
        QMessageBox::critical(this,fileSelect,msg.c_str(),
            QMessageBox::Ok,QMessageBox::Cancel);
        return;
    }
    if(fileVersionSelectedInTable>=totalEnableForSelection)
    {
        std::string msg = "The file have not been save. Thus, it cannot set as reference";
        QMessageBox::critical(this,fileSelect,msg.c_str(),
            QMessageBox::Ok,QMessageBox::Cancel);
        return;
    }
    QMessageBox::StandardButton reply=QMessageBox::question(parent,
            "Set reference version","Set reference version",
            QMessageBox::Ok|QMessageBox::No);
    
    if(reply == QMessageBox::Ok)
    {
        bool ok;
        QString comment = QInputDialog::getText(this,"GetCommentForm","Comment",
                            QLineEdit::Normal,QDir::home().dirName(), &ok);

        if(ok)
        {
            bool success = file.setReference(fileSelect.toStdString(), 
                    fileVersionSelectedInTable,comment.toStdString());
            if(success)
            {
                std::string msg;
                std::ostringstream converter;
                converter<<fileVersionSelectedInTable;
                msg = "The version(s) of file before No. ";
                msg.append(converter.str());
                msg.append(" drop successfully");
                QMessageBox::information(this,fileSelect,msg.c_str(),
                    QMessageBox::Ok,QMessageBox::Cancel);
            }
            else
            {
                
                std::string msg = "There are error(s) occur. The function is aborted";
                QMessageBox::critical(this,fileSelect,msg.c_str(),
                    QMessageBox::Ok,QMessageBox::Cancel);
            }
            retrieveVersionDataForFile();
        }
        else
        {
            QMessageBox msgBox;
            std::string errorMessage = "The user must click 'Ok' in GetCommentForm.";
            msgBox.setWindowTitle("Error!");
            msgBox.setText("Incomplete Progress");
            msgBox.setDetailedText(errorMessage.c_str());
            msgBox.exec();
        }
    }    
}
