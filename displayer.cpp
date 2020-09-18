#include "displayer.h"
#include <QVBoxLayout>

Displayer::Displayer(QWidget *parent) : QWidget(parent)
{
    lbl_label = new QLabel(tr("hallo"), this);
    lbl_label->setAlignment(Qt::AlignCenter);
    plt_plot = new QwtPlot(this);
    QVBoxLayout *lay_mainLayout = new QVBoxLayout();
    lay_mainLayout->addWidget(plt_plot);
    lay_mainLayout->addWidget(lbl_label);
    setLayout(lay_mainLayout);
}
