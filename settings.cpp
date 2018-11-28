#include "settings.h"

#include <QAbstractItemModel>
#include <QDir>

#include <QStandardPaths>
#include <QVariantList>
#include <QDebug>
#include <QApplication>
#include "utils/jsonhelper.h"

/*static*/
Settings* Settings::mInstance = nullptr;

Settings *Settings::instance()
{
    if (!mInstance)
        mInstance = new Settings();
    return mInstance;
}

void Settings::load()
{
    QString path = this->path();
    if (!QFile(path).exists()) {

        QDir dir(qApp->applicationDirPath());
        if (dir.dirName() == "debug") {
            dir.cdUp();
        }

#ifdef Q_OS_WIN
        path = dir.filePath("settings.win32.json");
#endif
#ifdef Q_OS_LINUX
        path = dir.filePath("settings.linux.json");
#endif
    }

    if (!QFile(path).exists()) {
        return;
    }

    QJsonDocument doc = loadJson(path);

    if (!doc.isObject()) {
        return;
    }

    QJsonObject settings = doc.object();

    mSessions = settings.value("sessions").toArray();
    mExps = settings.value("exps").toObject();

    QJsonArray editors = settings.value("editors").toArray();

    for(QJsonValue editor: editors) {
        mEditors.append(Editor(editor.toObject()));
    }
}

QString Settings::path() const
{
    QString home = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir d(home);
    if (!d.exists()) {
        d.cdUp();
        if (!d.mkdir("mugi-grep")) {
            qDebug() << "cannot create" << d.filePath("mugi-grep");
        }
        d.cd("mugi-grep");
    }
    return d.filePath("settings.json");
}

QString Settings::error() const
{
    return mError;
}

QJsonArray Settings::sessions() const
{
    return mSessions;
}

QJsonObject Settings::exps() const {
    return mExps;
}

void Settings::setSessions(const QJsonArray &value)
{
    mSessions = value;
}

void Settings::setExps(const QJsonObject &value)
{
    mExps = value;
}

void Settings::save()
{
    QJsonObject settings;

    QJsonArray editors;
    Editor editor;
    foreach(editor,mEditors) {
        QJsonObject editor_;
        editor.toJson(editor_);
        editors << editor_;
    }
    settings["editors"] = editors;
    settings["sessions"] = mSessions;
    settings["exps"] = mExps;

    QString path = this->path();
    saveJson(path, settings);

    qDebug() << "saved to" << path;
}

QString Settings::editor(const QString &path) const
{

    if (mEditors.isEmpty())
        return QString();

    int p = path.lastIndexOf(".");
    if (p < 0) {
        return mEditors.last().app();
    }
    QString ext = path.mid(p+1);

    Editor editor;
    foreach(editor,mEditors) {
        if (ext.indexOf(editor.exp()) > -1)
            return editor.app();
    }
    return QString();
}

Settings::Settings()
{
    load();
}

void Settings::toModel(QAbstractItemModel *model)
{
    for(int i=0;i<model->rowCount();i++) {
        QString exts;
        QString app;
        if (mEditors.size() > i) {
            exts = mEditors[i].exts();
            app = mEditors[i].app();
        }
        model->setData(model->index(i,0),exts);
        model->setData(model->index(i,1),app);
    }
}

void Settings::fromModel(QAbstractItemModel *model)
{
    mEditors.clear();
    for(int i=0;i<model->rowCount();i++) {
        QString exts = model->data(model->index(i,0)).toString();
        QString app = model->data(model->index(i,1)).toString();
        if (!exts.isEmpty() && !app.isEmpty())
            mEditors.append( Editor(exts,app) );
    }
}
