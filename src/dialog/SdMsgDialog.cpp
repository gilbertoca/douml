// *************************************************************************
//
// Copyright 2004-2010 Bruno PAGES  .
// Copyright 2012-2013 Nikolai Marchenko.
// Copyright 2012-2013 Leonardo Guilherme.
//
// This file is part of the DOUML Uml Toolkit.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 3.0 as published by
// the Free Software Foundation and appearing in the file LICENSE.GPL included in the
//  packaging of this file.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License Version 3.0 for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// e-mail : doumleditor@gmail.com
// home   : http://sourceforge.net/projects/douml
//
// *************************************************************************





#include <stdio.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <hhbox.h>
#include <qpushbutton.h>
#include <bbuttongroup.h>
#include <qradiobutton.h>
#include <qcursor.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "SdMsgDialog.h"
#include "SdMsgBaseCanvas.h"
#include "BrowserClass.h"
#include "OperationData.h"
#include "SdDurationCanvas.h"
#include "SdLifeLineCanvas.h"
#include "SdObjCanvas.h"
#include "LabelCanvas.h"
#include "DialogUtil.h"
#include "UmlDesktop.h"
#include "BrowserView.h"
#include "translate.h"
#include "menufactory.h"

QSize SdMsgDialog::previous_size;

SdMsgDialog::SdMsgDialog(SdMsgBaseCanvas * a, const QStringList & defaults)
    : QDialog(0/*, "Message editor", TRUE*/), mc(a)
{
    setWindowTitle(tr("Message dialog"));

    QVBoxLayout * vbox = new QVBoxLayout(this);
    QHBoxLayout * hbox;

    vbox->setMargin(5);

    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    hbox->setMargin(5);

    SmallPushButton * b = new SmallPushButton(tr("message :"), this);

    hbox->addWidget(b);
    connect(b, SIGNAL(clicked()), this, SLOT(menu_op()));

    edoper = new QComboBox(this);
    edoper->setEditable(true);

    if (a->msg == 0)
        edoper->addItem(a->explicit_msg);
    else
        edoper->addItem(a->msg->definition(TRUE, FALSE));

    edoper->setAutoCompletion(completion());

    // gets operations
    if (!a->get_dest()->isaDuration())
        cl = 0;
    else if ((cl = ((SdDurationCanvas *) a->get_dest())->get_line()->get_obj()->get_class())
             != 0) {
        cl->get_opers(opers, list);
        edoper->addItems(list);

        if (!cl->is_writable())
            cl = 0;
    }

    edoper->setCurrentIndex(0);

    QSizePolicy sp = edoper->sizePolicy();

    sp.setHorizontalPolicy(QSizePolicy::Expanding);
    edoper->setSizePolicy(sp);

    hbox->addWidget(edoper);

    //

    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    hbox->setMargin(5);
    hbox->addWidget(new QLabel(tr("stereotype : "), this));
    edst = new QComboBox(this);
    edst->setEditable(true);

    if (a->stereotype != 0) {
        QString s = a->stereotype->get_name();

        s = s.mid(2, s.length() - 4);
        edst->addItem(s);
    }
    else
        edst->addItem("");

    edst->setCurrentIndex(0);
    edst->addItems(defaults);
    edst->setAutoCompletion(completion());
    hbox->addWidget(edst);
    edst->setSizePolicy(sp);

    //

    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    hbox->setMargin(5);

    HHBox * htab = new HHBox(this);

    htab->setMargin(5);

    BButtonGroup * bg = new BButtonGroup(2, Qt::Horizontal, tr("Message type"), htab);

    bg->setExclusive(TRUE);
    synchronous_rb = new QRadioButton(tr("synchronous"), bg);
    asynchronous_rb = new QRadioButton(tr("asynchronous"), bg);

    if (a->is_synchronous())
        synchronous_rb->setChecked(TRUE);
    else
        asynchronous_rb->setChecked(TRUE);

    hbox->addWidget(htab);

    vbox->addWidget(new QLabel(tr("\n\nWhen the arguments are specified they replace the \
operation's parameter(s) without any check"),
                               this));

    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    hbox->setMargin(5);
    hbox->addWidget(new QLabel(tr("arguments : "), this));
    edargs = new MultiLineEdit(this);
    edargs->setText(a->get_args());
    hbox->addWidget(edargs);
    edargs->setFocus();

    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    hbox->setMargin(5);
    QPushButton * ok = new QPushButton(tr("&OK"), this);
    QPushButton * cancel = new QPushButton(tr("&Cancel"), this);
    QSize bs(cancel->sizeHint());

    ok->setDefault(TRUE);
    ok->setFixedSize(bs);
    cancel->setFixedSize(bs);

    hbox->addWidget(ok);
    hbox->addWidget(cancel);

    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void SdMsgDialog::polish()
{
    QDialog::ensurePolished();
    UmlDesktop::limitsize_move(this, previous_size, 0.8, 0.8);
}

SdMsgDialog::~SdMsgDialog()
{
    previous_size = size();
}

void SdMsgDialog::menu_op()
{
    QMenu m(0);

    MenuFactory::addItem(m, tr("Choose"), -1);
    m.addSeparator();

    int index = list.indexOf(edoper->currentText().trimmed());

    if (index != -1)
        MenuFactory::addItem(m, tr("Select in browser"), 0);

    BrowserNode * bn = BrowserView::selected_item();

    if ((bn != 0) &&
        (bn->get_type() == UmlOperation) &&
        !bn->deletedp() &&
        (opers.indexOf((OperationData *) bn->get_data()) != -1))
        MenuFactory::addItem(m, tr("Choose operation selected in browser"), 1);
    else
        bn = 0;

    if (cl != 0)
        MenuFactory::addItem(m, tr("Create operation and choose it"), 2);

    if ((index != -1) || (bn != 0) || (cl != 0)) {
        QAction* retAction = m.exec(QCursor::pos());
        if(retAction)
        {
        switch (retAction->data().toInt()) {
        case 0:
            opers[index]->get_browser_node()->select_in_browser();
            break;

        case 2:
            bn = cl->add_operation();

            if (bn == 0)
                return;

            bn->select_in_browser();

            // no break
        case 1: {
            OperationData * od = (OperationData *) bn->get_data();

            if ((index = opers.indexOf(od)) == -1) {
                index = opers.count();
                opers.append(od);

                QString s = od->definition(TRUE, FALSE);

                list.append(s);
                edoper->addItem(s);
            }
        }

        edoper->setCurrentIndex(index + 1);
        }
        }
    }
}

void SdMsgDialog::accept()
{
    QString s = edoper->currentText().trimmed();

    if (!s.isEmpty()) {
        int index = list.indexOf(s);

        if (index >= 0)
            mc->set_msg(opers[index], QString(), edargs->text().trimmed());
        else
            mc->set_msg(0, s, edargs->text().trimmed());
    }
    else
        mc->set_msg(0, QString(), QString());

    mc->set_synchronous(synchronous_rb->isChecked());

    s = edst->currentText().trimmed();

    if (!s.isEmpty())
        s = "<<" + s + ">>";

    mc->update_st(s);

    QDialog::accept();
}
