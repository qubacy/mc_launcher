#include "statuschecker.h"

StatusChecker::StatusChecker(QWidget *parent)
    : QProgressBar (parent),
      m_label (this)
{
    m_label.setGeometry(rect());
    m_label.setObjectName("progressLabel");
    m_label.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_label.setFont(QFont("Arial"));
    
    m_label.setText(tr("Launcher is starting..."));
}

void StatusChecker::resizeEvent(QResizeEvent *event)
{
    QProgressBar::resizeEvent(event);
    
    m_label.resize(event->size());
}

void StatusChecker::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    painter.setPen(QColor(0, 0, 0, 0));
    painter.setBrush(QBrush(QColor(94,122,0)));
    painter.drawRect(rect());
    
    QRect progrRect(rect());
    
    progrRect.setWidth(rect().width() * static_cast<float>(value() / 100.));
    
    painter.setBrush(QBrush(QColor(141,182,0)));
    painter.drawRect(progrRect);
}

QSize StatusChecker::sizeHint() const
{
    return QSize(Qt::SizeHint::MaximumSize, 30);
}

void StatusChecker::setLabelText(const QString &labelText)
{
    m_label.setText(labelText);
}
