#include "notifydelegate.h"
#include <QLabel>
#include <QDebug>
//#include <QApplication>
#include <QTextDocument>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QAbstractItemView>
#include <QTextOption>
#include <QMouseEvent>
#include <QTextFrame>


NotifyDelegate::NotifyDelegate(int tableWidth, QObject *parent)
    : QStyledItemDelegate(parent)
{       
    qDebug()<<"NotifyDelegate"<<tableWidth;
    minColumnHeight = 72; //equals to icon width
    textDocWidth = tableWidth - minColumnHeight;
    numNew = 0;
    separatorColor = "#E0E0E0"; //eeeeee
    mouseOverColor = "#F3FBFE";
    redNtfColor = "#F4F4F4";
    newNtfColor = Qt::white;
}

void NotifyDelegate::setNumNew(quint32 newcnt)
{
    this->numNew = newcnt;
}

void NotifyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        QStyledItemDelegate::paint(painter, option, index);

    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);
    //qDebug()<<"paint"<<index.column() << options.state;

    if(index.column() == 0 )
    {
        painter->save();

        QPixmap pixmap = index.data(Qt::EditRole).value<QPixmap>();
        QRect rect = options.rect;
        //rect.setSize(QSize(options.rect.width()-6, options.rect.height() - 6));
        painter->drawPixmap(rect, pixmap, rect);

        QPen bottomLine;
        bottomLine.setColor(separatorColor);
        bottomLine.setWidth(1);
        painter->setPen(bottomLine);

        //  if(option.state & QStyle::State_Selected)
        //    painter->fillRect(option.rect, option.palette.color(QPalette::Background));

        if(index.row() < numNew) //new notification
        {
            QBrush brush(newNtfColor);
            painter->fillRect(option.rect, brush);
        }

        bool hovered = false;
        if(option.state & QStyle::State_MouseOver)
        {
            hovered = true;
            QBrush brush(mouseOverColor);
            painter->fillRect(option.rect, brush);
        }
        else
        {
            QAbstractItemView *table = qobject_cast<QAbstractItemView *>(this->parent());
            if(table)
            {
                QModelIndex hoveredIndex = table->indexAt(table->viewport()->mapFromGlobal(QCursor::pos()));
                if(hoveredIndex.row() == index.row())
                {
                    hovered = true;
                    QBrush brush(mouseOverColor);
                    painter->fillRect(option.rect, brush);
                    table->update(hoveredIndex);
                }
            }
        }

        painter->drawLine(options.rect.bottomLeft(), options.rect.bottomRight());
        painter->restore();
    }
    else if(index.column() == 1)
    {
        painter->save();
        QTextDocument doc;
        QTextOption opt(Qt::AlignVCenter);
        opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        doc.setDefaultTextOption(opt);
        doc.setDocumentMargin(0.0);
        doc.setHtml(options.text);
        doc.setPageSize(QSize(options.rect.size()));
        options.text = "";

        QTextFrameFormat fmt = doc.rootFrame()->frameFormat();
        fmt.setLeftMargin(0.0);
        fmt.setTopMargin(10.0);
        fmt.setRightMargin(12.0);
        fmt.setBottomMargin(4.0);
        //fmt.setBackground(QBrush(Qt::red));
        //fmt.setBorder(1.0);
        fmt.setBorderStyle(QTextFrameFormat::BorderStyle_None);
        doc.rootFrame()->setFrameFormat(fmt);

        QSize size = doc.size().toSize();
        size.setHeight(72);
        QRect clip(0,0, options.rect.width(), options.rect.height());

        QPen bottomLine;
        bottomLine.setColor(separatorColor);
        bottomLine.setWidth(1);
        painter->setPen(bottomLine);

        // if(option.state & QStyle::State_Selected)
        //   painter->fillRect(clip, option.palette.color(QPalette::Background));

        painter->translate(options.rect.left(), options.rect.top());
        if(index.row() < numNew) //new notification
        {
            QBrush brush(newNtfColor);
            painter->fillRect(clip, brush);
        }

        bool hovered;
        if(option.state & QStyle::State_MouseOver )
        {
            QBrush brush(mouseOverColor);
            painter->fillRect(clip, brush);
        }
        else
        {
            QAbstractItemView *table = qobject_cast<QAbstractItemView *>(this->parent());
            if(table)
            {
                QModelIndex hoveredIndex = table->indexAt(table->viewport()->mapFromGlobal(QCursor::pos()));
                if(hoveredIndex.row() == index.row())
                {
                    hovered = true;
                    QBrush brush(mouseOverColor);
                    painter->fillRect(clip, brush);
                    table->update(hoveredIndex);
                }
            }
        }

        doc.drawContents(painter, clip);

        painter->drawLine(QPoint(0,options.rect.height()-1),QPoint(options.rect.width(),options.rect.height()-1));
        painter->restore();
        //qDebug()<<"paint options"<<option.rect.size() << option.rect.bottomLeft() << options.rect.size() << options.rect.bottomLeft();
        //qDebug()<<"paint options"<< index.row()<< options.rect.size() << doc.size()<<doc.pageSize()<<clip.size();
    }

}


