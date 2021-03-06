/*
 *  LDForge: LDraw parts authoring CAD
 *  Copyright (C) 2013, 2014 Santeri Piippo
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <QGroupBox>
#include <QSpinBox>
#include <map>
#include "main.h"
#include "basics.h"

class QIcon;
class QCheckBox;
class QButtonGroup;
class QBoxLayout;
class QRadioButton;

//
// Convenience widget - is a groupbox of radio buttons.
//
class RadioGroup : public QGroupBox
{
	Q_OBJECT

public:
	typedef QList<QRadioButton*>::Iterator Iterator;

	explicit RadioGroup()
	{
		init (Qt::Vertical);
	}

	explicit RadioGroup (QWidget* parent = null) : QGroupBox (parent)
	{
		init (Qt::Vertical);
	}

	explicit RadioGroup (const QString& title, QWidget* parent = null);
	explicit RadioGroup (const QString& title, QList<char const*> entries, int const defaultId,
		const Qt::Orientation orient = Qt::Vertical, QWidget* parent = null);

	void            addButton	(const char* entry);
	void            addButton	(QRadioButton* button);
	Iterator        begin();
	Iterator        end();
	void            init (Qt::Orientation orient);
	bool            isChecked (int n) const;
	void            rowBreak();
	void            setCurrentRow (int row);
	void            setValue (int val);
	int             value() const;

	QRadioButton*   operator[] (int n) const;
	RadioGroup&     operator<< (QRadioButton* button);
	RadioGroup&     operator<< (const char* entry);

signals:
	void buttonPressed (int btn);
	void buttonReleased (int btn);
	void valueChanged (int val);

private:
	QList<QRadioButton*> m_objects;
	QList<QBoxLayout*> m_layouts;
	QBoxLayout* m_coreLayout;
	QBoxLayout* m_currentLayout;
	bool m_vert;
	int m_curId, m_defId, m_oldId;
	QButtonGroup* m_buttonGroup;

	Q_DISABLE_COPY (RadioGroup)

private slots:
	void slot_buttonPressed (int btn);
	void slot_buttonReleased (int btn);
};
