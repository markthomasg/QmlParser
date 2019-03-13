#include "qmlparser.h"

QmlParser::QmlParser(QObject *parent)
    :QObject(parent)
{
    QQmlApplicationEngine engine;

    QString qmlCode;

    QFile file;
    file.setFileName("F:/Development/C++/QmlTranslator/mainwindow.ui");
//    file.setFileName("F:/Development/C++/pronto/Src/Mac/Pronto.Shell/Forms/MailWidget/MailWidget.ui");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error: open";
//        return;
    }

    /// Реализация через QDomDocument
    QDomDocument doc;

    if (!doc.setContent(file.readAll()))
        qDebug() << "bad xml file";
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "ui")
        qDebug() << "bad xml file: not ui file";

    QDomNode node = root.firstChild();
    generateInternals(node);

    outputInternal();

    QFile qmlFile("F:/Development/C++/QmlTranslator/test.qml");
    QString str = "import QtQuick 2.0\nimport QtQuick.Controls 2.1\n"
                  "import QtQuick.Window 2.0\n"
                  "ApplicationWindow {\nid: window\nvisible: true\n"
                  "Rectangle {"
                  "anchors.fill: parent;"
                  "color: \"red\";"
                  "}"
                  ""
                  ""
                  "}";

    if (!qmlFile.open(QIODevice::WriteOnly))
    {
        qmlFile.close();
    } else
    {
        QTextStream out(&qmlFile);
        out << str;
        qmlFile.close();
    }

    engine.load(QUrl("file:F:/Development/C++/QmlTranslator/test.qml"));
//    if (engine.rootObjects().isEmpty())
//        return;

}

void QmlParser::generateInternals(QDomNode &node)
{
    while (!node.isNull())
    {
        Internal * internal= nullptr;

        // Если элемент корневой, то выделить родителя
        if (node.parentNode().nodeName() == "ui")
        {
            if (node.nodeName() == "widget")
            {
                //            qDebug() << "\tmain():" << node.nodeName() << "=" << node.attributes().namedItem("class").nodeValue();
                internal = new Internal;
                internal->className = node.attributes().namedItem("class").nodeValue();
                internal->name = node.attributes().namedItem("name").nodeValue();

                internals.push_back(internal);
            }

            if (node.hasChildNodes())
                checkChildren(node, internal);
            node = node.nextSibling();
        }
        else
        {
            checkChildren(node, internal);
        }
    }

}

void QmlParser::checkChildren(QDomNode &node, Internal *parent)
{
    Internal *child = nullptr;

    QDomNode childNode = node.firstChild();
//    QString output = "c():" + node.nodeName() +  "->";
    while (!childNode.isNull())
    {
//            qDebug() << "before check: " << childNode.nodeName() << "node:" << node.nodeName();
            //                 << "NODE TYPE: " << childNode.isAttr() << childNode.isElement() << childNode.isText() << childNode.isEntity() << childNode.isCharacterData();

        if (childNode.nodeName() == "widget")
        {
            child = new Internal;
            child->parent = parent;
            child->className = childNode.attributes().namedItem("class").nodeValue();
            child->name = childNode.attributes().namedItem("name").nodeValue();
            parent->children.push_back(child);
//                qDebug() << "\t" << output << childNode.nodeName() << "=" << childNode.attributes().namedItem("class").nodeValue();

        }
//            else
//            {
//                if (!childNode.isText())
//                    child->className =  "not a widget: " + childNode.nodeName();
//            }

        if (childNode.hasChildNodes())
            checkChildren(childNode, child);

        childNode = childNode.nextSibling();
    }
}


void QmlParser::outputChild(Internal *obj)
{
    static int deep = 1;
    if (!obj->children.isEmpty())
    {
        for(auto child: obj->children)
        {
            QString tab;
            for (int i = 0; i < deep; ++i) tab += "  ";
            qDebug() << tab << child->className << "(" << child->name << ")";
            ++deep;
            outputChild(child);
            --deep;
        }
    }
}

void QmlParser::outputInternal()
{
    qDebug() << "\n\nCreating QML...";
    for(auto obj: internals)
    {
        qDebug() << obj->className;
        outputChild(obj);
    }
}