QSize NotifyDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // qDebug()<<"sizeHint";
    if (!index.isValid())
        return QStyledItemDelegate::sizeHint(option, index);

    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    if (index.column() == 0)
    {
        return QSize(72,72);
    }
    else
    {
        QTextDocument doc;
        QTextOption opt(Qt::AlignVCenter);
        opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        doc.setTextWidth(textDocWidth); //320
        doc.setDefaultTextOption(opt);
        doc.setHtml(options.text);
        //  qDebug()<<"sizeHint 1"<< index.row()<< doc.size().height() <<option.rect.height() <<options.rect.height();
        return QSize(doc.size().width(), (((minColumnHeight>  doc.size().height()) ? minColumnHeight : doc.size().height())+12)); //24 is for margines from the specification
    }
}

QWidget *NotifyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug()<<"createEditor";
    if (index.column() == 0)
    {
        QLabel *iconLabel = new QLabel(parent);
        iconLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        iconLabel->setMargin(12);
        //iconLabel->setContentsMargins(12,12,0,12);
        iconLabel->setMouseTracking(true);
        return iconLabel;
    }
    else if (index.column() == 1)
    {
        QLabel *textLabel = new QLabel(parent);
        //textLabel->setAlignment(Qt::AlignVCenter);
        //textLabel->setMargin(12); //overriden dont work        
        textLabel->setMouseTracking(true);
        return textLabel;
    }
    else
        return QStyledItemDelegate::createEditor(parent, option, index);

}

void NotifyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //qDebug()<<"setEditorData"<<index.model()->data(index, Qt::DisplayRole).toString();

    if (index.column() == 0 )
    {
        QString path  = index.model()->data(index, Qt::EditRole).toString();
        QLabel *iconLabel = static_cast<QLabel *>(editor); //++ setimage
        iconLabel->setPixmap(QPixmap(path));
        //qDebug()<<"setEditorData2.1"<<index.model()->data(index, Qt::DisplayRole).toString();
    }
    else if(index.column() == 1)
    {
        QLabel *textLabel = qobject_cast<QLabel *>(editor);
        textLabel->setTextFormat(Qt::RichText);
        textLabel->clear();
        //qDebug()<<"setEditorData2.2"<<index.model()->data(index, Qt::DisplayRole).toString();
    }
    else
        QStyledItemDelegate::setEditorData(editor, index);
}

void NotifyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    //qDebug()<< "setModelData"<<index;
    if(index.column() == 1)
    {
        QLabel *label = static_cast<QLabel*>(editor);
        model->setData(index,label->text(),Qt::EditRole);
        // qDebug()<< "setModelData" << model->data(index,Qt::DisplayRole);
    }
    else
        QStyledItemDelegate::setModelData(editor, model, index);
}


void NotifyDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & index ) const
{
    //  qDebug()<<"updateEditorGeometry"<<index;
    editor->setGeometry(option.rect);
}

void NotifyDelegate::updateTextDocWidth(qreal diff)
{
    this->textDocWidth += diff;
    qDebug()<<"updateTextDocWidth"<<textDocWidth;
    //QAbstractItemView *table = qobject_cast<QAbstractItemView *>(this->parent());
    //QModelIndex bottomRigthIndex = table->model()->index(0, 1, QModelIndex());
    // emit sizeHintChanged(bottomRigthIndex);
}

/*
bool NotifyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    qDebug()<<"editorEvent"<<index;

    return QStyledItemDelegate::editorEvent(event,model,option,index);
}
*/

void NotifyDelegate::sizeHintChanged(const QModelIndex &index)
{
    qDebug()<<"sizeHintChanged"<<index;
    QStyledItemDelegate::sizeHintChanged(index);
}
