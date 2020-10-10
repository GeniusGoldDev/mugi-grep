#ifndef DISPLAYOPTIONSWIDGET_H
#define DISPLAYOPTIONSWIDGET_H

#include <QWidget>

#include "displayoptions.h"

namespace Ui {
class DisplayOptionsWidget;
}

class DisplayOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayOptionsWidget(QWidget *parent = nullptr);
    ~DisplayOptionsWidget();

    int linesBefore() const;
    int linesAfter() const;

    DisplayOptions options() const;
    void setOptions(const DisplayOptions& options);

    void trigChanged();

signals:
    void optionsChanged();

private:
    Ui::DisplayOptionsWidget *ui;
};

#endif // DISPLAYOPTIONSWIDGET_H
