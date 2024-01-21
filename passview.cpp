#include "passview.h"
#include "ui_passview.h"

PassView::PassView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PassView)
{
    ui->setupUi(this);
}

PassView::~PassView()
{
    delete ui;
}
