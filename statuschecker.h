#ifndef STATUSCHECKER_H
#define STATUSCHECKER_H

#include <QProgressBar>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>

#include <QLabel>

class StatusChecker : public QProgressBar
{
    Q_OBJECT
public:
    StatusChecker(QWidget *parent = nullptr);
    
    void resizeEvent(QResizeEvent *) override;
    void paintEvent(QPaintEvent *) override;
    
    QSize sizeHint() const override;
    
public slots:
    void setLabelText(const QString& labelText);
    
private:
    QLabel m_label;
};

#endif // STATUSCHECKER_H
