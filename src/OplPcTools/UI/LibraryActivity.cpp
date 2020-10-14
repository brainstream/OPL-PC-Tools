#include <OplPcTools/Library.h>
#include <OplPcTools/UI/LibraryActivity.h>

using namespace OplPcTools::UI;

namespace {

class LibraryActivityIntent : public Intent
{
public:
    Activity * createActivity(QWidget * _parent) override
    {
        return new LibraryActivity(_parent);
    }

    QString activityClass() const override
    {
        return "Library";
    }
};

} // namespace

LibraryActivity::LibraryActivity(QWidget * _parent) :
    Activity(_parent)
{
    setupUi(this);
    mp_tabs->setCurrentIndex(0);
    connect(&Library::instance(), &Library::loaded, this, &LibraryActivity::onLibraryLoaded);
    onLibraryLoaded();
}

void LibraryActivity::onLibraryLoaded()
{
    mp_label_path->setText(Library::instance().directory());
}

QSharedPointer<Intent> LibraryActivity::createIntent()
{
    return QSharedPointer<Intent>(new LibraryActivityIntent());
}
