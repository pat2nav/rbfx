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

#include "../Core/CommonEditorActions.h"

namespace Urho3D
{

CreateRemoveNodeAction::CreateRemoveNodeAction(Node* node, bool removed)
    : removed_(removed)
    , scene_(node->GetScene())
    , data_(node)
{
}

bool CreateRemoveNodeAction::IsAlive() const
{
    return scene_ != nullptr;
}

void CreateRemoveNodeAction::Redo() const
{
    if (removed_)
        RemoveNode();
    else
        AddNode();
}

void CreateRemoveNodeAction::Undo() const
{
    if (removed_)
        AddNode();
    else
        RemoveNode();
}

void CreateRemoveNodeAction::AddNode() const
{
    if (!scene_)
        return;

    if (Node* node = data_.SpawnExact(scene_))
        ;
    else
        throw UndoException("Cannot create node with id {}", data_.GetId());
}

void CreateRemoveNodeAction::RemoveNode() const
{
    if (!scene_)
        return;

    if (Node* node = scene_->GetNode(data_.GetId()))
        node->Remove();
    else
        throw UndoException("Cannot remove node with id {}", data_.GetId());
}

ChangeNodeTransformAction::ChangeNodeTransformAction(Node* node, const Transform& oldTransform)
    : scene_(node->GetScene())
    , nodes_{{node->GetID(), NodeData{oldTransform, node->GetDecomposedTransform()}}}
{
}

bool ChangeNodeTransformAction::IsAlive() const
{
    return scene_ != nullptr;
}

void ChangeNodeTransformAction::Redo() const
{
    if (!scene_)
        return;

    for (const auto& [nodeId, nodeData] : nodes_)
    {
        if (Node* node = scene_->GetNode(nodeId))
            node->SetTransform(nodeData.newTransform_);
        else
            throw UndoException("Cannot find node with id {}", nodeId);
    }
}

void ChangeNodeTransformAction::Undo() const
{
    if (!scene_)
        return;

    for (const auto& [nodeId, nodeData] : nodes_)
    {
        if (Node* node = scene_->GetNode(nodeId))
            node->SetTransform(nodeData.oldTransform_);
        else
            throw UndoException("Cannot find node with id {}", nodeId);
    }
}

bool ChangeNodeTransformAction::MergeWith(const EditorAction& other)
{
    const auto otherAction = dynamic_cast<const ChangeNodeTransformAction*>(&other);
    if (!otherAction)
        return false;

    if (scene_ != otherAction->scene_)
        return false;

    for (const auto& [nodeId, nodeData] : otherAction->nodes_)
    {
        if (nodes_.contains(nodeId))
            nodes_[nodeId].newTransform_ = nodeData.newTransform_;
        else
            nodes_[nodeId] = nodeData;
    }
    return true;
}

}
