//
// Copyright (c) 2017-2020 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../SystemUI/SerializableInspectorWidget.h"

#include "../Scene/Component.h"
#include "../Scene/Node.h"
#include "../SystemUI/Widgets.h"
#include "../SystemUI/SystemUI.h"

#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Urho3D
{

SerializableInspectorWidget::SerializableInspectorWidget(Context* context, const SerializableVector& objects)
    : Object(context)
    , objects_(objects)
{
    URHO3D_ASSERT(!objects_.empty());
}

SerializableInspectorWidget::~SerializableInspectorWidget()
{
}

void SerializableInspectorWidget::PruneObjects()
{
    ea::erase_if(objects_, [](const WeakPtr<Serializable>& obj) { return !obj; });
}

void SerializableInspectorWidget::RenderTitle()
{
    ui::Text("%s", GetTitle().c_str());
}

ea::string SerializableInspectorWidget::GetTitle()
{
    PruneObjects();
    if (objects_.empty())
        return "Nothing selected";

    ea::string extras;
    for (const Serializable* object : objects_)
    {
        ea::optional<unsigned> id;

        // TODO(editor): This is a hack to get IDs
        if (const Node* node = dynamic_cast<const Node*>(object))
            id = node->GetID();
        else if (const Component* component = dynamic_cast<const Component*>(object))
            id = component->GetID();

        if (id)
        {
            if (!extras.empty())
                extras += ", ";
            extras += ea::to_string(*id);
        }
    }

    Serializable* object = objects_.front();
    if (objects_.size() == 1)
    {
        if (extras.empty())
            return Format("{}", object->GetTypeName());
        else
            return Format("{} ({})", object->GetTypeName(), extras);
    }
    else
    {
        if (extras.empty())
            return Format("{}x {}", objects_.size(), object->GetTypeName());
        else
            return Format("{}x {} ({})", objects_.size(), object->GetTypeName(), extras);
    }
}

void SerializableInspectorWidget::RenderContent()
{
    PruneObjects();
    if (objects_.empty())
        return;

    const auto attributes = objects_[0]->GetAttributes();
    if (!attributes)
        return;

    pendingSetAttributes_.clear();
    for (const AttributeInfo& info : *attributes)
    {
        if (info.mode_ & AM_NOEDIT)
            continue;

        const IdScopeGuard guard{info.name_.c_str()};
        RenderAttribute(info);
    }

    if (!pendingSetAttributes_.empty())
    {
        for (const auto& [info, value] : pendingSetAttributes_)
        {
            OnEditAttributeBegin(this, objects_, info);
            for (Serializable* object : objects_)
            {
                if (object)
                {
                    object->SetAttribute(info->name_, value);
                    object->ApplyAttributes();
                }
            }
            OnEditAttributeEnd(this, objects_, info);
        }
    }

}

void SerializableInspectorWidget::RenderAttribute(const AttributeInfo& info)
{
    Variant value;
    info.accessor_->Get(objects_[0], value);

    Variant tempValue;
    const bool isUndefined = ea::any_of(objects_.begin() + 1, objects_.end(),
        [&](const WeakPtr<Serializable>& obj) { info.accessor_->Get(obj, tempValue); return tempValue != value; });
    const bool isDefaultValue = value == info.defaultValue_;

    Widgets::ItemLabel(info.name_.c_str(), Widgets::GetItemLabelColor(isUndefined, isDefaultValue));

    const ColorScopeGuard guardBackgroundColor{ImGuiCol_FrameBg, Widgets::GetItemBackgroundColor(isUndefined), isUndefined};

    Widgets::EditVariantOptions options;
    if (!info.enumNames_.empty())
        options = options.Enum(info.enumNames_);

    if (Widgets::EditVariant(value, options))
        pendingSetAttributes_.emplace_back(&info, value);
}

}
