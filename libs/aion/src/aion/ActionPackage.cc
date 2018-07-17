#include "ActionPackage.hh"
#include "ActionLabel.hh"

#include "common/auto.hh"

#include "common/NetMessage.hh"

using namespace aion;

ActionPackage::ActionPackage(std::vector<ActionEntry> const& entries, std::vector<ActionLabel*> const& labels)
  : mEntries(entries), mLabels(labels)
{
}

ActionPackage::~ActionPackage()
{
    if (mDeleteLabels)
        for (_ l : mLabels)
            delete l;
}

void ActionPackage::saveToFile(const std::string& filename) const
{
    // QFileInfo(QString::fromStdString(filename)).dir().mkpath(".");

    NetMessage m;

    m.writeUInt64(mEntries.size(), "#entries");
    m.writeUInt64(mLabels.size(), "#labels");

    for (_ const& e : mEntries)
        m.writeStruct(e, "e");

    for (_ const& l : mLabels)
    {
        m.writeString(l->getName(), "name");
        m.writeString(l->getFile(), "file");
        m.writeString(l->getFunction(), "function");
        m.writeInt32(l->getLine(), "line");
    }

    m.writeToFile(filename);
}

SharedActionPackage ActionPackage::loadFromFile(const std::string& filename)
{
    _ entries = std::vector<ActionEntry>{};
    _ labels = std::vector<ActionLabel*>{};

    _ m = NetMessage::readFromFile(filename);
    if (!m)
        return nullptr;

    _ eCnt = m->readUInt64("#entries");
    _ lCnt = m->readUInt64("#labels");

    while (eCnt > 0)
    {
        entries.push_back(m->readStruct<ActionEntry>("e"));
        --eCnt;
    }

    for (_ lIdx = 0u; lIdx < lCnt; ++lIdx)
    {
        _ name = m->readString("name");
        _ file = m->readString("file");
        _ function = m->readString("function");
        _ line = m->readInt32("line");

        labels.push_back(new ActionLabel(name, function, file, line, lIdx));
    }

    _ sap = std::make_shared<ActionPackage>(entries, labels);
    sap->mDeleteLabels = true;
    return sap;
}

SharedActionPackage ActionPackage::complete()
{
    _ entries = ActionLabel::copyAllEntries();
    _ labels = ActionLabel::getAllLabels();
    return std::make_shared<ActionPackage>(entries, labels);
}
