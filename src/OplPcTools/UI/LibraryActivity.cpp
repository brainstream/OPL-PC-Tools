#include <OplPcTools/UI/Application.h>
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
    connect(&Application::instance().library(), &Library::loaded, this, &LibraryActivity::onLibraryLoaded);
}

void LibraryActivity::onLibraryLoaded()
{
    mp_label_path->setText(Application::instance().library().directory().absolutePath());
}

QSharedPointer<Intent> LibraryActivity::createIntent()
{
    return QSharedPointer<Intent>(new LibraryActivityIntent());
}
