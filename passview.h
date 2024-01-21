#ifndef PASSVIEW_H
#define PASSVIEW_H

#include <QWidget>

namespace Ui {
class PassView;
}

class PassView : public QWidget
{
    Q_OBJECT

public:
    explicit PassView(QWidget *parent = nullptr);
    ~PassView();

private:
    Ui::PassView *ui;
};

#endif // PASSVIEW_H
