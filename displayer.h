#ifndef DISPLAYER_H
#define DISPLAYER_H

#include <QWidget>
#include <QLabel>

#include "qwt_plot.h"

class Displayer : public QWidget
{
    Q_OBJECT
public:
    explicit Displayer(QWidget *parent = nullptr);

signals:

public slots:

public:
    QLabel* lbl_label;
    QwtPlot* plt_plot;
};

#endif // DISPLAYER_H
