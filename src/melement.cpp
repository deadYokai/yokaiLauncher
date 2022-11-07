
#include <ui_melement.h>
#include <melement.h>
#include <QFile>


MElement::MElement(QWidget *parent) : QWidget(parent), ui(new Ui::melement){
    ui->setupUi(this);
}

MElement::~MElement(){
    delete ui;
}

void MElement::setDescription(QString* d){

}
void MElement::setModName(QString* m){

}
void MElement::setIcon(QPixmap* p){

}

void MElement::clicked(){

}