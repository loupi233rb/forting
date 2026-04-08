#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>

#include "fortingcore.h"
#include "airuleparser.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    Forting::File f;
    f.init();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("f",&f);

    qmlRegisterUncreatableMetaObject(
        Forting::staticMetaObject,
        "FortingStruct", 1, 0,
        "Forting",
        "Enum only");

    QObject::connect(
        &engine,

        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("forting", "MainWindow");

    return app.exec();


    // QString script = Forting::readFromFile("C:/Users/lei29/Desktop/tempStation.txt");
    // Forting::RuleParser parser;
    // if (!parser.parse(script)) {
    //     auto e = parser.lastError();
    //     qDebug() << "Parse error at" << e.line << ":" << e.column << e.message;
    //     return 1;
    // }

    // Forting::FileEntry f;
    // f.name = "report";
    // f.suffix = "pdf";
    // f.size = 12345;
    // f.created = QDateTime(QDate(2025, 1, 2), QTime(10, 0));
    // f.modified = QDateTime(QDate(2026, 3, 7), QTime(12, 0));
    // f.isDir = false;
    // f.isFile = true;

    // QVector<Forting::action> res = parser.evaluate(f);
    // for (int i = 0; i < res.size(); ++i) {
    //     qDebug().noquote()
    //     << QString("unit=%1 decision=%2 tag='%3' rename='%4' delete=%5")
    //             .arg(i)
    //             .arg(res[i].decision)
    //             .arg(res[i].tagValue)
    //             .arg(res[i].renameValue)
    //             .arg(res[i].deleteFlag ? "true" : "false");
    // }
    // return 0;
}
