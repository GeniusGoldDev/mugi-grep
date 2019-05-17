#ifndef HTML_H
#define HTML_H

#include <QStringList>

namespace Html
{

    QString span(const QString &text_, const QString &color, bool bold = false);

    QString span(const QString &text_, const QString &color, const QString &background);

    QString anchor(const QString &text, const QString& path, const QString& color);

    QString spanZebra(const QStringList& text, const QString& color, const QString& background, const QString& altBackground);

}

#endif // HTML_H
