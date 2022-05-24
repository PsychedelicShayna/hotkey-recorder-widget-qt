#include "keyboard_modifier_list_widget.hpp"

void ExtendedQListView::mouseReleaseEvent(QMouseEvent* mouse_event) {
    const QPoint& release_pos { mouse_event->pos() };
    const QModelIndex& item_index { indexAt(release_pos) };

    if(item_index.isValid()) {
        emit ModelItemClicked(item_index);
    }
}

void KbModifierListWidget::handleItemDataChanged(QStandardItem* item) {
    const WINAPI_KEYBOARD_MODIFIER& item_modifier { QStringToWinApiKbModifier(item->text()) };

    if(item->isCheckable() && item_modifier != WINMOD_NULLMOD)  {
        emit ModifierItemCheckStateChanged(item_modifier, item->checkState());
        emit ModifierBitmaskChanged(GetModifierCheckStateAsBitmask());
    }
}

void KbModifierListWidget::handleModelItemClicked(QModelIndex index) {
    QStandardItem* item { standardItemModel->itemFromIndex(index) };
    const Qt::CheckState& item_check_state { item->checkState() };

    if(item_check_state == Qt::Checked) {
        item->setCheckState(Qt::Unchecked);
    } else if(item_check_state == Qt::Unchecked) {
        item->setCheckState(Qt::Checked);
    } else {
        qWarning() << "KbModifierListWidget::handleItemClicked encountered an unknown Qt::CheckState value:" << item_check_state;
    }
}

QStandardItem* KbModifierListWidget::ItemAt(const quint32& index) const {
    return standardItemModel->item(index);
}

void KbModifierListWidget::wheelEvent(QWheelEvent* wheel_event) {
    wheel_event->accept();
}

void KbModifierListWidget::InsertCheckableItem(const quint32& index, const QString& label, const Qt::CheckState& initial_state) {
    QStandardItem* new_checkable_item { new QStandardItem };

    new_checkable_item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    new_checkable_item->setData(initial_state, Qt::CheckStateRole);
    new_checkable_item->setText(label);

    standardItemModel->insertRow(index, new_checkable_item);
}

void KbModifierListWidget::AddCheckableItem(const QString& label, const Qt::CheckState& initial_state) {
    InsertCheckableItem(count(), label, initial_state);
}

void KbModifierListWidget::AddModifierItem(const WINAPI_KEYBOARD_MODIFIER& modifier) {
    AddCheckableItem(WinApiKbModifierToQString(modifier, true));
}

quint32 KbModifierListWidget::GetModifierCheckStateAsBitmask() const {
    quint32 modifier_bitmask { NULL };

    for(qint32 i { 0 }; i < count(); ++i) {
        const auto& item { ItemAt(i) };

        const WINAPI_KEYBOARD_MODIFIER& modifier {
            QStringToWinApiKbModifier(item->text())
        };

        if(item->isCheckable() && modifier != WINMOD_NULLMOD) {
            if(item->checkState() == Qt::Checked) {
                modifier_bitmask |= modifier;
            }
        }
    }

    return modifier_bitmask;
}

void KbModifierListWidget::SetModifierCheckStateFromBitmask(const quint32& bitmask) {
    blockSignals(true);

    for(qint32 i { 0 }; i < count(); ++i) {
        const auto& item { ItemAt(i) };

        const WINAPI_KEYBOARD_MODIFIER& item_modifier {
            QStringToWinApiKbModifier(item->text())
        };

        if(item->isCheckable() && item_modifier != WINMOD_NULLMOD) {
            item->setCheckState(bitmask & item_modifier ? Qt::Checked : Qt::Unchecked);
        }
    }

    blockSignals(false);
}

void KbModifierListWidget::AddItemsFromBitmask(const quint32& bitmask) {
    for(const WINAPI_KEYBOARD_MODIFIER& modifier : ALL_WINAPI_KEYBOARD_MODIFIERS) {
        if(modifier & bitmask) {
            AddModifierItem(modifier);
        }
    }
}

KbModifierListWidget::KbModifierListWidget(const quint32& enabled_modifiers_bitmask) {
    for(const auto& modifier : ALL_WINAPI_KEYBOARD_MODIFIERS) {
        if(modifier & enabled_modifiers_bitmask) {
            AddModifierItem(modifier);
        }
    }
}

KbModifierListWidget::KbModifierListWidget(QWidget* parent)
    :
      QComboBox            { parent                 },
      extendedQListView    { new ExtendedQListView  },
      standardItemModel    { new QStandardItemModel }
{
    setView(extendedQListView);
    setModel(standardItemModel);

    connect(standardItemModel,    &QStandardItemModel::itemChanged,
            this,                 &KbModifierListWidget::handleItemDataChanged);

    connect(extendedQListView,    &ExtendedQListView::ModelItemClicked,
            this,                 &KbModifierListWidget::handleModelItemClicked);
}
